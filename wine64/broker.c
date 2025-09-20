/*
 * broker.c
 * Minimal USB broker for shim.dll.
 * The USB broker will redirect all RPC calls from the shim to the
 * Linux USB subsystem using LibUSB calls.
 * Created: September 10, 2025
 * Author: radiomanV
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>
#include <libusb-1.0/libusb.h>

#include "protocol.h"

typedef struct {
	uint32_t id;
	libusb_device_handle *h;
	uint8_t ifnum;
	char path[128];
} handle_entry_t;

typedef struct {
	int sock;
	int running;
	libusb_hotplug_callback_handle cb;
} hotplug_sub_t;

static handle_entry_t libusb_map[64];
static pthread_mutex_t map_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t id_mutex = PTHREAD_MUTEX_INITIALIZER;
static int map_idx = 0;
static uint32_t map_next_id = 1;
static volatile int num_clients = 0;
static volatile int b_verbose = 1;
static volatile int b_sigint = 0;

#define LOG(fmt, ...)                                             \
	do {                                                          \
		if (b_verbose)                                            \
			fprintf(stderr, "[broker] " fmt "\n", ##__VA_ARGS__); \
	} while (0)

#define ERR(fmt, ...) fprintf(stderr, "[broker:ERR] " fmt "\n", ##__VA_ARGS__)

// Socket buffer send function
static int socket_send(int sock, const void *buf, int len)
{
	const char *pbuf = (const char *)buf;
	int done = 0;
	while (done < len) {
		int n = send(sock, pbuf + done, len - done, 0);
		if (n <= 0)
			return RPC_E_SEND_ERR;
		done += n;
	}
	return EXIT_SUCCESS;
}

// Socket buffer receive function
static int socket_recv(int sock, void *buf, int len)
{
	char *p = (char *)buf;
	int rcvd = 0;
	while (rcvd < len) {
		int n = recv(sock, p + rcvd, len - rcvd, 0);
		if (n <= 0)
			return RPC_E_RECV_ERR;
		rcvd += n;
	}
	return EXIT_SUCCESS;
}

// Socket reply function
static int send_reply(int s, uint16_t type, const void *payload, uint32_t plen)
{
	rpc_hdr_t h = { .len = plen, .type = type, .version = PROTO_VERSION };
	if (socket_send(s, &h, sizeof(h)))
		return RPC_E_SEND_ERR;
	if (plen && socket_send(s, payload, (int)plen))
		return RPC_E_SEND_ERR;
	return EXIT_SUCCESS;
}

// Helpers to store/read/delete a handle_entry
static int map_put(libusb_device_handle *h, uint8_t ifnum, uint32_t *out_id,
		   const char *path)
{
	pthread_mutex_lock(&map_mutex);
	if (map_idx >= (int)(sizeof(libusb_map) / sizeof(libusb_map[0]))) {
		pthread_mutex_unlock(&map_mutex);
		return EXIT_FAILURE;
	}

	uint32_t id;
	pthread_mutex_lock(&id_mutex);
	id = map_next_id++;
	if (id == 0)
		id = map_next_id++;
	pthread_mutex_unlock(&id_mutex);

	libusb_map[map_idx] =
		(handle_entry_t){ .id = id, .h = h, .ifnum = ifnum };
	strncpy(libusb_map[map_idx].path, path, sizeof(libusb_map[0].path));
	map_idx++;

	pthread_mutex_unlock(&map_mutex);
	*out_id = id;
	return EXIT_SUCCESS;
}

static handle_entry_t *map_get(uint32_t id)
{
	for (int i = 0; i < map_idx; i++) {
		if (libusb_map[i].id == id)
			return &libusb_map[i];
	}
	return NULL;
}

static int map_del(uint32_t id)
{
	pthread_mutex_lock(&map_mutex);
	for (int i = 0; i < map_idx; i++) {
		if (libusb_map[i].id == id) {
			libusb_map[i] = libusb_map[map_idx - 1];
			map_idx--;
			pthread_mutex_unlock(&map_mutex);
			return EXIT_SUCCESS;
		}
	}
	pthread_mutex_unlock(&map_mutex);
	return EXIT_FAILURE;
}

// LibUSB enumerate helper
static int usb_enum(dev_info_t **out_list, uint32_t *out_cnt)
{
	libusb_device **list = NULL;
	ssize_t n = libusb_get_device_list(NULL, &list);
	if (n < 0)
		return EXIT_FAILURE;

	uint32_t cap = (uint32_t)(n > 512 ? 512 : n);
	dev_info_t *arr = (dev_info_t *)calloc(cap, sizeof(dev_info_t));
	if (!arr) {
		libusb_free_device_list(list, 1);
		return EXIT_FAILURE;
	}

	uint32_t cnt = 0;
	for (ssize_t i = 0; i < n && cnt < cap; i++) {
		libusb_device *dev = list[i];
		struct libusb_device_descriptor dd;
		if (libusb_get_device_descriptor(dev, &dd))
			continue;
		dev_info_t di = { 0 };
		di.vid = dd.idVendor;
		di.pid = dd.idProduct;
		di.bus = libusb_get_bus_number(dev);
		di.addr = libusb_get_device_address(dev);
		di.ifnum = 0;
		snprintf(di.path, sizeof(di.path), "usb:%u-%u", di.bus,
			 di.addr);
		struct libusb_config_descriptor *cfg = NULL;
		if (libusb_get_active_config_descriptor(dev, &cfg)) {
			di.has_eps = 0;
			memset(di.wMaxPacketSize, 0, sizeof(di.wMaxPacketSize));
		} else {
			di.has_eps = 1;
			memset(di.wMaxPacketSize, 0, sizeof(di.wMaxPacketSize));
			for (int i = 0; i < cfg->bNumInterfaces; i++) {
				const struct libusb_interface *it =
					&cfg->interface[i];
				for (int a = 0; a < it->num_altsetting; a++) {
					const struct libusb_interface_descriptor
						*id = &it->altsetting[a];
					for (int k = 0; k < id->bNumEndpoints;
					     k++) {
						const struct libusb_endpoint_descriptor
							*ep = &id->endpoint[k];
						uint8_t num =
							ep->bEndpointAddress &
							0x0F;
						if (num < 16)
							di.wMaxPacketSize[num] =
								ep->wMaxPacketSize;
					}
				}
			}
		}
		libusb_free_config_descriptor(cfg);

		arr[cnt++] = di;
	}
	libusb_free_device_list(list, 1);
	*out_list = arr;
	*out_cnt = cnt;
	return EXIT_SUCCESS;
}

// LibUSB open_device RPC helper
static libusb_device_handle *open_device(const char *path, open_resp_t *resp)
{
	if (resp) {
		resp->status = RPC_E_OPEN_ERR;
		resp->product[0] = '\0';
		resp->speed = LIBUSB_SPEED_UNKNOWN;
		resp->handle_id = 0;
	}

	int wa = 0, wb = 0;
	if (!path)
		return NULL;
	if (strcmp(path, "usb:auto-first") == 0) {
		wa = -1, wb = -1;
	} else if (sscanf(path, "usb:%d-%d", &wb, &wa) != 2) {
		return NULL;
	}

	libusb_device **list = NULL;
	ssize_t n = libusb_get_device_list(NULL, &list);
	if (n < 0)
		return NULL;

	libusb_device_handle *hnd = NULL;

	for (ssize_t i = 0; i < n; i++) {
		libusb_device *d = list[i];
		if ((wb == -1 && wa == -1) ||
		    (libusb_get_bus_number(d) == wb &&
		     libusb_get_device_address(d) == wa)) {
			if (!libusb_open(d, &hnd)) {
				if (resp) {
					libusb_device *dev =
						libusb_get_device(hnd);
					if (dev) {
						resp->speed =
							libusb_get_device_speed(
								dev);

						struct libusb_device_descriptor
							dd;
						if (libusb_get_device_descriptor(
							    dev, &dd) ==
							    LIBUSB_SUCCESS &&
						    dd.iProduct) {
							unsigned char tmp[sizeof(
								resp->product)];
							int m = libusb_get_string_descriptor_ascii(
								hnd,
								dd.iProduct,
								tmp,
								sizeof(tmp) -
									1);
							if (m > 0) {
								tmp[m] = 0;
								strncpy(resp->product,
									(const char
										 *)
										tmp,
									sizeof(resp->product) -
										1);
								resp->product
									[sizeof(resp->product) -
									 1] = 0;
							}
						}
					}
				}
				uint32_t id = 0;
				if (map_put(hnd, 0, &id, path) == 0) {
					if (resp) {
						resp->handle_id = id;
					}
					int cfg = 0;
					if (libusb_get_configuration(hnd,
								     &cfg) ==
						    LIBUSB_SUCCESS &&
					    cfg == 0) {
						libusb_set_configuration(hnd,
									 1);
					}
					if (libusb_claim_interface(hnd, 0) ==
					    LIBUSB_SUCCESS)
						resp->status = 0;
					else
						resp->status = RPC_E_OPEN_BUSY;
					break;
				} else {
					libusb_close(hnd);
					hnd = NULL;
				}
			}
			if (wb != -1 || wa != -1)
				break;
		}
	}

	libusb_free_device_list(list, 1);
	return hnd;
}

/* ------------ Client hotplug subscription socket notifier -------------*/
static int LIBUSB_CALL on_hotplug(libusb_context *ctx, libusb_device *dev,
				  libusb_hotplug_event event, void *user)
{
	hotplug_sub_t *sub = (hotplug_sub_t *)user;
	if (!sub || sub->sock < 0)
		return 0;

	struct libusb_device_descriptor desc;
	if (libusb_get_device_descriptor(dev, &desc))
		return 0;

	hotplug_evt_t evt = { 0 };
	evt.arrived = (event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED) ? 1 : 0;
	evt.vid = desc.idVendor;
	evt.pid = desc.idProduct;
	evt.bus = libusb_get_bus_number(dev);
	evt.addr = libusb_get_device_address(dev);

	// Send reply to client
	if (send_reply(sub->sock, MSG_HOTPLUG_EVT, &evt, sizeof(evt)) != 0) {
		sub->running = 0;
	}
	return 0;
}

