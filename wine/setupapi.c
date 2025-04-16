/*
 * setupapi.c
 * Winelib wrapper for Minipro TL866A/CS, TL866II+, XgPro T48, T56 and T76
 * programmers.
 * This library will redirect all USB related functions from Minipro or Xgpro
 * software to the Linux USB subsystem using the standard LibUsb library.
 * Created: May 5, 2014
 * Author: radiomanV
 */

#define __WINESRC__
#define __CYGWIN__
#define _GNU_SOURCE

#include <glob.h>
#ifdef UDEV
#include <libudev.h>
#endif
#include <libusb-1.0/libusb.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <dbt.h>
#include <winbase.h>
#include <windef.h>
#include <winnt.h>

// Defines
#define TL866A_VID 0x04d8
#define TL866A_PID 0xe11c
#define TL866II_VID 0xa466
#define TL866II_PID 0x0a53
#define T76_VID 0xa466
#define T76_PID 0x1a86
#define PIPE_TRANSFER_TIMEOUT 0x03
#define X86_PUSH 0x68
#define X86_RET 0xc3
#define X86_JMP 0xeb

// Typedefs
typedef struct {
  libusb_device_handle *handle;
  UCHAR PipeID;
  PUCHAR Buffer;
  ULONG BufferLength;
  PUINT LengthTransferred;
  LPOVERLAPPED Overlapped;
} Args;

typedef struct {
  struct libusb_transfer *transfer;
  int timeout;
} Endpoint;

typedef BOOL(WINAPI *pMessageBoxA)(HWND, LPCSTR, LPCSTR, UINT);
typedef HWND(WINAPI *pGetForegroundWindow)();
typedef LRESULT(WINAPI *pSendMessageA)(HWND, UINT, WPARAM, LPARAM);
typedef BOOL(WINAPI *pRedrawWindow)(HWND, const RECT *, HRGN, UINT);

// Notification interfaces
const GUID MINIPRO_GUID =
  {0x85980D83, 0x32B9, 0x4BA1, {0x8F, 0xDF, 0x12, 0xA7, 0x11, 0xB9, 0x9C, 0xA2}};
const GUID XGPRO_GUID1 =
  {0xE7E8BA13, 0x2A81, 0x446E, {0xA1, 0x1E, 0x72, 0x39, 0x8F, 0xBD, 0xA8, 0x2F}};
const GUID XGPRO_GUID2 =
  {0x015DE341, 0x91CC, 0x8286, {0x39, 0x64, 0x1A, 0x00, 0x6B, 0xC1, 0xF0, 0x0F}};

// Global variables
int debug = 0;
BOOL cancel;

libusb_device **devs = NULL;
libusb_device_handle *device_handle[4];
Endpoint endpoints[2][7];
unsigned short device_vid;
unsigned short device_pid;
GUID m_guid;

HANDLE *usb_handle;
HANDLE *winusb_handle;
int *devices_count;

HANDLE hotplug_thread;
HWND hWnd;

pMessageBoxA message_box;
pGetForegroundWindow get_foreground_window;
pSendMessageA send_message;
pRedrawWindow redraw_window;

pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;
void device_changed(unsigned int);

// These are functions signature extracted from Xgpro.exe and should be
// compatible from V7.0 and above.
const unsigned char xgpro_open_devices_pattern1[] = {
    0x53, 0x57, 0x6A, 0x00, 0x68, 0x80, 0x00, 0x00, 0x40, 0x6A, 0x03,
    0x6A, 0x00, 0x6A, 0x03, 0x68, 0x00, 0x00, 0x00, 0xC0, 0x68};

const unsigned char xgpro_open_devices_pattern2[] = {
    0x6A, 0x00, 0x68, 0x80, 0x00, 0x00, 0x40, 0x6A, 0x03, 0x6A, 0x00,
    0x6A, 0x03, 0x68, 0x00, 0x00, 0x00, 0xC0, 0x51};

// These are functions signature extracted from MiniPro.exe and should be
// compatible from V6.0 and above.
const unsigned char minipro_open_devices_pattern[] = {
    0x6A, 0x00, 0x68, 0x80, 0x00, 0x00, 0x00, 0x6A, 0x03, 0x6A, 0x00,
    0x6A, 0x03};
const unsigned char usb_write_pattern[] = {
    0x8B, 0x94, 0x24, 0x0C, 0x10, 0x00, 0x00, 0x8D, 0x44, 0x24, 0x00,
    0x6A, 0x00, 0x50, 0x8B, 0x84};
const unsigned char usb_write2_pattern[] = {
    0x8B, 0x94, 0x24, 0x10, 0x10, 0x00, 0x00, 0x8D, 0x44, 0x24, 0x00,
    0x6A, 0x00, 0x50, 0x8B, 0x84};
const unsigned char usb_read_pattern[] = {
    0x64, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x8B, 0x4C, 0x24, 0x08, 0x8B,
    0x54, 0x24, 0x04, 0x6A, 0xFF};
const unsigned char usb_read2_pattern[] = {
    0x8B, 0x4C, 0x24, 0x0C, 0x8B, 0x54, 0x24, 0x08, 0x8D, 0x44, 0x24,
    0x0C, 0x6A, 0x00, 0x50, 0x51};
const unsigned char brickbug_pattern[] = {
    0x83, 0xC4, 0x18, 0x3D, 0x13, 0xF0, 0xC2, 0xC8, 0x75};

// Print given array in hex
void print_hex(const unsigned char *buffer, unsigned int size) {
  unsigned int i;
  for (i = 0; i < size; i++) {
    printf("%02X ", buffer[i]);
    if ((i + 1) % 16 == 0 || i + 1 == size) {
      unsigned int start = i / 16 * 16;
      if ((i + 1) % 16 != 0) {
        printf("%*s", (16 - (i + 1) % 16) * 3, "");
      }
      printf("  ");
      for (unsigned int j = start; j <= i; j++) {
        printf("%c", (buffer[j] < 32 || buffer[j] > 126) ? '.' : buffer[j]);
      }
      printf("\n");
    }
  }
  printf("\n");
}

// USB open/close function replacement
void close_devices() {
  if (devs != NULL) {
    printf("Close devices.\n");
    // Xgpro T76 doesn't support multiple devices yet.
    for (int i = 0; i < (device_pid == T76_PID ? 1 : 4); i++) {
      if (device_handle[i] != NULL) {
        libusb_release_interface(device_handle[i], 0);
        libusb_close(device_handle[i]);
        device_handle[i] = NULL;
      }
    }
    // close session
    libusb_free_device_list(devs, 1);
    libusb_exit(NULL);
    devs = NULL;
  }
}

