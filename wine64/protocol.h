/*
 * protocol.h
 * USB broker protocol call definitions
 * Created: September 10, 2025
 * Author: radiomanV
 */

#pragma once
#include <stdint.h>

#if defined(_MSC_VER)
#define PACKED
#pragma pack(push, 1)
#else
#define PACKED __attribute__((__packed__))
#endif

#define PROTO_VERSION	 2
#define RPC_PORT_DEFAULT 35866

// ---- MSG ----
enum msg_type {
	MSG_ENUMERATE = 1,
	MSG_OPEN,
	MSG_CLOSE,
	MSG_BULK,
	MSG_CANCEL,
	MSG_HOTPLUG_SUB,
	MSG_HOTPLUG_EVT,
};

// ---- RPC HEADER ----
typedef struct PACKED {
	uint32_t len;
	uint16_t type;
	uint16_t version;
} rpc_hdr_t;

// --- DEVICE INFO ---
typedef struct PACKED {
	uint16_t vid, pid;
	uint8_t bus, addr;
	uint8_t ifnum;
	uint8_t has_eps;
	uint16_t wMaxPacketSize[16];
	char path[128];
} dev_info_t;

// ---- ENUMERATE ----
typedef struct PACKED {
	int32_t status;
	uint32_t count;
	dev_info_t devs[0];
} enum_resp_t;

// ---- OPEN ----
typedef struct PACKED {
	char path[128];
} open_req_t;

typedef struct PACKED {
	int32_t status;
	uint32_t handle_id;
	uint8_t speed;
	char product[64];
} open_resp_t;

// ---- CLOSE ----
typedef struct PACKED {
	uint32_t handle_id;
} close_req_t;

typedef struct PACKED {
	int32_t status;
} close_resp_t;

// ---- BULK ----
typedef struct PACKED {
	uint32_t handle_id;
	uint8_t ep;
	uint32_t timeout_ms;
	uint32_t len;
} bulk_req_t;

typedef struct PACKED {
	int32_t status;
	uint32_t rx_len;
	uint8_t data[0];
} bulk_resp_t;

// ---- CANCEL ----
typedef struct PACKED {
	uint32_t handle_id;
} cancel_req_t;

typedef struct PACKED {
	int32_t status;
} cancel_resp_t;

// ---- HOTPLUG ----
typedef struct PACKED {
	uint16_t vid, pid;
} hotplug_sub_req_t;

typedef struct PACKED {
	uint8_t arrived;
	uint16_t vid, pid;
	uint8_t bus, addr;
} hotplug_evt_t;

typedef enum rpc_err_t {
	BR_E_WSA               = -1,
	BR_E_SOCK              = -2,
	BR_E_CONNECT_IMMEDIATE = -3,
	BR_E_CONNECT_SOERR     = -4,
	BR_E_CONNECT_TIMEOUT   = -5,
	BRK_E_READY_TIMEOUT    = -50,
	BRK_E_READY_FAILED     = -51,
	BRK_E_WAIT_TIMEOUT     = -52,
	RPC_E_RESP_LEN_NULL    = -100,
	RPC_E_RESP_BUF_NULL    = -101,
	RPC_E_MKSOCK_FAIL      = -102,
	RPC_E_CONNECT_TIMEOUT  = -103,
	RPC_E_CONNECT_FAIL     = -104,
	RPC_E_SEND_HEADER      = -105,
	RPC_E_SEND_REQ         = -106,
	RPC_E_SEND_EXTRA       = -107,
	RPC_E_RECV_HEADER      = -108,
	RPC_E_PROTO_MISMATCH   = -109,
	RPC_E_RESP_TOO_LARGE   = -110,
	RPC_E_RECV_PAYLOAD     = -111,
	RPC_E_RECV_ERR         = -112,
	RPC_E_SEND_ERR         = -113,
	RPC_E_OPEN_BUSY        = -114,
	RPC_E_OPEN_ERR         = -115,
} rpc_err_t;

#if defined(_MSC_VER)
#pragma pack(pop)
#endif
#undef PACKED
