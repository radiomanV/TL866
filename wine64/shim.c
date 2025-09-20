/*
 * shim.c
 * Native x86 dll wrapper for Minipro TL866A/CS, TL866II+, XgPro T48, T56 and T76
 * programmers.
 * This library will redirect all USB related functions from Minipro or Xgpro
 * software to the Linux USB subsystem via usb-broker RPC calls.
 * Created: September 10, 2025
 * Author: radiomanV
 */

#include <winsock2.h>
#include <windows.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <dbt.h>
#include <shlwapi.h>
#include <uxtheme.h>

#include "protocol.h"
#include "resource.h"

// Defines
#define TL866A_VID	  0x04d8
#define TL866A_PID	  0xe11c
#define TL866II_VID	  0xa466
#define TL866II_PID	  0x0a53
#define T76_VID		  0xa466
#define T76_PID		  0x1a86

#define X86_PUSH	  0x68
#define X86_RET		  0xc3
#define X86_JMP		  0xeb

// Typedefs
typedef struct {
	void *handle;
	UCHAR PipeID;
	PUCHAR Buffer;
	ULONG BufferLength;
	PUINT LengthTransferred;
	LPOVERLAPPED Overlapped;
} Args;

typedef struct {
	uint32_t handle_id;
	uint16_t wMaxPacketSize [16];
} broker_handle_t;

// Broker connection status
typedef enum {
	BRK_STATE_INIT = 0,
	BRK_STATE_CONNECTING = 1,
	BRK_STATE_READY = 2,
	BRK_STATE_FAILED = 3,
} broker_state_t;

// Notification interfaces
const GUID MINIPRO_GUID = {
	   0x85980D83, 0x32B9, 0x4BA1,
	   { 0x8F, 0xDF, 0x12, 0xA7, 0x11, 0xB9, 0x9C, 0xA2 } };

const GUID XGPRO_GUID1 = {
	   0xE7E8BA13, 0x2A81, 0x446E,
	   { 0xA1, 0x1E, 0x72, 0x39, 0x8F, 0xBD, 0xA8, 0x2F } };

const GUID XGPRO_GUID2 = {
	   0x015DE341, 0x91CC, 0x8286,
	   { 0x39, 0x64, 0x1A, 0x00, 0x6B, 0xC1, 0xF0, 0x0F } };

// Xgpro/Minipro pointers
HANDLE *usb_handle;
HANDLE *winusb_handle;
int *devices_count;

// Globals
GUID m_guid;
uint16_t device_vid;
uint16_t device_pid;
uint16_t broker_port;
int ep_timeout[2][7];
broker_handle_t broker_handle[4];
volatile LONG broker_state = BRK_STATE_INIT;
HANDLE broker_evt = NULL;
HANDLE cancel_evt = NULL;
HINSTANCE m_hInst = NULL;
HANDLE hotplug_thread = NULL;
HWND hWnd_notify;
WNDPROC prev_wndproc = NULL;
int debug = 0;

// These are function signatures extracted from Xgpro.exe and should be
// compatible from V7.0 and above.
const unsigned char xgpro_open_devices_pattern1[] = {
	0x53, 0x57, 0x6A, 0x00, 0x68, 0x80, 0x00, 0x00, 0x40, 0x6A, 0x03,
	0x6A, 0x00, 0x6A, 0x03, 0x68, 0x00, 0x00, 0x00, 0xC0, 0x68
};

const unsigned char xgpro_open_devices_pattern2[] = {
	0x6A, 0x00, 0x68, 0x80, 0x00, 0x00, 0x40, 0x6A, 0x03, 0x6A,
	0x00, 0x6A, 0x03, 0x68, 0x00, 0x00, 0x00, 0xC0, 0x51
};

// These are function signatures extracted from MiniPro.exe and should be
// compatible from V6.0 and above.
const unsigned char minipro_open_devices_pattern[] = {
	0x6A, 0x00, 0x68, 0x80, 0x00, 0x00, 0x00, 0x6A,
    0x03, 0x6A, 0x00, 0x6A, 0x03 };

const unsigned char usb_write_pattern[] = {
	0x8B, 0x94, 0x24, 0x0C, 0x10, 0x00, 0x00, 0x8D,
	0x44, 0x24, 0x00, 0x6A,0x00, 0x50, 0x8B, 0x84 };

const unsigned char usb_write2_pattern[] = {
    0x8B, 0x94, 0x24, 0x10, 0x10, 0x00, 0x00, 0x8D,
	0x44, 0x24, 0x00, 0x6A, 0x00, 0x50, 0x8B, 0x84 };

const unsigned char usb_read_pattern[] = {
    0x64, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x8B, 0x4C,
	0x24, 0x08, 0x8B, 0x54, 0x24, 0x04, 0x6A, 0xFF };

const unsigned char usb_read2_pattern[] = {
	0x8B, 0x4C, 0x24, 0x0C, 0x8B, 0x54, 0x24, 0x08,
	0x8D, 0x44, 0x24, 0x0C, 0x6A, 0x00, 0x50, 0x51 };

const unsigned char brickbug_pattern[] = {
	0x83, 0xC4, 0x18, 0x3D, 0x13, 0xF0, 0xC2, 0xC8, 0x75 };

// Print given array in hex
void print_hex(const unsigned char *buffer, unsigned int size)
{
	unsigned int i;
	for (i = 0; i < size; i++) {
		printf("%02X ", buffer[i]);
		if ((i + 1) % 16 == 0 || i + 1 == size) {
			unsigned int start = i / 16 * 16;
			if ((i + 1) % 16 != 0) {
				printf("%*s", (16 - (i + 1) % 16) * 3, "");
			}
			if (debug != 3) {
				printf("  ");
				for (unsigned int j = start; j <= i; j++) {
					printf("%c", (buffer[j] < 32 ||
							buffer[j] > 126) ?
							       '.' :
							       buffer[j]);
				}
			}
			printf("\n");
		}
	}
	printf("\n");
}

// memmem replacement function for Windows
static inline void *memmem_r(const void *haystack, size_t hlen,
			     const void *needle, size_t nlen)
{
	if (nlen == 0)
		return (void *)haystack;
	if (hlen < nlen)
		return NULL;

	const unsigned char *h = (const unsigned char *)haystack;
	const unsigned char *n = (const unsigned char *)needle;
	const unsigned char *h_end = h + (hlen - nlen) + 1;
	const unsigned char first = n[0];

	while (h < h_end) {
		const unsigned char *p = (const unsigned char *)memchr(
			h, first, (size_t)(h_end - h));
		if (!p)
			return NULL;
		if (memcmp(p, n, nlen) == 0)
			return (void *)p;
		h = p + 1;
	}
	return NULL;
}


/*================== RPC client functions ==========s========*/

// Init WSA
int wsa_init(void)
{
	static LONG wsa_init = 0;
	if (InterlockedCompareExchange(&wsa_init, 1, 0) == 0) {
		WSADATA w;
		if (WSAStartup(MAKEWORD(2, 2), &w)) {
			InterlockedExchange(&wsa_init, 0);
			printf("WSA initialization error.\n");
			return BR_E_WSA;
		}
	}
	return NO_ERROR;
}

// Get a socket
static SOCKET get_socket(struct sockaddr_in *out_addr)
{
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
		return INVALID_SOCKET;

	DWORD to = 5000;
	setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (const char *)&to, sizeof(to));
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char *)&to, sizeof(to));
	int one = 1;
	setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const char *)&one,
		   sizeof(one));
	setsockopt(s, SOL_SOCKET, SO_DONTROUTE, (const char *)&one,
		   sizeof(one));

	if (out_addr) {
		memset(out_addr, 0, sizeof(*out_addr));
		out_addr->sin_family = AF_INET;
		out_addr->sin_port = htons(broker_port);
		out_addr->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	}
	return s;
}

// Connect to broker
static int connect_to_broker(int timeout_ms)
{
	struct sockaddr_in addr;
	SOCKET sock = get_socket(&addr);
	if (sock == INVALID_SOCKET)
		return BR_E_SOCK;

	// Set non-blocking I/O socket
	u_long nb = 1;
	ioctlsocket(sock, FIONBIO, &nb);

	int rc = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
	if (rc == 0) {
		closesocket(sock);
		return 0;
	}

	int err = WSAGetLastError();
	if (err != WSAEWOULDBLOCK && err != WSAEINPROGRESS) {
		closesocket(sock);
		return BR_E_CONNECT_IMMEDIATE;
	}

	fd_set wf;
	FD_ZERO(&wf);
	FD_SET(sock, &wf);
	struct timeval tv;
	tv.tv_sec = timeout_ms / 1000;
	tv.tv_usec = (timeout_ms % 1000) * 1000;
	rc = select((int)sock + 1, NULL, &wf, NULL, &tv);
	if (rc == 1) {
		int soerr = 0;
		int len = sizeof(soerr);
		getsockopt(sock, SOL_SOCKET, SO_ERROR, (char *)&soerr, &len);
		closesocket(sock);
		return (soerr == 0) ? NO_ERROR : BR_E_CONNECT_SOERR;
	}
	closesocket(sock);
	return BR_E_CONNECT_TIMEOUT;
}