// LibUSB hotplug thread loop
static void *hotplug_loop(void *arg)
{
	hotplug_sub_t *sub = (hotplug_sub_t *)arg;
	struct timeval tv = { .tv_sec = 0, .tv_usec = 250000 };
	while (sub->running) {
		int rc =
			libusb_handle_events_timeout_completed(NULL, &tv, NULL);
		if (rc != 0)
			usleep(1000);
		char tmp;
		int n = recv(sub->sock, &tmp, 1, MSG_PEEK | MSG_DONTWAIT);
		if (n == 0) {
			sub->running = 0;
			break;
		}
	}
	return NULL;
}

/* ---------- Client worker thread ---------------*/
static void *client_worker(void *arg)
{
	//LOG("client connected (fd=%d)", sock);

	int sock = (int)(intptr_t)arg;
	for (;;) {
		rpc_hdr_t hdr;
		if (socket_recv(sock, &hdr, sizeof(hdr)))
			break;
		if (hdr.version != PROTO_VERSION) {
			ERR("version mismatch: %u", hdr.version);
			break;
		}

		uint8_t *request = NULL;
		if (hdr.len) {
			request = (uint8_t *)malloc(hdr.len);
			if (!request)
				break;
			if (socket_recv(sock, request, (int)hdr.len)) {
				free(request);
				break;
			}
		}

		uint8_t buf[1 << 16];
		uint32_t len = 0;
		switch (hdr.type) {

		// Enumerate USB devices RPC
		case MSG_ENUMERATE: {
			dev_info_t *list = NULL;
			uint32_t cnt = 0;
			enum_resp_t *resp = NULL;
			int rc = usb_enum(&list, &cnt);
			LOG("Enumerate: %u USB devices found.", cnt);
			if (rc == 0) {
				len = sizeof(enum_resp_t) +
				      cnt * sizeof(dev_info_t);
				resp = (enum_resp_t *)malloc(len);
				if (!resp) {
					if (list)
						free(list);
					break;
				}
				resp->status = 0;
				resp->count = cnt;
				memcpy((uint8_t *)resp + sizeof(*resp), list,
				       cnt * sizeof(dev_info_t));
				free(list);
				send_reply(sock, MSG_ENUMERATE, resp, len);
				free(resp);
				resp = NULL;
				request = NULL;
				continue;
			} else {
				enum_resp_t b = { .status = -1, .count = 0 };
				send_reply(sock, MSG_ENUMERATE, &b, sizeof(b));
				if (request)
					free(request);
				continue;
			}
		} break;

		// Open device RPC
		case MSG_OPEN: {
			const open_req_t *rq = (const open_req_t *)request;
			open_resp_t resp;
			memset(&resp, 0, sizeof(resp));
			resp.handle_id = 0;
			resp.speed = 0;
			resp.product[0] = '\0';
			open_device(rq->path, &resp);
			if (!resp.status) {
				LOG("Open device: %s", rq->path);
			}
			send_reply(sock, MSG_OPEN, &resp, sizeof(resp));
		} break;

		// Close device RPC
		case MSG_CLOSE: {
			const close_req_t *rq = (const close_req_t *)request;
			close_resp_t resp = { .status = -1 };
			handle_entry_t *ent = map_get(rq->handle_id);
			if (ent && ent->h) {
				LOG("Close device: %s", ent->path);
				libusb_release_interface(ent->h, ent->ifnum);
				libusb_close(ent->h);
				map_del(rq->handle_id);
				resp.status = 0;
			}
			send_reply(sock, MSG_CLOSE, &resp, sizeof(resp));
		} break;

		// Bulk transfer RPC
		case MSG_BULK: {
			const bulk_req_t *rq = (const bulk_req_t *)request;
			handle_entry_t *ent = map_get(rq->handle_id);
			LOG("%s %s Bulk transfer: %u bytes", ent->path,
			    rq->ep & 0x80 ? "IN" : "OUT", rq->len);

			int ep_in = ((rq->ep & 0x80) != 0);
			int rc = -1, xfer = 0;
			uint8_t *payload = NULL;
			size_t pay_len = 0;
			size_t hdr_len = sizeof(*rq);

			if (!ep_in) {
				payload = request + hdr_len;
				pay_len = (hdr.len > hdr_len) ?
						  (hdr.len - hdr_len) :
						  0;
			} else {
				payload = buf + sizeof(bulk_resp_t);
				pay_len = rq->len;
			}

			if (ent && ent->h) {
				if (hdr.type == MSG_BULK) {
					rc = libusb_bulk_transfer(
						ent->h, rq->ep, payload,
						(int)pay_len, &xfer,
						rq->timeout_ms);
				} else {
					rc = libusb_interrupt_transfer(
						ent->h, rq->ep, payload,
						(int)pay_len, &xfer,
						rq->timeout_ms);
				}
			}

			bulk_resp_t *resp = (bulk_resp_t *)buf;
			resp->status = rc;
			resp->rx_len = (ep_in && rc == 0) ? (uint32_t)xfer : 0;
			len = sizeof(*resp) + resp->rx_len;
			send_reply(sock, hdr.type, buf, len);
		} break;

		// Cancel transfer RPC
		case MSG_CANCEL: {
			LOG("Cancel transfer");
			cancel_resp_t resp = { .status = 0 };
			send_reply(sock, MSG_CANCEL, &resp, sizeof(resp));
		} break;

		// Hotplug subscribe RPC
		case MSG_HOTPLUG_SUB: {
			LOG("Hotplug subcribe");
			hotplug_sub_t sub = { .sock = sock, .running = 1 };
			int rc = libusb_hotplug_register_callback(
				NULL,
				LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED |
					LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT,
				0, LIBUSB_HOTPLUG_MATCH_ANY,
				LIBUSB_HOTPLUG_MATCH_ANY,
				LIBUSB_HOTPLUG_MATCH_ANY, on_hotplug, &sub,
				&sub.cb);
			if (rc != LIBUSB_SUCCESS) {
				ERR("hotplug not available: %s",
				    libusb_error_name(rc));
			} else {
				pthread_t th;
			    // GCC atomic increment builtin
			    __sync_add_and_fetch(&num_clients, 1);
				pthread_create(&th, NULL, hotplug_loop, &sub);
				pthread_detach(th);
				while (sub.running) {
					char tmp;
					int n = recv(sock, &tmp, 1,
						     MSG_PEEK | MSG_DONTWAIT);
					if (n == 0) {
						sub.running = 0;
						break;
					}
					usleep(100000);
				}
				libusb_hotplug_deregister_callback(NULL, sub.cb);
				if (request)
					free(request);
			    // GCC atomic decrement builtin
				__sync_sub_and_fetch(&num_clients, 1);
				goto done;
			}
		} break;

		default:
			ERR("unknown msg type: 0x%04x", hdr.type);
			break;
		}

		if (request)
			free(request);
	}

done:
	close(sock);
	//LOG("client disconnected (fd=%d)", sock);
	return NULL;
}