int open_devices() {
  close_devices();
  device_handle[0] = NULL;
  device_handle[1] = NULL;
  device_handle[2] = NULL;
  device_handle[3] = NULL;
  devs = NULL;

  // Initialize all transfers pointers and timeouts
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 7; j++) {
      endpoints[i][j].transfer = NULL;
      endpoints[i][j].timeout = 5000;
    }
  }

  // initialize a new session
  libusb_init(NULL);
// set verbosity level
#if LIBUSB_API_VERSION >= 0x01000106
  libusb_set_option(NULL, LIBUSB_OPTION_LOG_LEVEL, 3);
#else
  libusb_set_debug(NULL, 3);
#endif

  usb_handle[0] = INVALID_HANDLE_VALUE;

  // Xgpro T76 doesn't support multiple devices yet.
  if (device_pid != T76_PID) {
    usb_handle[1] = INVALID_HANDLE_VALUE;
    usb_handle[2] = INVALID_HANDLE_VALUE;
    usb_handle[3] = INVALID_HANDLE_VALUE;
  }

  if (device_vid == TL866II_VID) {
    *devices_count = 0;
    winusb_handle[0] = INVALID_HANDLE_VALUE;
    if (device_pid != T76_PID) {
      winusb_handle[1] = INVALID_HANDLE_VALUE;
      winusb_handle[2] = INVALID_HANDLE_VALUE;
      winusb_handle[3] = INVALID_HANDLE_VALUE;
    }
  }

  printf("Open devices.\n");
  int devices_found = 0, ret;
  struct libusb_device_descriptor desc;
  int count = libusb_get_device_list(NULL, &devs);

  if (count < 0) {
    return 0;
  }

  char name[128];
  for (int i = 0; i < count; i++) {
    ret = libusb_get_device_descriptor(devs[i], &desc);
    if (ret != LIBUSB_SUCCESS) {
      return 0;
    }

    if (device_pid == desc.idProduct && device_vid == desc.idVendor) {
      if (libusb_open(devs[i], &device_handle[devices_found]) ==
              LIBUSB_SUCCESS &&
          libusb_claim_interface(device_handle[devices_found], 0) ==
              LIBUSB_SUCCESS) {
        usb_handle[devices_found] = (HANDLE)devices_found;
        if (device_vid == TL866II_VID) {
          winusb_handle[devices_found] = (HANDLE)devices_found;
          *devices_count = devices_found + 1;
        }
        libusb_get_string_descriptor_ascii(device_handle[devices_found], 2,
                                           (unsigned char *)name, sizeof(name));

        // Get device name string and remove trailing spaces
        if (strstr(name, "Xingong")) {
          strcpy(name, "XGecu TL866II+");
        } else if (strstr(name, "MiniPro")) {
          strcpy(name, "Minipro TL866A/CS");
        }
        char *end = name + strlen(name) - 1;
        while (end > name && isspace((unsigned char)*end)) end--;
        end[1] = '\0';

        // Get device speed string
        char *speed = "";
        switch (libusb_get_device_speed(devs[i])) {
          case LIBUSB_SPEED_LOW:
            speed = "Low speed (1.5MBit/s)";
            break;
          case LIBUSB_SPEED_FULL:
            speed = "Full speed (12MBit/s)";
            break;
          case LIBUSB_SPEED_HIGH:
            speed = "High speed (480MBit/s)";
            break;
          case LIBUSB_SPEED_SUPER:
            speed = "Super speed (5000MBit/s)";
          default:
            break;
        }
        devices_found++;
        printf("Found USB device %u: VID_%04X, PID_%04X; %s; %s\n",
               devices_found, desc.idVendor, desc.idProduct, name, speed);

        // Xgpro T76 doesn't support multiple devices yet.
        if (devices_found == ((device_pid == T76_PID) ? 1 : 4)) return 0;
      }
    }
  }
  return 0;
}

// Helper function to retrieve a transfer structure from a PipeID
struct libusb_transfer *get_transfer(UCHAR PipeID) {
  return endpoints[(PipeID > 80 ? 1 : 0)][(PipeID & 0x7f) - 1].transfer;
}

// Helper functions to set/get timeout from PipeID
int get_timeout(UCHAR PipeID) {
  return endpoints[(PipeID > 80 ? 1 : 0)][(PipeID & 0x7f) - 1].timeout;
}

void set_timeout(UCHAR PipeID, int ep_timeout) {
  endpoints[(PipeID > 80 ? 1 : 0)][(PipeID & 0x7f) - 1].timeout = ep_timeout;
}

// libusb transfer callback
void transfer_cb(struct libusb_transfer *transfer) {
  // We only set the completion flag here.
  *(int *)transfer->user_data = 1;
}

/*** Xgpro replacement functions. ***/