// Try to connect to broker
int try_connect(DWORD timeout_ms)
{
	DWORD start = GetTickCount();
	DWORD limit = start + timeout_ms;

	// We're already connected, fast exit.
	if (InterlockedCompareExchange(&broker_state, BRK_STATE_READY,
				       BRK_STATE_READY) == BRK_STATE_READY)
		return NO_ERROR;

	// Try to become connector thread
	LONG prev = InterlockedCompareExchange(
		&broker_state, BRK_STATE_CONNECTING, BRK_STATE_INIT);
	if (prev != BRK_STATE_INIT)
		prev = InterlockedCompareExchange(
			&broker_state, BRK_STATE_CONNECTING, BRK_STATE_FAILED);

	// Reset the event so that others threads will wait for this cycle
	if (prev == BRK_STATE_INIT || prev == BRK_STATE_FAILED) {
		if (broker_evt)
			ResetEvent(broker_evt);

		for (;;) {
			if (connect_to_broker(200) == 0) {
				InterlockedExchange(&broker_state,
						    BRK_STATE_READY);
				if (broker_evt)
					SetEvent(broker_evt);
				return NO_ERROR;
			}
			if ((LONG)(GetTickCount() - limit) >= 0) {
				InterlockedExchange(&broker_state,
						    BRK_STATE_FAILED);
				if (broker_evt)
					SetEvent(broker_evt);
				return BRK_E_READY_TIMEOUT;
			}
			Sleep(100);
		}
	}

	// Another thread is connecting. Wait for timeout or READY/FAIL
	for (;;) {
		LONG s = broker_state;
		if (s == BRK_STATE_READY)
			return NO_ERROR;
		if (s == BRK_STATE_FAILED)
			return BRK_E_READY_FAILED;

		DWORD now = GetTickCount();
		DWORD left =
			(DWORD)((LONG)(limit - now) > 0 ? (limit - now) : 0);
		if (left == 0)
			return BRK_E_WAIT_TIMEOUT;

		if (broker_evt) {
			DWORD wr = WaitForSingleObject(broker_evt, left);
			if (wr == WAIT_TIMEOUT)
				return BRK_E_WAIT_TIMEOUT;
		} else {
			Sleep(min(left, 50));
		}
	}
	return NO_ERROR;
}

// Reset wait event state if a fatal wsa error detected
void reset_state(void)
{
	InterlockedExchange(&broker_state, BRK_STATE_FAILED);
	int e = WSAGetLastError();
	int fatal = (e == WSAECONNREFUSED || e == WSAETIMEDOUT ||
		     e == WSAENETUNREACH || e == WSAEHOSTUNREACH ||
		     e == WSAENETDOWN || e == WSAECONNRESET ||
		     e == WSAESHUTDOWN);
	if (fatal && broker_evt)
		SetEvent(broker_evt);
}

// Socket send function
static int socket_send(SOCKET s, const void *buf, int len)
{
	const char *pbuf = (const char *)buf;
	int sent = 0;
	while (sent < len) {
		int n = send(s, pbuf + sent, len - sent, 0);
		if (n <= 0)
			return RPC_E_SEND_ERR;
		sent += n;
	}
	return NO_ERROR;
}

// Socket receive function
static int socket_recv(SOCKET s, void *buf, int len)
{
	char *pbuf = (char *)buf;
	int recvd = 0;
	while (recvd < len) {
		int n = recv(s, pbuf + recvd, len - recvd, 0);
		if (n <= 0)
			return RPC_E_RECV_ERR;
		recvd += n;
	}
	return NO_ERROR;
}

// RPC call function to USB broker
int rpc_call(uint16_t type, const void *req, uint32_t req_len,
	     const void *extra, uint32_t extra_len, void *resp_buf,
	     uint32_t *resp_len)
{
	// Validity check
	if (!resp_len)
		return RPC_E_RESP_LEN_NULL;
	if (*resp_len && !resp_buf)
		return RPC_E_RESP_BUF_NULL;

	// Try to connect to broker
	int ret = try_connect(3000);
	if (ret != 0)
		return ret;

	// Get a socket
	struct sockaddr_in addr;
	SOCKET sock = get_socket(&addr);
	if (sock == INVALID_SOCKET)
		return RPC_E_MKSOCK_FAIL;

	// Connect to broker
	if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
		int e = WSAGetLastError();
		reset_state();
		closesocket(sock);
		return (e == WSAETIMEDOUT) ? RPC_E_CONNECT_TIMEOUT :
					     RPC_E_CONNECT_FAIL;
	}

	// Send header
	rpc_hdr_t snd_hdr;
	snd_hdr.len = req_len + extra_len;
	snd_hdr.type = type;
	snd_hdr.version = PROTO_VERSION;
	if (socket_send(sock, &snd_hdr, (int)sizeof(snd_hdr))) {
		reset_state();
		closesocket(sock);
		return RPC_E_SEND_HEADER;
	}

	// Send request
	if (req_len && socket_send(sock, req, (int)req_len)) {
		closesocket(sock);
		return RPC_E_SEND_REQ;
	}

	// Send extra data
	if (extra_len && socket_send(sock, extra, (int)extra_len)) {
		closesocket(sock);
		return RPC_E_SEND_EXTRA;
	}

	// Receive header
	rpc_hdr_t rcv_hdr;
	if (socket_recv(sock, &rcv_hdr, (int)sizeof(rcv_hdr))) {
		reset_state();
		closesocket(sock);
		return RPC_E_RECV_HEADER;
	}

	// Check if required protocol version match
	if (rcv_hdr.version != PROTO_VERSION) {
		reset_state();
		closesocket(sock);
		return RPC_E_PROTO_MISMATCH;
	}

	// Check for response header length
	if (rcv_hdr.len > *resp_len) {
		reset_state();
		closesocket(sock);
		return RPC_E_RESP_TOO_LARGE;
	}

	// Receive payload
	if (rcv_hdr.len) {
		if (socket_recv(sock, resp_buf, (int)rcv_hdr.len)) {
			closesocket(sock);
			return RPC_E_RECV_PAYLOAD;
		}
	}
	*resp_len = rcv_hdr.len;

	// Disconect and return
	closesocket(sock);
	return NO_ERROR;
}

/*================== Minipro/Xgpro replacement functions =====================*/

// USB close replacement function. Called by Xgpro/Minipro
void close_devices(void)
{
	int max = (device_pid == T76_PID) ? 1 : 4;
	int any = 0;
	for (int i = 0; i < max; i++) {
		if (broker_handle[i].handle_id ||
		    usb_handle[i] != INVALID_HANDLE_VALUE) {
			any = 1;
			break;
		}
	}
	if (!any)
		return;

	printf("Close devices\n");
	try_connect(1000);

	for (int i = 0; i < max; i++) {
		if (broker_handle[i].handle_id) {
			close_req_t q = { .handle_id =
						  broker_handle[i].handle_id };
			uint8_t rb[32];
			uint32_t rl = sizeof(rb);
			rpc_call(MSG_CLOSE, &q, sizeof(q), NULL, 0, rb, &rl);
			broker_handle[i].handle_id = 0;
		}
	}

	for (int i = 0; i < 4; i++) {
		usb_handle[i] = INVALID_HANDLE_VALUE;
		if (device_vid == TL866II_VID)
			winusb_handle[i] = INVALID_HANDLE_VALUE;
	}
	if (devices_count)
		*devices_count = 0;
}