// SIGINT handler
static void sigint_handler(int sig)
{
	LOG("SIGINT");
	b_sigint = 1;
}

/*-------------Main loop------------------------------*/
int main(int argc, char **argv)
{
	int port = RPC_PORT_DEFAULT;
	int no_exit = 0;

	// Parse arguments
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--port") && i + 1 < argc) {
			port = atoi(argv[++i]);
		} else if (!strcmp(argv[i], "--quiet")) {
			b_verbose = 0;
		} else if (!strcmp(argv[i], "--no-exit")) {
			no_exit = 1;
		} else if (!strcmp(argv[i], "--help")) {
			fprintf(stderr, "usb-broker options:\n");
			fprintf(stderr, "  --port N	listen port (default %d)\n",
				RPC_PORT_DEFAULT);
			fprintf(stderr,
				"  --no-exit	don't exit when no clients\n");
			fprintf(stderr, "  --quiet	less logs\n");
			return 0;
		}
	}

	// Initialize LibUSB
	if (libusb_init(NULL) != 0) {
		ERR("libusb_init failed");
		return EXIT_FAILURE;
	}

	// Initialize SIGINT handler
	signal(SIGINT, sigint_handler);

	// Initialize broker server
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket");
		return EXIT_FAILURE;
	}
	int on = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		return EXIT_FAILURE;
	}
	if (listen(sock, 32) < 0) {
		perror("listen");
		return EXIT_FAILURE;
	}

	// Set the non-blocking flag
	int flags = fcntl(sock, F_GETFL, 0) | O_NONBLOCK;
	fcntl(sock, F_SETFL, flags);
	LOG("listening on 127.0.0.1:%d", port);

	// Enter client wait loop
	int wait = 0;
	do {
		struct timeval tv = { .tv_sec = 3, .tv_usec = 0 };
		fd_set rf;
		FD_ZERO(&rf);
		FD_SET(sock, &rf);
		int rc = select(sock + 1, &rf, NULL, NULL, &tv);
		if (rc < 0) {
			if (errno == EINTR)
				continue;
			perror("select");
			break;
		}

		// Accept and create a new client worker thread
		if (rc > 0 && FD_ISSET(sock, &rf)) {
			int cs = accept(sock, NULL, NULL);
			if (cs >= 0) {
				pthread_t th;
				pthread_create(&th, NULL, client_worker,
					       (void *)(intptr_t)cs);
				pthread_detach(th);
				wait = 3;
			}
		}
		if(wait) wait--;
	} while ((num_clients || no_exit || wait) && !b_sigint);

	close(sock);
	libusb_exit(NULL);
	return EXIT_SUCCESS;
}