// USB transfer for WinUsb_ReadPipe/WinUsb_WritePipe.
// This function will run in a separate thread if overlapped
// transfer is specified.
void usb_transfer(Args *args) {

  int ret, completed = 0;

  // Allocate transfer
  struct libusb_transfer *tr = get_transfer(args->PipeID);
  tr = libusb_alloc_transfer(0);
  if (!tr) {
    printf("Out of memory!\n");
    free(args);
    return;
  }

  // Initialize transfer structure for bulk transfer
  libusb_fill_bulk_transfer(tr, args->handle, args->PipeID, args->Buffer,
                            args->BufferLength,
                            (libusb_transfer_cb_fn)transfer_cb, &completed,
                            get_timeout(args->PipeID));

  // Submit the transfer
  ret = libusb_submit_transfer(tr);
  if (ret < 0) {
    printf("\nIO error: %s\n", libusb_error_name(ret));
    free(args);
    libusb_free_transfer(tr);
    return;
  }

  // Wait for transfer to complete
  while (!completed) {
    ret = libusb_handle_events_completed(NULL, &completed);
    if (ret < 0) {
      if (ret == LIBUSB_ERROR_INTERRUPTED)
        continue;
      libusb_cancel_transfer(tr);
      continue;
    }
  }

  // Check if transfer was okay
  if (tr->status != 0) {
    printf("\nIO Error: %s\n", libusb_error_name(tr->status));
    libusb_free_transfer(tr);
    tr = NULL;
    free(args);
    return;
  }

  // Get the actual transfer length
  *args->LengthTransferred = tr->actual_length;

  // Free the allocated transfer structure
  libusb_free_transfer(tr);
  tr = NULL;

  // If debug mode is active print some debug info
  if (debug) {
    pthread_mutex_lock(&print_lock);
    printf("%s %s %u bytes on endpoint 0x%02X\n",
           (args->PipeID & 0x80) ? "Read" : "Write",
           args->Overlapped ? "Async" : "Normal", *args->LengthTransferred,
           args->PipeID);
    if (debug == 1) {
      print_hex(args->Buffer, *args->LengthTransferred);
    }
    pthread_mutex_unlock(&print_lock);
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
 * EP3_OUT=0x03, EP1_IN=0x83; All used                                 *
 ***********************************************************************
 * T48; wMaxPacketSize=512 bytes, 4 endpoints; USB 2.0, 480MBit/s      *
 * EP1_OUT=0x01, EP1_IN=0x81, EP2_OUT=0x02, EP2_IN=0x82; All used      *
 ***********************************************************************
 * T56 wMaxPacketSize = 512 bytes, 2 endpoints; USB 2.0, 480MBit/s     *
 * EP1_OUT=0x01, EP1_IN=0x81; All used                                 *
 ***********************************************************************
 * T76 wMaxPacketSize = 1024 bytes, 14 endpoints; USB 3.0, 5000MBit/s  *
 * EP1_OUT=0x01, EP1_IN=0x81, EP2_OUT=0x02, EP2_IN=0x82,               *
 * EP3_OUT=0x03, EP1_IN=0x83, EP3_OUT=0x04, EP1_IN=0x84,               *
 * EP3_OUT=0x05, EP1_IN=0x85, EP3_OUT=0x06, EP1_IN=0x86                *
 * EP3_OUT=0x07, EP1_IN=0x87;                                          *
 * Only EP1_OUT, EP1_IN, EP2_IN, EP5_OUT are used in current firmware  *
 ***********************************************************************/

// WinUsb_ReadPipe/winUsb_WritePipe LibUsb implementation.
BOOL WINAPI WinUsb_Transfer(HANDLE InterfaceHandle, UCHAR PipeID, PUCHAR Buffer,
                            ULONG BufferLength, PUINT LengthTransferred,
                            LPOVERLAPPED Overlapped) {
  // Check for usb handles
  if (InterfaceHandle == INVALID_HANDLE_VALUE) return FALSE;
  libusb_device_handle *handle = device_handle[(int)InterfaceHandle];
  if (handle == NULL) return FALSE;

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
    libusb_device *device = libusb_get_device(handle);
    if (device == NULL) {
      return FALSE;
    }
    // Round BufferLength to the next multiple of endpoint wMaxPacketSize
    int wMaxPacketSize = libusb_get_max_packet_size(device, PipeID) - 1;
    BufferLength = (BufferLength + wMaxPacketSize) & ~wMaxPacketSize;
  }

  // Prepare args
  Args *args = malloc(sizeof(Args));
  if (!args) {
    printf("Out of memory!\n");
    return FALSE;
  }

  *args = (Args){.handle = handle,
                 .PipeID = PipeID,
                 .Buffer = Buffer,
                 .BufferLength = BufferLength,
                 .LengthTransferred = LengthTransferred,
                 .Overlapped = Overlapped};

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

// WinUsb_SetPipePolicy LibUsb implementation.
// Only setting pipe timeout is supported
BOOL WINAPI WinUsb_SetPipePolicy(HANDLE InterfaceHandle, UCHAR PipeID,
                                 ULONG PolicyType, ULONG ValueLength,
                                 PVOID Value) {
  if (PolicyType == 0x03) {
    set_timeout(PipeID, *(int *)Value);
  }
  return TRUE;
}

// WinUsb_AbortPipe LibUsb implementation
BOOL WINAPI WinUsb_AbortPipe(HANDLE InterfaceHandle, UCHAR PipeID) {
  struct libusb_transfer *tr = get_transfer(PipeID);
  if (tr) {
    libusb_cancel_transfer(tr);
  }
  return TRUE;
}

// WinUsb unused but stubbbed functions.
BOOL WINAPI WinUsb_FlushPipe(HANDLE InterfaceHandle, UCHAR PipeID) {
  return TRUE;
}

BOOL WINAPI WinUsb_Initialize(HANDLE DeviceHandle, PVOID *InterfaceHandle) {
  return TRUE;
}

BOOL WINAPI WinUsb_Free(HANDLE InterfaceHandle) { return TRUE; }

/*** Minipro replacement functions ***/

// USB read implementation. Use the WinUsb_Transfer above
int uread(HANDLE hDevice, unsigned char *data, unsigned int size) {
  unsigned int transferred = 0;
  set_timeout(LIBUSB_ENDPOINT_IN | 1, 20000);
  BOOL ret = WinUsb_Transfer(hDevice, LIBUSB_ENDPOINT_IN | 1, data, size,
                             &transferred, NULL);
  return (ret ? transferred : -1);
}

// USB write implementation. Use the WinUsb_Transfer above
BOOL uwrite(HANDLE hDevice, unsigned char *data, size_t size) {
  unsigned int transferred = 0;
  set_timeout(LIBUSB_ENDPOINT_OUT | 1, 20000);
  return WinUsb_Transfer(hDevice, LIBUSB_ENDPOINT_OUT | 1, data, size,
                         &transferred, NULL);
}

// USB write to device zero
BOOL usb_write(unsigned char *lpInBuffer, unsigned int nInBufferSize) {
  return uwrite(0, lpInBuffer, nInBufferSize);
}

// USB read from device zero
int usb_read(unsigned char *lpOutBuffer, unsigned int nBytesToRead,
             unsigned int nOutBufferSize) {
  int ret = uread(0, lpOutBuffer, nBytesToRead);
  if (ret == -1)
    message_box(get_foreground_window(), "Read error!", "TL866",
                MB_ICONWARNING);
  return ret;
}

// USB write to specified device
BOOL usb_write2(HANDLE hDevice, unsigned char *lpInBuffer,
                unsigned int nInBufferSize) {
  return uwrite(hDevice, lpInBuffer, nInBufferSize);
}

// USB read from specified device
int usb_read2(HANDLE hDevice, unsigned char *lpOutBuffer,
              unsigned int nBytesToRead, unsigned int nOutBufferSize) {
  return uread(hDevice, lpOutBuffer, nBytesToRead);
}

// If make hotplug=udev is invoked then libudev is used for monitoring,
// otherwise libusb hotplug events monitoring is used.
#ifdef UDEV

/*** Udev functions ***/

// Return the device count using Udev library API
int get_device_count() {
  struct udev *udev = udev_new();
  if (!udev) {
    return -1;
  }

  struct udev_enumerate *enumerate;
  struct udev_list_entry *devices, *dev_list_entry;
  struct udev_device *dev;

  enumerate = udev_enumerate_new(udev);
  udev_enumerate_add_match_subsystem(enumerate, "usb");
  udev_enumerate_scan_devices(enumerate);
  devices = udev_enumerate_get_list_entry(enumerate);
  int count = 0;
  udev_list_entry_foreach(dev_list_entry, devices) {
    dev = udev_device_new_from_syspath(
        udev, udev_list_entry_get_name(dev_list_entry));
    if (!dev)
      return -1;

    const char *vid = udev_device_get_sysattr_value(dev, "idVendor");
    const char *pid = udev_device_get_sysattr_value(dev, "idProduct");
    if (vid && pid && strtoul(vid, NULL, 16) == device_vid &&
        strtoul(pid, NULL, 16) == device_pid)
      count++;
    udev_device_unref(dev);
  }
  udev_enumerate_unref(enumerate);
  udev_unref(udev);
  return count;
}

// Udev hotplug USB monitoring thread
void notifier_function() {
  struct udev *udev;
  struct udev_monitor *mon;
  struct udev_device *dev;

  udev = udev_new();
  if (!udev) {
    printf("Can't create udev\n");
    return;
  }

  // Get a new udev monitor from the netlink
  mon = udev_monitor_new_from_netlink(udev, "udev");
  if (!mon) {
    printf("NetLink not available!\n");
    return;
  }

  // Get the device count
  int count = get_device_count();
  if (count == -1) {
    printf("udev error.\n");
    return;
  }

  printf("Using Udev hotplug events.\n\n");
  udev_monitor_filter_add_match_subsystem_devtype(mon, "usb", NULL);
  udev_monitor_enable_receiving(mon);
  int udev_mon_fd = udev_monitor_get_fd(mon);

  // Enter the monitoring loop
  cancel = FALSE;
  while (!cancel) {
    fd_set fds;
    struct timeval tv;
    int ret;

    FD_ZERO(&fds);
    FD_SET(udev_mon_fd, &fds);
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    ret = select(udev_mon_fd + 1, &fds, NULL, NULL, &tv);
    if (ret > 0 && FD_ISSET(udev_mon_fd, &fds)) {
      dev = udev_monitor_receive_device(mon);
      if (dev && !strcasecmp(udev_device_get_devtype(dev), "usb_device")) {
        int count_new;
        if (!strcasecmp(udev_device_get_action(dev), "add")) {
          count_new = get_device_count();
          if (count != count_new) {
            count = count_new;
            // printf("device added.\n");
            device_changed(DBT_DEVICEARRIVAL);
          }
        } else if (!strcasecmp(udev_device_get_action(dev), "remove")) {
          count_new = get_device_count();
          if (count != count_new) {
            count = count_new;
            // printf("device removed.\n");
            device_changed(DBT_DEVICEREMOVECOMPLETE);
          }
        }
        udev_device_unref(dev);
      }
    }
    usleep(50000);
  }
  udev_monitor_unref(mon);
}

// Use libusb hotplug events.
#else

// LibUsb hotplug callback function
int hotplug_cb(struct libusb_context *ctx, struct libusb_device *dev,
               libusb_hotplug_event event, void *user_data) {

  // Notify the event loop thread to rescan the devices.
  // If we call the device_changed function from here the code will crash.
  // See https://libusb.sourceforge.io/api-1.0/libusb_hotplug.html
  *(int *)user_data = event;
  return 0;
}

// LibUsb hotplug USB monitoring thread
void notifier_function() {
  printf("Using LibUsb hotplug events.\n\n");
  int changed = 0;
  libusb_hotplug_callback_handle callback_handle;

  // Register the hotplug callback for the desired USB_VID/PID
  libusb_init(NULL);
  int rc = libusb_hotplug_register_callback(
      NULL,
      LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, 0,
      device_vid, device_pid, LIBUSB_HOTPLUG_MATCH_ANY,
      (libusb_hotplug_callback_fn)hotplug_cb, &changed, &callback_handle);

  // Check if the registration was okay
  if (LIBUSB_SUCCESS != rc) {
    printf("LibUsb hotplug callback error.\n");
    return;
  }

  // Enter the monitoring thread and handle hotplug events.
  while (!cancel) {
    libusb_handle_events_completed(NULL, NULL);
    usleep(50000);
    if (changed) {
      changed = 0;
      device_changed(changed == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED
                         ? DBT_DEVICEARRIVAL
                         : DBT_DEVICEREMOVECOMPLETE);
    }
  }

  // Deregister hotplug callback and exit
  libusb_hotplug_deregister_callback(NULL, callback_handle);
  libusb_exit(NULL);
}
#endif

// Notifier function. This will force the software to rescan devices.
void device_changed(unsigned int event) {

  // Initialize the device broadcast interface
  DEV_BROADCAST_DEVICEINTERFACE_W DevBi;
  DevBi.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE_W);
  DevBi.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
  DevBi.dbcc_classguid = m_guid;

  // Close all devices
  close_devices();
  usleep(100000);

  // Broadcast a device change message
  send_message(hWnd, WM_DEVICECHANGE, event, (LPARAM)&DevBi);
  usleep(100000);

  // Force software to refresh the GUI
  redraw_window(hWnd, NULL, NULL, RDW_INVALIDATE);
}

// RegisterDeviceNotifications WINAPI replacement
HANDLE WINAPI RegisterDeviceNotifications(HANDLE hRecipient,
                                          LPVOID NotificationFilter,
                                          DWORD Flags) {
  printf("RegisterDeviceNotifications hWnd = %p\n", hRecipient);
  hWnd = hRecipient;
  hotplug_thread =
      CreateThread(NULL, 0, (void *)notifier_function, NULL, 0, NULL);
  if (!hotplug_thread) printf("Failed to create the USB monitoring thread!\n");
  return 0;
}

/*** Patcher functions ***/

// Dll redirect patch function
BOOL patch_function(char *library, char *func_name, void *custom_func) {
  DWORD dwOldProtection;
  DWORD func_addr = 0;

  void *BaseAddress = GetModuleHandleA(NULL);
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
    printf("Library '%s' was not found in the import directory.\n", library);
    return FALSE;
  }

  // If the desired library was found we can get the function address
  DWORD_PTR ProcAddress =
      (DWORD_PTR)GetProcAddress(GetModuleHandleA(library), func_name);

  // Check if the desired function address was found
  if (!ProcAddress) {
    printf("Function '%s' was not found in '%s' library.\n", func_name,
           library);
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
      VirtualProtect(info.BaseAddress, info.RegionSize, PAGE_READWRITE,
                     &dwOldProtection);
      func_addr = thunk->u1.Function;
      thunk->u1.Function = (DWORD_PTR)custom_func;
      VirtualProtect(info.BaseAddress, info.RegionSize, info.Protect,
                     &dwOldProtection);
    }
    thunk++;
  }

  // check if the patch was ok.
  if (!func_addr) {
    printf("Function '%s' was not found in the IAT thunk table.\n", func_name);
    return FALSE;
  }
  return TRUE;
}