// USB open replacement function. Called by Xgpro/Minipro
int open_devices(void)
{
	// Close devices first
	close_devices();

	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 7; j++)
			ep_timeout[i][j] = 5000;

	// Initialize all handles
	usb_handle[0] = INVALID_HANDLE_VALUE;
	if (device_pid != T76_PID) {
		usb_handle[1] = usb_handle[2] = usb_handle[3] =
			INVALID_HANDLE_VALUE;
	}

	if (device_vid == TL866II_VID) {
		*devices_count = 0;
		winusb_handle[0] = INVALID_HANDLE_VALUE;
		if (device_pid != T76_PID)
			winusb_handle[1] = winusb_handle[2] = winusb_handle[3] =
				INVALID_HANDLE_VALUE;
	}

	// Try to connect to the usb-broker
	printf("Open devices:\n");
	if (try_connect(30000) != NO_ERROR) {
		printf("Couldn't connect to usb-broker\n");
		return 0;
	}

	// Performs an enumerate RPC call to retrieve the USB devices list
	static uint8_t resp_buf[16384];
	uint32_t resp_len = sizeof(resp_buf);
	int ret =
		rpc_call(MSG_ENUMERATE, NULL, 0, NULL, 0, resp_buf, &resp_len);
	if (ret != 0 || resp_len < sizeof(enum_resp_t)) {
		printf("ENUM rpc_call failed rc=%d len=%u\n", ret, resp_len);
		return 0;
	}

	const enum_resp_t *enum_resp = (const enum_resp_t *)resp_buf;
	if (enum_resp->status != 0) {
		printf("ENUM status=%d\n", (int)enum_resp->status);
		return 0;
	}

	const uint8_t *p_list = resp_buf + sizeof(enum_resp_t);
	size_t left = resp_len - sizeof(enum_resp_t);
	uint32_t count = enum_resp->count;
	if (left < count * sizeof(dev_info_t)) {
		printf("ENUM payload too short: count=%u payload=%zu need=%zu\n",
		       count, left, (size_t)count * sizeof(dev_info_t));
		return 0;
	}

	const dev_info_t *list = (const dev_info_t *)p_list;
	int max_devices = (device_pid == T76_PID) ? 1 : 4;
	int devices_found = 0;

	// Loop through all devices in the list and parse each entry
	for (uint32_t i = 0; i < count; i++) {
		const dev_info_t *dev_info = &list[i];

		// If not our required VID/PID skip
		if (!(dev_info->vid == device_vid &&
		      dev_info->pid == device_pid))
			continue;

		// We found a device, lets try to open it
		open_req_t open_req;
		memset(&open_req, 0, sizeof(open_req));
		snprintf(open_req.path, sizeof(open_req.path), "%.*s",
			 (int)sizeof(open_req.path) - 1, dev_info->path);

		uint8_t out_buf[256];
		uint32_t out_len = sizeof(out_buf);
		ret = rpc_call(MSG_OPEN, &open_req, sizeof(open_req), NULL, 0,
			       out_buf, &out_len);
		if (ret != 0 || out_len < sizeof(open_resp_t)) {
			printf("OPEN rpc_call failed rc=%d len=%u\n", ret,
			       out_len);
			continue;
		}
		const open_resp_t *open_resp = (const open_resp_t *)out_buf;
		if (open_resp->status != 0) {
			printf("Device %s %s\n", dev_info->path,
			       (int)open_resp->status == RPC_E_OPEN_BUSY ?
				       "busy." :
				       "open error.");
			continue;
		}

		// If device was succesfully open initialize its handle
		int idx = devices_found;
		broker_handle[idx].handle_id = open_resp->handle_id;
		usb_handle[idx] = (HANDLE)idx;
		if (device_vid == TL866II_VID) {
			winusb_handle[idx] = (HANDLE)idx;
			*devices_count = idx + 1;
		}

		// Save wMaxPacketSize for later
		memcpy(broker_handle[idx].wMaxPacketSize,
		       dev_info->wMaxPacketSize,
		       sizeof(dev_info->wMaxPacketSize));

		// Format the device name
		char name[128] = { 0 };
		if (open_resp->product[0]) {
			snprintf(name, sizeof(name), "%.*s",
				 (int)sizeof(name) - 1, open_resp->product);
		} else {
			strcpy(name, "Unknown");
		}
		if (strstr(name, "Xingong"))
			strcpy(name, "XGecu TL866II+");
		else if (strstr(name, "MiniPro"))
			strcpy(name, "Minipro TL866A/CS");
		char *end = name + strlen(name) - 1;
		while (end >= name && isspace((unsigned char)*end))
			*end-- = '\0';

		// Format the device speed
		const char *speed = "";
		switch (open_resp->speed) {
		case 1:
			speed = "Low speed (1.5MBit/s)";
			break;
		case 2:
			speed = "Full speed (12MBit/s)";
			break;
		case 3:
			speed = "High speed (480MBit/s)";
			break;
		case 4:
			speed = "Super speed (5000MBit/s)";
		}

		// Print the device name and speed
		devices_found++;
		printf("  %u: VID_%04X, PID_%04X; %s; %s\n",
		       devices_found, dev_info->vid, dev_info->pid, name,
		       speed);

		// Limit the maximum devices to 4 or 1
		if (devices_found == max_devices)
			break;
	}

	if(!devices_found)
		printf("No devices found.\n");
	return 0;
}

// Helper function to get a pipe timeout
int get_timeout(UCHAR PipeID)
{
	return ep_timeout[(PipeID > 80 ? 1 : 0)][(PipeID & 0x7f) - 1];
}

// Helper function to set a pipe timeout
void set_timeout(UCHAR PipeID, int val)
{
	ep_timeout[(PipeID > 80 ? 1 : 0)][(PipeID & 0x7f) - 1] = val;
}

// Helper function to get wMaxPacketSize for a specified device
static inline int get_mps(int idx, unsigned char pipeId)
{
	unsigned epnum = (unsigned)(pipeId & 0x0F);
	if (idx < 0 || idx >= 4 || epnum >= 16)
		return 0;
	return (int)broker_handle[idx].wMaxPacketSize[epnum];
}

// Helper function to get USB error string
static const char *usb_errstr(int code)
{
	switch (code) {
	case 0:   return "Success";
	case -1:  return "Input/Output Error";
	case -2:  return "Invalid parameter";
	case -3:  return "Access denied";
	case -4:  return "No such device";
	case -5:  return "Entity not found";
	case -6:  return "Resource busy";
	case -7:  return "Operation timed out";
	case -8:  return "Overflow";
	case -9:  return "Pipe error";
	case -10: return "System call interrupted";
	case -11: return "Insufficient memory";
	case -12: return "Operation not supported";
	default:  return "Unknown error";
	}
}


/*============= Xgpro replacement functions. ==============*/

// USB transfer for WinUsb_ReadPipe/WinUsb_WritePipe.
// This function will run in a separate thread if overlapped
// transfer is specified.
void usb_transfer(Args *args)
{
	// Submit the bulk transfer to the broker
	{
		int idx = (int)(uintptr_t)args->handle;
		if (idx < 0 || idx > 3 || broker_handle[idx].handle_id == 0) {
			printf("\nIO error: %s\n", usb_errstr(-4));
			free(args);
			return;
		}

		const int ep_in = (args->PipeID & 0x80) ? 1 : 0;
		bulk_req_t req;
		memset(&req, 0, sizeof(req));
		req.handle_id = broker_handle[idx].handle_id;
		req.ep = args->PipeID;
		req.timeout_ms = get_timeout(args->PipeID);
		req.len = args->BufferLength;

		// Response buffer: for IN we need space for data
		uint32_t rcap = (uint32_t)(sizeof(bulk_resp_t) +
					   (ep_in ? args->BufferLength : 0));
		uint8_t *rbuf = (uint8_t *)malloc(rcap ? rcap : 1);
		if (!rbuf) {
			printf("Out of memory!\n");
			free(args);
			return;
		}

		uint32_t rlen = rcap;
		const void *extra = ep_in ? NULL : (const void *)args->Buffer;
		uint32_t xlen = ep_in ? 0 : args->BufferLength;

		int ret = rpc_call(MSG_BULK, &req, sizeof(req), extra, xlen,
				   rbuf, &rlen);
		if (ret != 0 || rlen < sizeof(bulk_resp_t)) {
			printf("\nIO error: %s\n", usb_errstr(ret ? -1 : -11));
			free(rbuf);
			free(args);
			return;
		}

		const bulk_resp_t *resp = (const bulk_resp_t *)rbuf;

		// Check if transfer was okay
		if (resp->status != 0) {
			printf("\nIO Error: %s\n", usb_errstr(resp->status));
			free(rbuf);
			free(args);
			return;
		}

		// Get the actual transfer length
		if (ep_in) {
			uint32_t rcvd = resp->rx_len;
			if (rcvd && args->Buffer) {
				const uint8_t *data =
					rbuf + sizeof(bulk_resp_t);
				if (rcvd > args->BufferLength)
					rcvd = args->BufferLength;
				memcpy(args->Buffer, data, rcvd);
			}
			*args->LengthTransferred = rcvd;
		} else {
			*args->LengthTransferred = args->BufferLength;
		}

		// Free the allocated transfer structure
		free(rbuf);
	}

	// If debug mode is active print some debug info
	if (debug) {
		printf("%s %s %u bytes on endpoint 0x%02X\n",
		       (args->PipeID & 0x80) ? "Read" : "Write",
		       args->Overlapped ? "Async" : "Normal",
		       *args->LengthTransferred, args->PipeID);
		if (debug == 1 || debug == 3) {
			print_hex(args->Buffer, *args->LengthTransferred);
		}
	}

	// If Overlapped (async) transfer was completed
	// signal the event to release the waiting object.
	if (args->Overlapped) {
		SetEvent(args->Overlapped->hEvent);
	}

	// Free the malloced args.
	free(args);
}

/********************** ENDPOINTS USAGE ********************************
 ***********************************************************************
 * TL866A/CS; wMaxPacketSize=64 bytes, 2 endpoints; USB 2.0, 12MBit/s  *
 * EP1_OUT=0x01, EP1_IN=0x81; All used                                 *
 ***********************************************************************
 * TL866II+; wMaxPacketSize=64 bytes, 6 endpoints; USB 2.0, 12MBit/s   *
 * EP1_OUT=0x01, EP1_IN=0x81, EP2_OUT=0x02, EP2_IN=0x82,               *
 * EP3_OUT=0x03, EP3_IN=0x83; All used                                 *
 ***********************************************************************
 * T48; wMaxPacketSize=512 bytes, 4 endpoints; USB 2.0, 480MBit/s      *
 * EP1_OUT=0x01, EP1_IN=0x81, EP2_OUT=0x02, EP2_IN=0x82; All used      *
 ***********************************************************************
 * T56 wMaxPacketSize = 512 bytes, 2 endpoints; USB 2.0, 480MBit/s     *
 * EP1_OUT=0x01, EP1_IN=0x81; All used                                 *
 ***********************************************************************
 * T76 wMaxPacketSize = 1024 bytes, 14 endpoints; USB 3.0, 5000MBit/s  *
 * EP1_OUT=0x01, EP1_IN=0x81, EP2_OUT=0x02, EP2_IN=0x82,               *
 * EP3_OUT=0x03, EP3_IN=0x83, EP4_OUT=0x04, EP4_IN=0x84,               *
 * EP5_OUT=0x05, EP5_IN=0x85, EP6_OUT=0x06, EP6_IN=0x86                *
 * EP7_OUT=0x07, EP7_IN=0x87;                                          *
 * Only EP1_OUT, EP1_IN, EP2_IN, EP5_OUT are used in current firmware  *
 ***********************************************************************/

// WinUsb_ReadPipe/winUsb_WritePipe implementation.
BOOL WINAPI WinUsb_Transfer(HANDLE InterfaceHandle, UCHAR PipeID, PUCHAR Buffer,
			    ULONG BufferLength, PUINT LengthTransferred,
			    LPOVERLAPPED Overlapped)
{
	// Check for usb handles
	int idx = (int)(uintptr_t)InterfaceHandle;
	if (idx < 0 || idx > 3 || broker_handle[idx].handle_id == 0) {
		printf("\nIO error: %s\n", usb_errstr(-4));
		return FALSE;
	}

	// Workaround for T76 endpoint 0x83 not used issue.
	// The Xgecu T76 software will issue a Winusb_ReadPipe on
	// endpoint 0x83 and later on will call WinUSB_AbortPipe.
	// Because the T76 firmware doesn't use endpoint 0x83 this will
	// get us a libusb timeout error and the Xgpro software locked
	// waiting for 'Overlapped->hEvent' to be signaled.
	// This will throw an error in Xgpro T76 and the programmer power
	// must be cycled.
	// We handle this bug here by releasing the waiting object first
	// and then aborting the transfer on this endpoint.
	if (device_pid == T76_PID && PipeID == 0x83) {
		if (Overlapped != NULL) {
			SetEvent(Overlapped->hEvent);
		}
		return TRUE;
	}

	// Workaround for Xgpro read BufferLength issue.
	// Depending on what chip is used we can get more bytes than
	// declared by the Xgpro software in 'BufferLength' argument;
	// so if the BufferLength < LengthTransferred we end with a libusb
	// overflow error (there is more unread data).
	// Perhaps the Windows driver handle this somewhat (multiple reads
	// or bigger buffers). We handle this by rounding the buffer size
	// in multiple of wMaxPacketSize bytes (64, 512 or 1024).

	if ((PipeID > 0x80)) {
		// Round BufferLength to the next multiple of endpoint wMaxPacketSize
		int wMaxPacketSize = get_mps(idx, PipeID) - 1;
		BufferLength = (BufferLength + wMaxPacketSize) &
			       ~wMaxPacketSize;
	}

	// Prepare args
	Args *args = malloc(sizeof(Args));
	if (!args) {
		printf("Out of memory!\n");
		return FALSE;
	}

	*args = (Args){ .handle = InterfaceHandle,
			.PipeID = PipeID,
			.Buffer = Buffer,
			.BufferLength = BufferLength,
			.LengthTransferred = LengthTransferred,
			.Overlapped = Overlapped };

	// If an overlapped (async) transfer is needed then create a
	// new thread and return immediately.
	if (Overlapped != NULL) {
		ResetEvent(Overlapped->hEvent);
		CreateThread(NULL, 0, (void *)usb_transfer, args, 0, NULL);
		return TRUE;
	} else {
		// Just a synchronous transfer is needed;
		usb_transfer(args);
	}

	return TRUE;
}

// WinUsb_SetPipePolicy implementation.
// Only setting pipe timeout is supported
BOOL WINAPI WinUsb_SetPipePolicy(HANDLE InterfaceHandle, UCHAR PipeID,
				 ULONG PolicyType, ULONG ValueLength,
				 PVOID Value)
{
	if (PolicyType == 0x03) {
		set_timeout(PipeID, *(int *)Value);
	}
	return TRUE;
}

// WinUsb_AbortPipe implementation
BOOL WINAPI WinUsb_AbortPipe(HANDLE InterfaceHandle, UCHAR PipeID)
{
	return TRUE;
}

// WinUsb unused but stubbbed functions.
BOOL WINAPI WinUsb_FlushPipe(HANDLE InterfaceHandle, UCHAR PipeID)
{
	return TRUE;
}

BOOL WINAPI WinUsb_Initialize(HANDLE DeviceHandle, PVOID *InterfaceHandle)
{
	return TRUE;
}

BOOL WINAPI WinUsb_Free(HANDLE InterfaceHandle)
{
	return TRUE;
}


/*============ Minipro replacement functions ==============*/

// USB read implementation. Use the WinUsb_Transfer above
int uread(HANDLE hDevice, unsigned char *data, unsigned int size)
{
	int transferred = 0;
	set_timeout(0x80 | 1, 20000);
	BOOL ret = WinUsb_Transfer(hDevice, 0x80 | 1, data, size,
				   (unsigned *)&transferred, NULL);
	return (ret ? transferred : -1);
}

// USB write implementation. Use the WinUsb_Transfer above
BOOL uwrite(HANDLE hDevice, unsigned char *data, size_t size)
{
	int transferred = 0;
	set_timeout(0x00 | 1, 20000);
	return WinUsb_Transfer(hDevice, 0x00 | 1, data, size,
			       (unsigned *)&transferred, NULL);
}

// USB write to device zero
BOOL usb_write(unsigned char *lpInBuffer, unsigned int nInBufferSize)
{
	return uwrite(0, lpInBuffer, nInBufferSize);
}

// USB read from device zero
int usb_read(unsigned char *lpOutBuffer, unsigned int nBytesToRead,
	     unsigned int nOutBufferSize)
{
	int ret = uread(0, lpOutBuffer, nBytesToRead);
	if (ret == -1)
		MessageBox(GetForegroundWindow(), "Read error!", "TL866",
			   MB_ICONWARNING);
	return ret;
}

// USB write to specified device
BOOL usb_write2(HANDLE hDevice, unsigned char *lpInBuffer,
		unsigned int nInBufferSize)
{
	return uwrite(hDevice, lpInBuffer, nInBufferSize);
}

// USB read from specified device
int usb_read2(HANDLE hDevice, unsigned char *lpOutBuffer,
	      unsigned int nBytesToRead, unsigned int nOutBufferSize)
{
	return uread(hDevice, lpOutBuffer, nBytesToRead);
}


/*=========== Program extension section ==================*/

#define WM_ADD_TOOLS_MENU (WM_APP + 1)
static const char *GitHubUrl = "https://github.com/radiomanV/TL866/tree/master/wine64";