// Inline helper patch function. Warning, this is x86 ASM code
static inline void patch(void *src, void *dest) {
  // push xxxx, ret; an absolute Jump replacement.
  *(BYTE *)src = X86_PUSH;
  *((DWORD *)((BYTE *)src + 1)) = (DWORD)dest;
  *((BYTE *)src + 5) = X86_RET;
}

// Xgpro patcher function. Called from DllMain. Returns TRUE if patch was ok and
// continue with program loading or FALSE to exit with error.
BOOL patch_xgpro() {
  // Get the BaseAddress, NT Header and Image Import Descriptor
  void *BaseAddress = GetModuleHandleA(NULL);
  PIMAGE_NT_HEADERS NtHeader =
      (PIMAGE_NT_HEADERS)((PBYTE)BaseAddress +
                          ((PIMAGE_DOS_HEADER)BaseAddress)->e_lfanew);

  // Search for version and set the Xgpro GUID and VID/PID
  unsigned char *version;
  if ((version = memmem(BaseAddress, NtHeader->OptionalHeader.SizeOfImage,
                        "Xgpro v", 7))) {

    // TL866II+, T48, T56 VID/PID and interface GUID
    device_vid = TL866II_VID;
    device_pid = TL866II_PID;
    memcpy(&m_guid, &XGPRO_GUID1, sizeof(GUID));
  } else if ((version =
                  memmem(BaseAddress, NtHeader->OptionalHeader.SizeOfImage,
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

  if (!patch_function("winusb.dll", "WinUsb_Initialize", &WinUsb_Initialize))
    return FALSE;

  if (!patch_function("winusb.dll", "WinUsb_Free", &WinUsb_Free)) return FALSE;

  // Searching for functions signature in code section.
  void *p_opendevices = NULL;
  void *p_closedevices = NULL;
  void *p_winusbhandle = NULL;
  void *p_usbhandle = NULL;
  void *p_devicescount = NULL;

  // Search for open_device function pattern 1 (xgpro < V12.7x)
  void *p_od1 =
      memmem(BaseAddress + NtHeader->OptionalHeader.BaseOfCode,
             NtHeader->OptionalHeader.SizeOfCode, &xgpro_open_devices_pattern1,
             sizeof(xgpro_open_devices_pattern1));

  // Search for open_device function pattern 2 (xgpro > V12.7x)
  void *p_od2 =
      memmem(BaseAddress + NtHeader->OptionalHeader.BaseOfCode,
             NtHeader->OptionalHeader.SizeOfCode, &xgpro_open_devices_pattern2,
             sizeof(xgpro_open_devices_pattern2));

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
    p_opendevices = p_od1 - 0x1D;
    p_closedevices = (void *)(*(int *)((unsigned char *)p_opendevices + 5)) +
                     (DWORD)((unsigned char *)p_opendevices + 9);
    p_winusbhandle = (void *)(*(int *)((unsigned char *)p_closedevices + 0x12));
    p_usbhandle = (void *)(*(int *)((unsigned char *)p_closedevices + 0x2));
    p_devicescount = (void *)(*(int *)((unsigned char *)p_opendevices + 0xAF));
  } else if (p_od2) {
    p_opendevices = p_od2 - 0x41;
    p_closedevices = (void *)(*(int *)((unsigned char *)p_opendevices + 8)) +
                     (DWORD)((unsigned char *)p_opendevices + 12);
    p_winusbhandle = (void *)(*(int *)((unsigned char *)p_closedevices + 0x12));
    p_usbhandle = (void *)(*(int *)((unsigned char *)p_closedevices + 0x2));
    p_devicescount = (void *)(*(int *)((unsigned char *)p_opendevices + 0x28));
    if (!patch_function("winusb.dll", "WinUsb_AbortPipe", &WinUsb_AbortPipe))
      return FALSE;

    if (!patch_function("winusb.dll", "WinUsb_FlushPipe", &WinUsb_FlushPipe))
      return FALSE;
  } else {
    printf("Function signatures not found! Unsupported Xgpro version.\n");
    return FALSE;
  }

  // Print debug info.
  printf("Base Address = %p\n", BaseAddress);
  printf("Code section = %p, 0x%lx\n",
         BaseAddress + NtHeader->OptionalHeader.BaseOfCode,
         (DWORD)NtHeader->OptionalHeader.SizeOfCode);
  printf("Open Devices found at %p\n", p_opendevices);
  printf("Close Devices found at %p\n", p_closedevices);
  printf("Usb Handle found at %p\n", p_usbhandle);
  printf("WinUsb Handle found at %p\n", p_winusbhandle);
  printf("Devices count found at %p\n", p_devicescount);

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
  VirtualProtect(BaseAddress + NtHeader->OptionalHeader.BaseOfCode,
                 NtHeader->OptionalHeader.SizeOfCode, PAGE_READWRITE,
                 &dwOldProtection);

  // patch open_devices function to point to our implementation
  patch(p_opendevices, &open_devices);

  // patch close_devices function to point to our implementation
  patch(p_closedevices, &close_devices);

  // restore the old READ_ONLY protection
  VirtualProtect(BaseAddress + NtHeader->OptionalHeader.BaseOfCode,
                 NtHeader->OptionalHeader.SizeOfCode, dwOldProtection,
                 &dwOldProtection);
  return TRUE;
}

// Minipro patcher function. Called from DllMain. Returns TRUE if patch was ok
// and continue with program loading or FALSE to exit with error.
BOOL patch_minipro() {
  // Get the BaseAddress, NT Header and Image Import Descriptor
  void *BaseAddress = GetModuleHandleA(NULL);
  PIMAGE_NT_HEADERS NtHeader =
      (PIMAGE_NT_HEADERS)((PBYTE)BaseAddress +
                          ((PIMAGE_DOS_HEADER)BaseAddress)->e_lfanew);

  unsigned char *version =
      memmem(BaseAddress, NtHeader->OptionalHeader.SizeOfImage, "MiniPro v", 9);
  if (!version) return FALSE;
  printf("Found %s\n", version);

  // Patch the Linux incompatible functions functions
  if (!patch_function("user32.dll", "RegisterDeviceNotificationA",
                      &RegisterDeviceNotifications))
    return FALSE;

  // Searching for functions signature in code section.
  void *p_opendevices =
      memmem(BaseAddress + NtHeader->OptionalHeader.BaseOfCode,
             NtHeader->OptionalHeader.SizeOfCode, &minipro_open_devices_pattern,
             sizeof(minipro_open_devices_pattern)) -
      0x28;
  void *p_closedevices =
      (void *)(*(int *)((unsigned char *)p_opendevices + 4)) +
      (DWORD)((unsigned char *)p_opendevices + 8);
  void *p_usbwrite = memmem(BaseAddress + NtHeader->OptionalHeader.BaseOfCode,
                            NtHeader->OptionalHeader.SizeOfCode,
                            &usb_write_pattern, sizeof(usb_write_pattern)) -
                     0x0A;
  void *p_usbwrite2 = memmem(BaseAddress + NtHeader->OptionalHeader.BaseOfCode,
                             NtHeader->OptionalHeader.SizeOfCode,
                             &usb_write2_pattern, sizeof(usb_write2_pattern)) -
                      0x0A;
  void *p_usbread = memmem(BaseAddress + NtHeader->OptionalHeader.BaseOfCode,
                           NtHeader->OptionalHeader.SizeOfCode,
                           &usb_read_pattern, sizeof(usb_read_pattern));
  void *p_usbread2 = memmem(BaseAddress + NtHeader->OptionalHeader.BaseOfCode,
                            NtHeader->OptionalHeader.SizeOfCode,
                            &usb_read2_pattern, sizeof(usb_read2_pattern));
  void *p_usbhandle = (void *)(*(int *)((unsigned char *)p_closedevices + 1));

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
      memmem(BaseAddress + NtHeader->OptionalHeader.BaseOfCode,
             NtHeader->OptionalHeader.SizeOfCode, &brickbug_pattern,
             sizeof(brickbug_pattern));
  // Print some debug info.
  printf("Base Address = %p\n", BaseAddress);
  printf("Code section = %p, 0x%lx\n",
         BaseAddress + NtHeader->OptionalHeader.BaseOfCode,
         (DWORD)NtHeader->OptionalHeader.SizeOfCode);
  printf("Open Devices found at %p\n", p_opendevices);
  printf("Close Devices found at %p\n", p_closedevices);
  printf("Usb Write found at %p\n", p_usbwrite);
  printf("Usb Read found at %p\n", p_usbread);
  printf("Usb Write2 found at %p\n", p_usbwrite2);
  printf("Usb Read2 found at %p\n", p_usbread2);
  printf("Usb Handle found at %p\n", p_usbhandle);
  if (p_brickbug) printf("Patched brick bug at %p\n", p_brickbug + 0x08);

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
  if (p_brickbug) *(p_brickbug + 0x08) = X86_JMP;

  // Restore the old READ_ONLY protection
  VirtualProtect(BaseAddress + NtHeader->OptionalHeader.BaseOfCode,
                 NtHeader->OptionalHeader.SizeOfCode, dwOldProtection,
                 &dwOldProtection);

  // Set the Minipro GUID
  memcpy(&m_guid, &MINIPRO_GUID, sizeof(GUID));

  // Set the vid/pid
  device_vid = TL866A_VID;
  device_pid = TL866A_PID;
  return TRUE;
}

/*** DllMain ***/
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
  // Get the module name (should be setupapi.dll)
  WCHAR buffer[MAX_PATH];
  GetModuleFileNameW(hinstDLL, buffer, sizeof(buffer));
  char *module_path = wine_get_unix_file_name(buffer);
  if(!module_path){
    module_path = "setupapi.dll";
  }
  switch (fdwReason) {

    // Dll loaded and atached to process
    case DLL_PROCESS_ATTACH:
      DisableThreadLibraryCalls(hinstDLL);
      printf("%s loaded.\n", module_path);

      // Set the debug mode if TL_DEBUG environment variable is set
      char *debug_var = NULL;
      debug_var = getenv("TL_DEBUG");
      if (debug_var && !strncmp(debug_var, "1", 1))
        debug = 1;
      else if (debug_var && !strncmp(debug_var, "2", 1))
        debug = 2;
      else
        debug = 0;

      // Set some used function pointers from the user32.dll library
      HMODULE hmodule = LoadLibraryA("user32.dll");
      message_box = (pMessageBoxA)GetProcAddress(hmodule, "MessageBoxA");
      get_foreground_window =
          (pGetForegroundWindow)GetProcAddress(hmodule, "GetForegroundWindow");
      send_message = (pSendMessageA)GetProcAddress(hmodule, "SendMessageA");
      redraw_window = (pRedrawWindow)GetProcAddress(hmodule, "RedrawWindow");

      // Try to patch the software
      if (patch_xgpro() || patch_minipro()) return TRUE;
      printf("%s unloaded.\n", wine_get_unix_file_name(buffer));
      return FALSE;
      break;

    // We are detached from a process, terminate the USB hotplug monitoring
    // thread, close all opened devices and exit.
    case DLL_PROCESS_DETACH:
      cancel = TRUE;
      WaitForSingleObject(hotplug_thread, 5000);
      close_devices();
      printf("%s unloaded.\n", module_path);
      break;
  }
  return TRUE;
}

/*************************************************************************/
/********************** Exported functions section ***********************/
/*************************************************************************/

// SetupApi redirected functions needed for the new wine >4.11 winex11.drv
// calls. These functions must be specified in setupapi.spec file.
typedef BOOL(WINAPI *pSetupDiGetDeviceInterfaceDetailW)(HANDLE, HANDLE, HANDLE,
                                                        DWORD, PDWORD, LPVOID);

typedef BOOL(WINAPI *pSetupDiGetDeviceRegistryPropertyW)(HANDLE, LPVOID, DWORD,
                                                         PDWORD, PBYTE, DWORD,
                                                         PDWORD);
typedef BOOL(WINAPI *pSetupDiCallClassInstaller)(LPVOID, HANDLE, LPVOID);
typedef HANDLE(WINAPI *pSetupDiGetClassDevsA)(const GUID *, PCSTR, HWND, DWORD);
typedef HANDLE(WINAPI *pSetupDiGetClassDevsW)(const GUID *, PCWSTR, HWND,
                                              DWORD);
typedef BOOL(WINAPI *pSetupDiEnumDeviceInfo)(HANDLE, DWORD, LPVOID);
typedef BOOL(WINAPI *pSetupDiEnumDeviceInterfaces)(HANDLE, LPVOID, const GUID *,
                                                   DWORD, HANDLE);
typedef BOOL(WINAPI *pSetupDiGetDevicePropertyW)(HANDLE, LPVOID, const LPVOID *,
                                                 LPVOID *, PBYTE, DWORD, PDWORD,
                                                 DWORD);
typedef BOOL (*pSetupDiDestroyDeviceInfoList)(HANDLE DeviceInfoSet);
typedef HANDLE(WINAPI *pSetupDiCreateDeviceInfoList)(const GUID *ClassGuid,
                                                     HWND hwndParent);
typedef BOOL(WINAPI *pSetupDiSetDevicePropertyW)(HANDLE, LPVOID, const LPVOID *,
                                                 LPVOID, const PBYTE, DWORD,
                                                 DWORD);
typedef BOOL(WINAPI *pSetupDiCreateDeviceInfoW)(HANDLE, PCWSTR, const GUID *,
                                                PCWSTR, HWND, DWORD, LPVOID);
typedef BOOL(WINAPI *pSetupDiOpenDeviceInfoW)(HANDLE, PCWSTR, HWND, DWORD,
                                              LPVOID);
typedef BOOL(WINAPI *pSetupDiRegisterDeviceInfo)(HANDLE, LPVOID, DWORD, LPVOID,
                                                 PVOID, LPVOID);
typedef BOOL(WINAPI *pSetupDiSetDeviceRegistryPropertyW)(HANDLE, LPVOID, DWORD,
                                                         const BYTE *, DWORD);
typedef HKEY(WINAPI *pSetupDiCreateDevRegKeyW)(HANDLE, LPVOID, DWORD, DWORD,
                                               DWORD, HANDLE, PCWSTR);
typedef BOOL(WINAPI *pSetupDiRemoveDevice)(HANDLE, LPVOID);
typedef void (WINAPI *pInstallHinfSectionA)(HWND, HINSTANCE, PCSTR, INT);
typedef void (WINAPI *pInstallHinfSectionW)(HWND, HINSTANCE, PCWSTR, INT);

// Helper function to obtain the function address to original setupapi.dll
FARPROC get_proc_address(LPCSTR lpProcName) {
  char sysdir[MAX_PATH];
  GetSystemDirectoryA(sysdir, MAX_PATH);
  strcat(sysdir, "\\setupapi.dll");
  HMODULE hmodule = LoadLibraryA(sysdir);
  FARPROC address = GetProcAddress(hmodule, lpProcName);
  //printf("%s : %p\n", lpProcName, address);
  return address;
}

WINAPI BOOL SetupDiGetDeviceInterfaceDetailW(
    HANDLE DeviceInfoSet, HANDLE DeviceInterfaceData,
    HANDLE DeviceInterfaceDetailData, DWORD DeviceInterfaceDetailDataSize,
    PDWORD RequiredSize, LPVOID DeviceInfoData) {
  pSetupDiGetDeviceInterfaceDetailW pfunc =
      (pSetupDiGetDeviceInterfaceDetailW)get_proc_address(
          "SetupDiGetDeviceInterfaceDetailW");
  return pfunc(DeviceInfoSet, DeviceInterfaceData, DeviceInterfaceDetailData,
               DeviceInterfaceDetailDataSize, RequiredSize, DeviceInfoData);
}

WINAPI BOOL SetupDiGetDeviceRegistryPropertyW(
    HANDLE DeviceInfoSet, LPVOID DeviceInfoData, DWORD Property,
    PDWORD PropertyRegDataType, PBYTE PropertyBuffer, DWORD PropertyBufferSize,
    PDWORD RequiredSize) {
  pSetupDiGetDeviceRegistryPropertyW pfunc =
      (pSetupDiGetDeviceRegistryPropertyW)get_proc_address(
          "SetupDiGetDeviceRegistryPropertyW");
  return pfunc(DeviceInfoSet, DeviceInfoData, Property, PropertyRegDataType,
               PropertyBuffer, PropertyBufferSize, RequiredSize);
}

WINAPI BOOL SetupDiCallClassInstaller(LPVOID InstallFunction,
                                      HANDLE DeviceInfoSet,
                                      LPVOID DeviceInfoDat) {
  pSetupDiCallClassInstaller pfunc =
      (pSetupDiCallClassInstaller)get_proc_address("SetupDiCallClassInstaller");
  return pfunc(InstallFunction, DeviceInfoSet, DeviceInfoDat);
}

WINAPI HANDLE SetupDiGetClassDevsA(const GUID *ClassGuid, PCSTR Enumerator,
                                   HWND hwndParent, DWORD Flags) {
  pSetupDiGetClassDevsA pfunc =
      (pSetupDiGetClassDevsA)get_proc_address("SetupDiGetClassDevsA");
  return pfunc(ClassGuid, Enumerator, hwndParent, Flags);
}

WINAPI HANDLE SetupDiGetClassDevsW(const GUID *ClassGuid, PCWSTR Enumerator,
                                   HWND hwndParent, DWORD Flags) {
  pSetupDiGetClassDevsW pfunc =
      (pSetupDiGetClassDevsW)get_proc_address("SetupDiGetClassDevsW");
  return pfunc(ClassGuid, Enumerator, hwndParent, Flags);
}

WINAPI BOOL SetupDiEnumDeviceInfo(HANDLE DeviceInfoSet, DWORD MemberIndex,
                                  LPVOID DeviceInfoData) {
  pSetupDiEnumDeviceInfo pfunc =
      (pSetupDiEnumDeviceInfo)get_proc_address("SetupDiEnumDeviceInfo");
  return pfunc(DeviceInfoSet, MemberIndex, DeviceInfoData);
}

WINAPI BOOL SetupDiEnumDeviceInterfaces(HANDLE DeviceInfoSet,
                                        LPVOID DeviceInfoData,
                                        const GUID *InterfaceClassGuid,
                                        DWORD MemberIndex,
                                        HANDLE DeviceInterfaceData) {
  pSetupDiEnumDeviceInterfaces pfunc =
      (pSetupDiEnumDeviceInterfaces)get_proc_address(
          "SetupDiEnumDeviceInterfaces");
  return pfunc(DeviceInfoSet, DeviceInfoData, InterfaceClassGuid, MemberIndex,
               DeviceInterfaceData);
}

WINAPI BOOL SetupDiGetDevicePropertyW(
    HANDLE DeviceInfoSet, LPVOID DeviceInfoData, const LPVOID *PropertyKey,
    LPVOID *PropertyType, PBYTE PropertyBuffer, DWORD PropertyBufferSize,
    PDWORD RequiredSize, DWORD Flags)

{
  pSetupDiGetDevicePropertyW pfunc =
      (pSetupDiGetDevicePropertyW)get_proc_address("SetupDiGetDevicePropertyW");
  return pfunc(DeviceInfoSet, DeviceInfoData, PropertyKey, PropertyType,
               PropertyBuffer, PropertyBufferSize, RequiredSize, Flags);
}

WINAPI BOOL SetupDiDestroyDeviceInfoList(HANDLE DeviceInfoSet) {
  pSetupDiDestroyDeviceInfoList pfunc =
      (pSetupDiDestroyDeviceInfoList)get_proc_address(
          "SetupDiDestroyDeviceInfoList");
  return pfunc(DeviceInfoSet);
}

WINAPI HANDLE SetupDiCreateDeviceInfoList(const GUID *ClassGuid,
                                          HWND hwndParent) {
  pSetupDiCreateDeviceInfoList pfunc =
      (pSetupDiCreateDeviceInfoList)get_proc_address(
          "SetupDiCreateDeviceInfoList");
  return pfunc(ClassGuid, hwndParent);
}

WINAPI BOOL SetupDiSetDevicePropertyW(HANDLE DeviceInfoSet,
                                      LPVOID DeviceInfoData,
                                      const LPVOID *PropertyKey,
                                      LPVOID PropertyType,
                                      const PBYTE PropertyBuffer,
                                      DWORD PropertyBufferSize, DWORD Flags) {
  pSetupDiSetDevicePropertyW pfunc =
      (pSetupDiSetDevicePropertyW)get_proc_address("SetupDiSetDevicePropertyW");
  return pfunc(DeviceInfoSet, DeviceInfoData, PropertyKey, PropertyType,
               PropertyBuffer, PropertyBufferSize, Flags);
}

WINAPI BOOL SetupDiCreateDeviceInfoW(HANDLE DeviceInfoSet, PCWSTR DeviceName,
                                     const GUID *ClassGuid,
                                     PCWSTR DeviceDescription, HWND hwndParent,
                                     DWORD CreationFlags,
                                     LPVOID DeviceInfoData) {
  pSetupDiCreateDeviceInfoW pfunc =
      (pSetupDiCreateDeviceInfoW)get_proc_address("SetupDiCreateDeviceInfoW");
  return pfunc(DeviceInfoSet, DeviceName, ClassGuid, DeviceDescription,
               hwndParent, CreationFlags, DeviceInfoData);
}

WINAPI BOOL SetupDiOpenDeviceInfoW(HANDLE DeviceInfoSet,
                                   PCWSTR DeviceInstanceId, HWND hwndParent,
                                   DWORD OpenFlags, LPVOID DeviceInfoData) {
  pSetupDiOpenDeviceInfoW pfunc =
      (pSetupDiOpenDeviceInfoW)get_proc_address("SetupDiOpenDeviceInfoW");
  return pfunc(DeviceInfoSet, DeviceInstanceId, hwndParent, OpenFlags,
               DeviceInfoData);
}

WINAPI BOOL SetupDiRegisterDeviceInfo(HANDLE DeviceInfoSet,
                                      LPVOID DeviceInfoData, DWORD Flags,
                                      LPVOID CompareProc, PVOID CompareContext,
                                      LPVOID DupDeviceInfoData) {
  pSetupDiRegisterDeviceInfo pfunc =
      (pSetupDiRegisterDeviceInfo)get_proc_address("SetupDiRegisterDeviceInfo");
  return pfunc(DeviceInfoSet, DeviceInfoData, Flags, CompareProc,
               CompareContext, DupDeviceInfoData);
}

WINAPI BOOL SetupDiSetDeviceRegistryPropertyW(HANDLE DeviceInfoSet,
                                              LPVOID DeviceInfoData,
                                              DWORD Property,
                                              const BYTE *PropertyBuffer,
                                              DWORD PropertyBufferSize)

{
  pSetupDiSetDeviceRegistryPropertyW pfunc =
      (pSetupDiSetDeviceRegistryPropertyW)get_proc_address(
          "SetupDiSetDeviceRegistryPropertyW");
  return pfunc(DeviceInfoSet, DeviceInfoData, Property, PropertyBuffer,
               PropertyBufferSize);
}

WINAPI HKEY SetupDiCreateDevRegKeyW(HANDLE DeviceInfoSet, LPVOID DeviceInfoData,
                                    DWORD Scope, DWORD HwProfile, DWORD KeyType,
                                    HANDLE InfHandle, PCWSTR InfSectionName) {
  pSetupDiCreateDevRegKeyW pfunc =
      (pSetupDiCreateDevRegKeyW)get_proc_address("SetupDiCreateDevRegKeyW");
  return pfunc(DeviceInfoSet, DeviceInfoData, Scope, HwProfile, KeyType,
               InfHandle, InfSectionName);
}

WINAPI BOOL SetupDiRemoveDevice(HANDLE DeviceInfoSet, LPVOID DeviceInfoData) {
  pSetupDiRemoveDevice pfunc =
      (pSetupDiRemoveDevice)get_proc_address("SetupDiRemoveDevice");
  return pfunc(DeviceInfoSet, DeviceInfoData);
}

WINAPI void InstallHinfSectionA(HWND Window, HINSTANCE ModuleHandle,
                                PCSTR CommandLine, INT ShowCommand) {
  pInstallHinfSectionA pfunc =
      (pInstallHinfSectionA)get_proc_address("InstallHinfSectionA");
  return pfunc(Window, ModuleHandle, CommandLine, ShowCommand);
}

WINAPI void InstallHinfSectionW(HWND Window, HINSTANCE ModuleHandle,
                                PCWSTR CommandLine, INT ShowCommand) {
  pInstallHinfSectionW pfunc =
      (pInstallHinfSectionW)get_proc_address("InstallHinfSectionW");
  return pfunc(Window, ModuleHandle, CommandLine, ShowCommand);
}