// About dialog
static INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT msg, WPARAM wParam,
				     LPARAM lParam)
{
	static HFONT hFontLinkU = NULL;
	static HCURSOR hCursorHand = NULL;
	switch (msg) {
	case WM_INITDIALOG: {

		// Set hand Cursor
		HWND hLink = GetDlgItem(hDlg, IDC_SHIM_LINK);
		if (!hCursorHand) {
			HCURSOR h = LoadCursorA(NULL, IDC_HAND);
			hCursorHand = h ? h : LoadCursorA(NULL, IDC_ARROW);
		}
		SetClassLongPtrW(hLink, GCLP_HCURSOR, (LONG_PTR)hCursorHand);

		// Set underline font
		HFONT hBase = (HFONT)SendMessageA(hDlg, WM_GETFONT, 0, 0);
		if (!hBase)
			hBase = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		LOGFONT lf = { 0 };
		GetObjectA(hBase, sizeof(lf), &lf);
		lf.lfUnderline = TRUE;
		hFontLinkU = CreateFontIndirectA(&lf);

		// Set version string
		char buf[256];
		if (LoadString(m_hInst, IDS_SHIM_VERSION, buf, 256) && buf[0]) {
			SetDlgItemText(hDlg, IDS_SHIM_VERSION, buf);
		}

		// Set GitHub link
		_snprintf(buf, sizeof(buf), "GitHub: %s", GitHubUrl);
		SetDlgItemTextA(hDlg, IDC_SHIM_LINK, buf);
		return TRUE;
	}

	case WM_CTLCOLORSTATIC: {
		HWND hCtrl = (HWND)lParam;
		if (GetDlgCtrlID(hCtrl) == IDC_SHIM_LINK) {
			HDC hdc = (HDC)wParam;
			SetTextColor(hdc, RGB(0, 0, 238));
			SetBkMode(hdc, TRANSPARENT);
			return (INT_PTR)GetStockObject(NULL_BRUSH);
		}
		break;
	}

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_SHIM_LINK &&
		    HIWORD(wParam) == STN_CLICKED) {
			ShellExecuteA(hDlg, "open", GitHubUrl, NULL, NULL,
				      SW_SHOWNORMAL);
			InvalidateRect(GetDlgItem(hDlg, IDC_SHIM_LINK), NULL,
				       TRUE);
			return TRUE;
		}

		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;

	case WM_DESTROY:
		if (hFontLinkU) {
			DeleteObject(hFontLinkU);
			hFontLinkU = NULL;
		}
		break;
	}
	return FALSE;
}

// WindowProc subclassing
LRESULT CALLBACK ShimWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Intercept program close
	switch (msg) {

	// Add Shim menu
	case WM_ADD_TOOLS_MENU: {
		HMENU hMain = GetMenu(hwnd);
		HMENU hShim = CreatePopupMenu();
		InsertMenuA(hShim, 0, MF_BYPOSITION | MF_STRING, IDM_SHIM_ABOUT,
			    "About USB wrapper...");
		int topCount = GetMenuItemCount(hMain);
		InsertMenuA(hMain, topCount, MF_BYPOSITION | MF_POPUP,
			    (UINT_PTR)hShim, "Shim(&W)");
		DrawMenuBar(hwnd);
		return 0;
	}

	// Intercept menu item click
	case WM_COMMAND: {
		UINT id = LOWORD(wParam);
		if (id == IDM_SHIM_ABOUT) {
			DialogBoxParamA(m_hInst,
					MAKEINTRESOURCE(IDD_SHIM_ABOUTBOX),
					hwnd, AboutDlgProc, 0);
			return 0;
		}
		break;
	}

	case WM_NCDESTROY: {
		LRESULT r = CallWindowProcA(prev_wndproc, hwnd, msg, wParam,
					    lParam);
		SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONG_PTR)prev_wndproc);
		prev_wndproc = NULL;
		return r;
	}

	case WM_QUERYENDSESSION:
	case WM_ENDSESSION:
	case WM_CLOSE:
	case WM_DESTROY:
		ShowWindow(hwnd, 0);
		if (cancel_evt) {
			SetEvent(cancel_evt);
		}
		if (hotplug_thread) {
			WaitForSingleObject(hotplug_thread, 5000);
			CloseHandle(hotplug_thread);
			CloseHandle(cancel_evt);
			cancel_evt = NULL;
			hotplug_thread = NULL;
		}
		break;
	}

	// Call old windowproc
	return CallWindowProcA(prev_wndproc, hwnd, msg, wParam, lParam);
}

/*=========== USB Hotplug detection section ==================*/

// Notifier function. This will force the software to rescan devices.
void device_changed(unsigned int event)
{
	// Initialize the device broadcast interface
	DEV_BROADCAST_DEVICEINTERFACE_W DevBi;
	DevBi.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE_W);
	DevBi.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	DevBi.dbcc_classguid = m_guid;

	// Close all devices
	close_devices();
	Sleep(100);

	// Broadcast a device change message
	SendMessage(hWnd_notify, WM_DEVICECHANGE, event, (LPARAM)&DevBi);
	Sleep(100);

	// Force software to refresh the GUI
	RedrawWindow(hWnd_notify, NULL, NULL, RDW_INVALIDATE);
}

// Hotplug USB monitoring thread
DWORD WINAPI notifier_thread(LPVOID unused)
{
	// Open a persistent event channel to the broker
	struct sockaddr_in a;
	SOCKET sock = get_socket(&a);
	if (sock == INVALID_SOCKET ||
	    connect(sock, (struct sockaddr *)&a, sizeof(a)) != 0) {
		if (sock != INVALID_SOCKET) {
			closesocket(sock);
		}
		printf("USB broker hotplug callback error.\n");
		SendMessage(hWnd_notify, WM_CLOSE, 0, 0);
		return EXIT_FAILURE;
	}

	rpc_hdr_t hdr = { 0 };
	hotplug_sub_req_t req = { 0 };
	req.vid = (uint16_t)device_vid;
	req.pid = (uint16_t)device_pid;
	hdr.type = MSG_HOTPLUG_SUB;
	hdr.version = PROTO_VERSION;
	hdr.len = (uint32_t)sizeof(req);

	if (socket_send(sock, &hdr, sizeof(hdr)) != 0 ||
	    socket_send(sock, &req, sizeof(req)) != 0) {
		printf("USB broker hotplug callback error.\n");
		closesocket(sock);
		SendMessage(hWnd_notify, WM_CLOSE, 0, 0);
		return EXIT_FAILURE;
	}

	printf("\nSubscribed to USB broker hotplug events.\n\n");

	// Enter the monitoring thread and handle hotplug events.
	int rc = EXIT_FAILURE;
	for (;;) {
		// Exit if cancel event is signaled
		if (WaitForSingleObject(cancel_evt, 50) == WAIT_OBJECT_0) {
			rc = EXIT_SUCCESS;
			break;
		}

		fd_set rf, xf;
		FD_ZERO(&rf);
		FD_ZERO(&xf);
		FD_SET(sock, &rf);
		FD_SET(sock, &xf);

		struct timeval tv;
		tv.tv_sec = 3;
		tv.tv_usec = 0;
		int r = select((int)sock + 1, &rf, NULL, NULL, &tv);
		if (r == 0)
			continue;
		if (r < 0) {
			int e = WSAGetLastError();
			if (e == WSAEWOULDBLOCK || e == WSAEINTR)
				continue;
			break;
		}

		rpc_hdr_t evh;
		if (socket_recv(sock, &evh, sizeof(evh)) != 0)
			break;

		if (evh.version != PROTO_VERSION)
			continue;
		if (evh.type != MSG_HOTPLUG_EVT) {
			if (evh.len) {
				char buf[256];
				int read_len = (int)((evh.len <= sizeof(buf)) ?
							     evh.len :
							     sizeof(buf));
				socket_recv(sock, buf, read_len);
			}
			continue;
		}

		//  Receive hotplug event
		hotplug_evt_t evt;
		if (evh.len != sizeof(evt)) {
			if (evh.len) {
				char dump[256];
				int to_read = (int)((evh.len <= sizeof(dump)) ?
							    evh.len :
							    sizeof(dump));
				socket_recv(sock, dump, to_read);
			}
			continue;
		}

		// Exit thread if error
		if (socket_recv(sock, &evt, sizeof(evt)) != 0)
			break;

		// Compare VID/PID and send the device_changed event
		if (evt.vid == device_vid && evt.pid == device_pid) {
			device_changed(evt.arrived ? DBT_DEVICEARRIVAL :
						     DBT_DEVICEREMOVECOMPLETE);
		}
	}

	// Exit from notifier thread
	printf("USB hotplug monitoring thread terminated.\n");
	closesocket(sock);
	if (rc)
		SendMessage(hWnd_notify, WM_CLOSE, 0, 0);
	return rc;
}

// RegisterDeviceNotifications WINAPI replacement
HANDLE WINAPI RegisterDeviceNotifications(HWND hwnd, LPVOID NotificationFilter,
					  DWORD Flags)
{
	// Initialize cancel event
	cancel_evt = CreateEventA(NULL, TRUE, FALSE, NULL);

	// Subclass WindowProc
	hWnd_notify = hwnd;
	if (IsWindow(hwnd) && !prev_wndproc) {
		SetLastError(0);
		LONG_PTR prev = SetWindowLongPtrA(hwnd, GWLP_WNDPROC,
						  (LONG_PTR)ShimWndProc);
		if (prev) {
			prev_wndproc = (WNDPROC)prev;
			PostMessageA(hwnd, WM_ADD_TOOLS_MENU, 0, 0);
		} else if (GetLastError() != 0)
			printf("Subclass failed.\n");
	}

	// Create notifier thread
	HANDLE h = CreateThread(NULL, 0, notifier_thread, NULL, 0, NULL);
	if (!h) {
		printf("Failed to create the hotplug monitoring thread!\n");
		return 0;
	}

	hotplug_thread = h;
	return 0;
}

/*================== Patcher functions ==================*/

// Inline helper patcher functions.
static inline void patch(void *src, void *dest)
{
	// push xxxx, ret; an absolute Jump replacement.
	*(BYTE *)src = X86_PUSH;
	*((DWORD *)((BYTE *)src + 1)) = (DWORD)dest;
	*((BYTE *)src + 5) = X86_RET;
}

static inline uint32_t read_le32(const void *p)
{
	uint32_t v;
	memcpy(&v, p, sizeof(v));
	return v;
}

static inline void *rel32_target(const void *instr, size_t disp_off,
				 size_t next_off)
{
	const uint8_t *ip = (const uint8_t *)instr;
	int32_t rel = (int32_t)read_le32(ip + disp_off);
	uintptr_t next = (uintptr_t)(ip + next_off);
	return (void *)(next + (intptr_t)rel);
}

static inline void *abs32_imm(const void *instr, size_t imm_off)
{
	return (void *)(uintptr_t)read_le32((const uint8_t *)instr + imm_off);
}

// Find signature
static inline uint8_t *find_sig(const uint8_t *code, size_t code_sz,
				const void *pat, size_t pat_sz, ptrdiff_t disp)
{
	uint8_t *p = (uint8_t *)memmem_r(code, code_sz, pat, pat_sz);
	return p ? (p + disp) : NULL;
}

// Dll redirect patch function
BOOL patch_function(char *library, char *func_name, void *custom_func)
{
	DWORD dwOldProtection;
	DWORD func_addr = 0;

	void *BaseAddress = GetModuleHandle(NULL);
	PIMAGE_NT_HEADERS NtHeader =
		(PIMAGE_NT_HEADERS)((PBYTE)BaseAddress +
				    ((PIMAGE_DOS_HEADER)BaseAddress)->e_lfanew);
	PIMAGE_IMPORT_DESCRIPTOR ImpDesc =
		(PIMAGE_IMPORT_DESCRIPTOR)((PBYTE)BaseAddress +
					   NtHeader->OptionalHeader
						   .DataDirectory
							   [IMAGE_DIRECTORY_ENTRY_IMPORT]
						   .VirtualAddress);

	// Search for library in the import directory
	while (ImpDesc->Characteristics && ImpDesc->Name) {
		if (strcasecmp(BaseAddress + ImpDesc->Name, library) == 0) {
			break; // Found it!
		}
		ImpDesc++;
	}

	// check if the library was found in the import directory
	if (!ImpDesc->Characteristics) {
		printf("Library '%s' was not found in the import directory.\n",
			library);
		return FALSE;
	}

	// If the desired library was found we can get the function address
	DWORD_PTR ProcAddress =
		(DWORD_PTR)GetProcAddress(GetModuleHandle(library), func_name);

	// Check if the desired function address was found
	if (!ProcAddress) {
		printf("Function '%s' was not found in '%s' library.\n",
			func_name, library);
		return FALSE;
	}

	// We have the address, let's search it in the thunk table
	PIMAGE_THUNK_DATA thunk =
		(PIMAGE_THUNK_DATA)(BaseAddress + ImpDesc->FirstThunk);
	while (thunk->u1.Function) {
		if ((DWORD_PTR)thunk->u1.Function == ProcAddress) {
			// if an entry is found, patch it to point to our custom function
			MEMORY_BASIC_INFORMATION info;
			VirtualQuery(&thunk->u1.Function, &info,
				     sizeof(MEMORY_BASIC_INFORMATION));
			VirtualProtect(info.BaseAddress, info.RegionSize,
				       PAGE_READWRITE, &dwOldProtection);
			func_addr = thunk->u1.Function;
			thunk->u1.Function = (DWORD_PTR)custom_func;
			VirtualProtect(info.BaseAddress, info.RegionSize,
				       info.Protect, &dwOldProtection);
		}
		thunk++;
	}

	// check if the patch was ok.
	if (!func_addr) {
		printf("Function '%s' was not found in the IAT thunk table.\n",
			func_name);
		return FALSE;
	}
	return TRUE;
}

/* Xgpro patcher function. Called from DllMain.
 * Returns TRUE if patch was ok and continue with
 * program loading or FALSE to exit with error.
 */
BOOL patch_xgpro()
{
	// Get the BaseAddress, NT Header and Image Import Descriptor
	void *base_address = GetModuleHandle(NULL);
	PIMAGE_NT_HEADERS ntheader =
		(PIMAGE_NT_HEADERS)((PBYTE)base_address +
				    ((PIMAGE_DOS_HEADER)base_address)->e_lfanew);

	// Search for version and set the Xgpro GUID and VID/PID
	unsigned char *version;
	if ((version = memmem_r(base_address,
				ntheader->OptionalHeader.SizeOfImage, "Xgpro v",
				7))) {
		// TL866II+, T48, T56 VID/PID and interface GUID
		device_vid = TL866II_VID;
		device_pid = TL866II_PID;
		memcpy(&m_guid, &XGPRO_GUID1, sizeof(GUID));
	} else if ((version = memmem_r(base_address,
				       ntheader->OptionalHeader.SizeOfImage,
				       "Xgpro T76 v", 11))) {
		// T76 VID/PID and interface GUID
		device_vid = T76_VID;
		device_pid = T76_PID;
		memcpy(&m_guid, &XGPRO_GUID2, sizeof(GUID));
	} else {
		return FALSE;
	}
	printf("Found %s\n", version);

	// Patch the Linux incompatible functions
	if (!patch_function("user32.dll", "RegisterDeviceNotificationA",
			    &RegisterDeviceNotifications))
		return FALSE;

	if (!patch_function("winusb.dll", "WinUsb_SetPipePolicy",
			    &WinUsb_SetPipePolicy))
		return FALSE;

	if (!patch_function("winusb.dll", "WinUsb_WritePipe", &WinUsb_Transfer))
		return FALSE;

	if (!patch_function("winusb.dll", "WinUsb_ReadPipe", &WinUsb_Transfer))
		return FALSE;

	if (!patch_function("winusb.dll", "WinUsb_Initialize",
			    &WinUsb_Initialize))
		return FALSE;

	if (!patch_function("winusb.dll", "WinUsb_Free", &WinUsb_Free))
		return FALSE;

	// Searching for functions signature in code section.
	void *p_opendevices = NULL;
	void *p_closedevices = NULL;
	void *p_winusbhandle = NULL;
	void *p_usbhandle = NULL;
	void *p_devicescount = NULL;

	const uint8_t *code = (const uint8_t *)base_address +
			      ntheader->OptionalHeader.BaseOfCode;
	size_t code_sz = ntheader->OptionalHeader.SizeOfCode;

	// Search for open_device function pattern 1 (xgpro < V12.7x)
	uint8_t *p_od1 = find_sig(code, code_sz, &xgpro_open_devices_pattern1,
				  sizeof(xgpro_open_devices_pattern1), 0);

	// Search for open_device function pattern 2 (xgpro > V12.7x)
	uint8_t *p_od2 = find_sig(code, code_sz, &xgpro_open_devices_pattern2,
				  sizeof(xgpro_open_devices_pattern2), 0);

	// If we obtained the most important function address (open_devices) then,
	// we can also calculate the other necessary addresses.
	// Basically we need two function pointers (open_devices and close_devices)
	// which are invoked by Minipro/Xgpro when the program is started/closed
	// or a device is attached or dettached.
	//
	// We also need three data pointers:
	// 1. usb_handle which is used in both Minipro and Xgpro as a handle to a
	// device obtained by calling the CreateFile API. This is actually an array of
	// four pointers which in windows holds a handle to the newly opened device or
	// an invalid handle value. As we redirect the open/close usb functions we
	// initialize each item with an index (0 to 3) or the INVALID_HANDLE_VALUE
	// (0xffffffff) if the coresponding device is not found.
	//
	// 2. winusb_handle used only in Xgpro because Xgpro uses the WinUsb library
	// for USB communications. Like the usb_handle this is actually an array of
	// four pointers which are initialized with an index (0 to 3) or the invalid
	// handle value in our custom open_devices function.
	//
	// 3. devices_count which is used only by Xgpro to know how many devices are
	// available. The Minipro and Xgpro can handle up to four devices while the
	// Xgpro_T76 software can handle only one programmer at this time.

	if (p_od1) {
		uint8_t *open = (uint8_t *)p_od1 - 0x1D;
		p_opendevices = (void *)open;
		p_closedevices = rel32_target(open, 5, 9);
		p_winusbhandle = abs32_imm(p_closedevices, 0x12);
		p_usbhandle = abs32_imm(p_closedevices, 0x02);
		p_devicescount = abs32_imm(open, 0xAF);
	} else if (p_od2) {
		uint8_t *open = (uint8_t *)p_od2 - 0x41;
		p_opendevices = (void *)open;
		p_closedevices = rel32_target(open, 8, 12);
		p_winusbhandle = abs32_imm(p_closedevices, 0x12);
		p_usbhandle = abs32_imm(p_closedevices, 0x02);
		p_devicescount = abs32_imm(open, 0x28);
		if (!patch_function("winusb.dll", "WinUsb_AbortPipe",
				    &WinUsb_AbortPipe))
			return FALSE;

		if (!patch_function("winusb.dll", "WinUsb_FlushPipe",
				    &WinUsb_FlushPipe))
			return FALSE;
	} else {
		printf("Function signatures not found! Unsupported Xgpro version.\n");
		return FALSE;
	}

	// Print debug info.
	printf("Base Address = 0x%p\n", base_address);
	printf("Code section = 0x%p, 0x%lx\n",
		base_address + ntheader->OptionalHeader.BaseOfCode,
		(DWORD)ntheader->OptionalHeader.SizeOfCode);
	printf("Open Devices found at 0x%p\n", p_opendevices);
	printf("Close Devices found at 0x%p\n", p_closedevices);
	printf("Usb Handle found at 0x%p\n", p_usbhandle);
	printf("WinUsb Handle found at 0x%p\n", p_winusbhandle);
	printf("Devices count found at 0x%p\n", p_devicescount);

	// Patch all low level functions in Xgpro.exe to point to our custom
	// functions.
	DWORD dwOldProtection;

	// Initialize the usb_handle, winusb_handle and devices_count pointers
	// These variables are used by Xgpro to handle all opened devices
	usb_handle = p_usbhandle;
	winusb_handle = p_winusbhandle;
	devices_count = p_devicescount;

	// Now this is the actual code patch. So we need to patch the code
	// to redirect the open_devices/close_devices functions to our custom
	// functions. The patch is done by inserting an absolute jump at the
	// desired adress. To do this we need first to change the READ_ONLY
	// attribute of the code section, patch the desired address and then
	// restore the old READ_ONLY attribute.
	// So, we have a self modifying code here.

	// Unprotect the code memory section (make it writable)
	VirtualProtect(base_address + ntheader->OptionalHeader.BaseOfCode,
		       ntheader->OptionalHeader.SizeOfCode, PAGE_READWRITE,
		       &dwOldProtection);

	// patch open_devices function to point to our implementation
	patch(p_opendevices, &open_devices);

	// patch close_devices function to point to our implementation
	patch(p_closedevices, &close_devices);

	// restore the old READ_ONLY protection
	VirtualProtect(base_address + ntheader->OptionalHeader.BaseOfCode,
		       ntheader->OptionalHeader.SizeOfCode, dwOldProtection,
		       &dwOldProtection);
	return TRUE;
}

/* Minipro patcher function. Called from DllMain.
 * Returns TRUE if patch was ok and continue with
 * program loading or FALSE to exit with error.
 */
BOOL patch_minipro()
{
	// Get the BaseAddress, NT Header and Image Import Descriptor
	void *BaseAddress = GetModuleHandle(NULL);
	PIMAGE_NT_HEADERS NtHeader =
		(PIMAGE_NT_HEADERS)((PBYTE)BaseAddress +
				    ((PIMAGE_DOS_HEADER)BaseAddress)->e_lfanew);

	unsigned char *version = memmem_r(BaseAddress,
					  NtHeader->OptionalHeader.SizeOfImage,
					  "MiniPro v", 9);
	if (!version)
		return FALSE;
	printf("Found %s\n", version);

	// Patch the Linux incompatible functions functions
	if (!patch_function("user32.dll", "RegisterDeviceNotificationA",
			    &RegisterDeviceNotifications))
		return FALSE;

	// Searching for functions signature in code section.
	const uint8_t *code = (const uint8_t *)BaseAddress +
			      NtHeader->OptionalHeader.BaseOfCode;
	size_t code_sz = NtHeader->OptionalHeader.SizeOfCode;
	uint8_t *p_opendevices =
		find_sig(code, code_sz, &minipro_open_devices_pattern,
			 sizeof(minipro_open_devices_pattern), -0x28);
	void *p_closedevices = rel32_target(p_opendevices, 4, 8);
	uint8_t *p_usbwrite = find_sig(code, code_sz, &usb_write_pattern,
				       sizeof(usb_write_pattern), -0x0A);
	uint8_t *p_usbwrite2 = find_sig(code, code_sz, &usb_write2_pattern,
					sizeof(usb_write2_pattern), -0x0A);
	uint8_t *p_usbread = find_sig(code, code_sz, &usb_read_pattern,
				      sizeof(usb_read_pattern), 0);
	uint8_t *p_usbread2 = find_sig(code, code_sz, &usb_read2_pattern,
				       sizeof(usb_read2_pattern), 0);
	void *p_usbhandle = abs32_imm(p_closedevices, 1);

	// check if all pointers are o.k.
	if (!p_opendevices || !p_usbwrite || !p_usbwrite2 || !p_usbread ||
	    !p_usbread2) {
		printf("Function signatures not found! Unsupported MiniPro version.\n");
		return FALSE;
	}

	// Search for brick bug. This is not an actually bug but a special code
	// used to brick pirated TL866A/CS devices. The problem is that they
	// used a wrong detection which can also brick genuine TL866A/CS devices
	// See this for more info: https://pastebin.com/i5iLGPs1
	unsigned char *p_brickbug =
		memmem_r(BaseAddress + NtHeader->OptionalHeader.BaseOfCode,
			 NtHeader->OptionalHeader.SizeOfCode, &brickbug_pattern,
			 sizeof(brickbug_pattern));

	// Print some debug info.
	printf("Base Address = 0x%p\n", BaseAddress);
	printf("Code section = 0x%p, 0x%lx\n",
		BaseAddress + NtHeader->OptionalHeader.BaseOfCode,
		(DWORD)NtHeader->OptionalHeader.SizeOfCode);
	printf("Open Devices found at 0x%p\n", p_opendevices);
	printf("Close Devices found at 0x%p\n", p_closedevices);
	printf("Usb Write found at 0x%p\n", p_usbwrite);
	printf("Usb Read found at 0x%p\n", p_usbread);
	printf("Usb Write2 found at 0x%p\n", p_usbwrite2);
	printf("Usb Read2 found at 0x%p\n", p_usbread2);
	printf("Usb Handle found at 0x%p\n", p_usbhandle);
	if (p_brickbug)
		printf("Patched brick bug at 0x%p\n", p_brickbug + 0x08);

	// Patch all low level functions in MiniPro.exe to point to our custom
	// functions.

	// Initialize the usb_handle pointer.
	// Compared to Xgpro software we have only a data pointer here.
	// We initialize each element with a simple index (0 to 3)
	// or the INVALID_HANDLE_VALUE.
	usb_handle = p_usbhandle;

	// Now this is the actual code patch. So we need to patch the code
	// to redirect all the usb realated functions as well as open/close devices
	// functions to point to our custom implementation.
	// functions. The patch is done by inserting an absolute jump at the
	// desired adress. To do this we need first to change the READ_ONLY
	// attribute of the code section, patch the desired address and then
	// restore the old READ_ONLY attribute.
	// So, we have a self modifying code here.

	// Unprotect the code memory section (make it writable)
	DWORD dwOldProtection;
	VirtualProtect(BaseAddress + NtHeader->OptionalHeader.BaseOfCode,
		       NtHeader->OptionalHeader.SizeOfCode, PAGE_READWRITE,
		       &dwOldProtection);

	// patch open_devices function
	patch(p_opendevices, &open_devices);

	// patch close_devices function
	patch(p_closedevices, &close_devices);

	// patch usb_write function
	patch(p_usbwrite, &usb_write);

	// patch usb_read function
	patch(p_usbread, &usb_read);

	// patch usb_write2 function
	patch(p_usbwrite2, &usb_write2);

	// patch usb_read2 function
	patch(p_usbread2, &usb_read2);

	// patch the brick bug
	if (p_brickbug)
		*(p_brickbug + 0x08) = X86_JMP;

	// Restore the old READ_ONLY protection
	VirtualProtect(BaseAddress + NtHeader->OptionalHeader.BaseOfCode,
		       NtHeader->OptionalHeader.SizeOfCode, dwOldProtection,
		       &dwOldProtection);

	// Set the Minipro GUID
	memcpy(&m_guid, &MINIPRO_GUID, sizeof(GUID));

	// Set the VID/PID
	device_vid = TL866A_VID;
	device_pid = TL866A_PID;
	return TRUE;
}

// Parse environment variables
void get_env_variables(void)
{
	debug = 0;
	broker_port = RPC_PORT_DEFAULT;

	// TL_DEBUG
	const char *dbg = getenv("TL_DEBUG");
	if (dbg && *dbg) {
		if (!strncmp(dbg, "1", 1))
			debug = 1;
		else if (!strncmp(dbg, "2", 1))
			debug = 2;
		else if (!strncmp(dbg, "3", 1))
			debug = 3;
		else
			debug = 0;
	}

	// BROKER_PORT
	const char *port = getenv("BROKER_PORT");
	if (port && *port) {
		char *end;
		errno = 0;
		unsigned long val = strtoul(port, &end, 10);
		if (errno == 0 && *end == '\0' && val > 0 && val <= USHRT_MAX) {
			broker_port = (unsigned short)val;
		} else {
			fprintf(stderr,
				"Invalid BROKER_PORT='%s' (expected 1-65535)\n",
				port);
		}
	}
}

// Machine to string helper
static const char *machine_to_str(USHORT m)
{
	switch (m) {
	case 0x8664:
		return "AMD64";
	case 0x014c:
		return "x86";
	case 0xAA64:
		return "ARM64";
	case 0x01c4:
		return "ARM";
	case 0x0000:
		return "NATIVE";
	default:
		return "Unknown";
	}
}

// Read registry
static int reg_read_sz(HKEY root, const char *subkey, const char *name,
		       char *out, DWORD out_sz)
{
	HKEY hkey;
	DWORD type = 0, sz = out_sz;
	if (RegOpenKeyEx(root, subkey, 0, KEY_READ, &hkey) != ERROR_SUCCESS)
		return 0;
	int ok = (RegQueryValueEx(hkey, name, NULL, &type, (LPBYTE)out, &sz) ==
			  ERROR_SUCCESS &&
		  type == REG_SZ);
	RegCloseKey(hkey);
	return ok;
}

// Detect Windows and Wine version
void detect_wine_version()
{
	typedef LONG(WINAPI * RtlGetVersion_t)(OSVERSIONINFO *);
	typedef BOOL(WINAPI * IsWow64Process2_t)(HANDLE, USHORT *, USHORT *);
	typedef BOOL(WINAPI * IsWow64Process_t)(HANDLE, PBOOL);
	typedef VOID(WINAPI * GetNativeSystemInfo)(LPSYSTEM_INFO);
	typedef const char *(*wine_get_version_t)(void);
	RtlGetVersion_t pRtlGetVersion = NULL;

	HMODULE ntdll = GetModuleHandle("ntdll.dll");
	HMODULE k32 = GetModuleHandle("kernel32.dll");
	if (!ntdll || !k32)
		return;

	printf("\n------------Wine info-------------------\n");
	pRtlGetVersion =
		(RtlGetVersion_t)GetProcAddress(ntdll, "RtlGetVersion");
	wine_get_version_t wine_get_version =
		(wine_get_version_t)(uintptr_t)GetProcAddress(
			ntdll, "wine_get_version");

	if (wine_get_version) {
		printf("Detected Wine version: %s\n", wine_get_version());
	}

	OSVERSIONINFO osv = { 0 };
	osv.dwOSVersionInfoSize = sizeof(osv);
	if (pRtlGetVersion && pRtlGetVersion(&osv) == 0) {
		wprintf(L"Windows version: %lu.%lu (build %lu)\n",
			osv.dwMajorVersion, osv.dwMinorVersion,
			osv.dwBuildNumber);
	} else {
		if (GetVersionExA(&osv)) {
			wprintf(L"(Windows version: %lu.%lu (build %lu)\n",
				osv.dwMajorVersion, osv.dwMinorVersion,
				osv.dwBuildNumber);
		}
	}

	char pname[256] = "";
	const char *cv = "Software\\Microsoft\\Windows NT\\CurrentVersion";
	if (reg_read_sz(HKEY_LOCAL_MACHINE, cv, "ProductName", pname,
			sizeof(pname)))
		printf("Product Name: %s\n", pname);

	IsWow64Process2_t pIsWow64Process2 =
		GetProcAddress(k32, "IsWow64Process2");

	if (pIsWow64Process2) {
		USHORT pm = 0, nm = 0;
		if (pIsWow64Process2(GetCurrentProcess(), &pm, &nm)) {
			BOOL wow64 = (pm != 0);
			USHORT proc_m = (pm != 0) ? pm : nm;

			printf("Arch (process/native): %s / %s%s\n",
			       machine_to_str(proc_m), machine_to_str(nm),
			       wow64 ? " (WOW64)" : "");
		}
	} else {
		IsWow64Process_t pIsWow64Process =
			GetProcAddress(k32, "IsWow64Process");
		GetNativeSystemInfo pGetNativeSystemInfo =
			(void *)GetProcAddress(k32, "GetNativeSystemInfo");

		SYSTEM_INFO si;
		ZeroMemory(&si, sizeof(si));
		if (pGetNativeSystemInfo)
			pGetNativeSystemInfo(&si);
		else
			GetSystemInfo(&si);

		BOOL wow64 = FALSE;
		if (pIsWow64Process)
			(void)pIsWow64Process(GetCurrentProcess(), &wow64);

		USHORT proc_m = (sizeof(void *) == 8) ? 0x8664 : 0x014c;
		const char *native_str = "Unknown";
		switch (si.wProcessorArchitecture) {
		case PROCESSOR_ARCHITECTURE_AMD64:
			native_str = "AMD64";
			break;
		case PROCESSOR_ARCHITECTURE_INTEL:
			native_str = "x86";
			break;
		case PROCESSOR_ARCHITECTURE_ARM64:
			native_str = "ARM64";
			break;
		case PROCESSOR_ARCHITECTURE_ARM:
			native_str = "ARM";
			break;
		}

		printf("Arch (process/native): %s / %s%s\n",
		       machine_to_str(proc_m), native_str,
		       wow64 ? " (WOW64)" : "");
	}
}

// Detect comctl32 and toolbar rendering bug in Wine
void detect_comctl(void)
{
	DWORD maj = 0, min = 0;
	HMODULE hComctl = LoadLibraryA("comctl32.dll");
	if (hComctl) {
		typedef HRESULT(CALLBACK * PFN_DLLGETVERSION)(DLLVERSIONINFO *);
		PFN_DLLGETVERSION p = (PFN_DLLGETVERSION)GetProcAddress(
			hComctl, "DllGetVersion");
		if (p) {
			DLLVERSIONINFO dvi = { 0 };
			dvi.cbSize = sizeof(dvi);
			if (SUCCEEDED(p(&dvi))) {
				maj = dvi.dwMajorVersion;
				min = dvi.dwMinorVersion;
			}
		}
		FreeLibrary(hComctl);
	}

	printf("comctl32 version: %lu.%lu\n", (unsigned long)maj,
	       (unsigned long)min);
	int theme_on = 0;
	HMODULE hUx = LoadLibraryA("uxtheme.dll");
	if (hUx) {
		BOOL(WINAPI * is_themed)(void) =
			(void *)GetProcAddress(hUx, "IsAppThemed");
		BOOL(WINAPI * is_theme_active)(void) =
			(void *)GetProcAddress(hUx, "IsThemeActive");
		theme_on = (is_themed && is_theme_active) ?
				   (is_themed() && is_theme_active()) :
				   0;
		FreeLibrary(hUx);
	}

	if (theme_on && !(maj == 5 && min == 80)) {
		printf("WARN: Theme enabled and comctl32 != 5.80; toolbar may render incorrectly.\n"
		       "Run `winetricks comctl32` (native 5.80) or disable the theme.\n");
	}
}

/*================== DllMain ====================*/
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	// Get the module name (should be shim.dll)
	wchar_t buffer[MAX_PATH];
	GetModuleFileNameW(hinstDLL, buffer, sizeof(buffer));
	const wchar_t *module_name = PathFindFileNameW(buffer);
	if (!module_name) {
		module_name = L"shim.dll";
	}

	switch (fdwReason) {

	// Dll loaded and atached to process
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hinstDLL);
		m_hInst = hinstDLL;

		// Attach console to our parrent process
		AttachConsole(ATTACH_PARENT_PROCESS);
		freopen("CONOUT$", "w", stdout);
		setvbuf(stdout, NULL, _IONBF, 0);
		freopen("CONOUT$", "w", stderr);
		setvbuf(stderr, NULL, _IONBF, 0);
		printf("%ls loaded\n", module_name);

		// Parse environment variables
		get_env_variables();

		// Detect Wine version
		detect_wine_version();

		// Detect comctl32 version
		detect_comctl();

		// Initialize broker event
		InterlockedExchange(&broker_state, BRK_STATE_INIT);
		broker_evt = CreateEvent(NULL, TRUE, FALSE, NULL);

		// Try to patch the software
		printf("\n------------Software--------------------\n");
		if ((patch_xgpro() || patch_minipro()) && !wsa_init()){
			printf("\n------------Devices----------------------\n");
			printf("Using USB broker port: %u\n", broker_port);
			return TRUE;
		}
		printf("%ls unloaded\n", module_name);
		return FALSE;

	// We are detached from a process, terminate hotplug thread and exit.
	case DLL_PROCESS_DETACH:
		printf("%ls unloaded\n", module_name);
		HANDLE h = (HANDLE)InterlockedExchangePointer(
			(PVOID *)broker_evt, NULL);
		CloseHandle(h);
		WSACleanup();
	}
	return TRUE;
}
