/*
* setupapi.c
* Winelib wrapper for Minipro TL866A/CS, TL866II+, XgPro T48, T56 and T76
* programmers.
* This library will redirect all USB related functions from Minipro or Xgpro software
* to the Linux USB subsystem using the standard LibUsb library. 
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

#define TL866A_VID 0x04d8
#define TL866A_PID 0xe11c
#define TL866II_VID 0xa466
#define TL866II_PID 0x0a53
#define T76_VID 0xa466
#define T76_PID 0x1a86

#define PIPE_TRANSFER_TIMEOUT 0x03

typedef struct {
  HANDLE InterfaceHandle;
  UCHAR PipeID;
  PUCHAR Buffer;
  ULONG BufferLength;
  PUINT LengthTransferred;
  LPOVERLAPPED Overlapped;
} Args;

// Global variables
libusb_device_handle *device_handle[4];
libusb_device **devs;
struct libusb_transfer *transfer[2][7];
int timeout[2][7];

int debug = 0;
HANDLE h_thread;
HWND hWnd;
BOOL cancel;
HANDLE *usb_handle;
HANDLE *winusb_handle;
int *devices_count;
GUID m_guid;

unsigned short device_vid;
unsigned short device_pid;

typedef BOOL(__stdcall *pMessageBoxA)(HWND, LPCSTR, LPCSTR, UINT);
typedef HWND(__stdcall *pGetForegroundWindow)();
typedef LRESULT(__stdcall *pSendMessageA)(HWND, UINT, WPARAM, LPARAM);
typedef BOOL(__stdcall *pRedrawWindow)(HWND, const RECT *, HRGN, UINT);

pMessageBoxA message_box;
pGetForegroundWindow get_foreground_window;
pSendMessageA send_message;
pRedrawWindow redraw_window;
void device_changed();

pthread_mutex_t mylock = PTHREAD_MUTEX_INITIALIZER;

// These are functions signature extracted from Xgpro.exe and should be
// compatible from V7.0 and above.
const unsigned char xgpro_open_devices_pattern1[] = {
    0x53, 0x57, 0x6A, 0x00, 0x68, 0x80, 0x00, 0x00, 0x40, 0x6A, 0x03,
    0x6A, 0x00, 0x6A, 0x03, 0x68, 0x00, 0x00, 0x00, 0xC0, 0x68};

const unsigned char xgpro_open_devices_pattern2[] = {
    0x6A, 0x00, 0x68, 0x80, 0x00, 0x00, 0x40, 0x6A, 0x03, 0x6A,
    0x00, 0x6A, 0x03, 0x68, 0x00, 0x00, 0x00, 0xC0, 0x51};

// These are functions signature extracted from MiniPro.exe and should be
// compatible from V6.0 and above.
const unsigned char minipro_open_devices_pattern[] = {
    0x6A, 0x00, 0x68, 0x80, 0x00, 0x00, 0x00,
    0x6A, 0x03, 0x6A, 0x00, 0x6A, 0x03};
const unsigned char usb_write_pattern[] = {0x8B, 0x94, 0x24, 0x0C, 0x10, 0x00,
                                           0x00, 0x8D, 0x44, 0x24, 0x00, 0x6A,
                                           0x00, 0x50, 0x8B, 0x84};
const unsigned char usb_write2_pattern[] = {0x8B, 0x94, 0x24, 0x10, 0x10, 0x00,
                                            0x00, 0x8D, 0x44, 0x24, 0x00, 0x6A,
                                            0x00, 0x50, 0x8B, 0x84};
const unsigned char usb_read_pattern[] = {0x64, 0xA1, 0x00, 0x00, 0x00, 0x00,
                                          0x8B, 0x4C, 0x24, 0x08, 0x8B, 0x54,
                                          0x24, 0x04, 0x6A, 0xFF};
const unsigned char usb_read2_pattern[] = {0x8B, 0x4C, 0x24, 0x0C, 0x8B, 0x54,
                                           0x24, 0x08, 0x8D, 0x44, 0x24, 0x0C,
                                           0x6A, 0x00, 0x50, 0x51};
const unsigned char brickbug_pattern[] = {0x83, 0xC4, 0x18, 0x3D, 0x13,
                                          0xF0, 0xC2, 0xC8, 0x75};

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
  printf("Close devices.\n");
  if (devs != NULL) {
    // Xgpro T76 doesn't support multiple devices yet.
    for (int i = 0; i < (device_pid == T76_PID ? 1 : 4); i++) {
      if (device_handle[i] != NULL) {
        libusb_release_interface(device_handle[i], 0);
        libusb_close(device_handle[i]);
        device_handle[i] = NULL;
      }
    }
    libusb_free_device_list(devs, 1);
    libusb_exit(NULL); // close session
    devs = NULL;
  }
}

int open_devices() {
  char *debug_var = NULL;
  debug_var = getenv("TL_DEBUG");
  if (debug_var && 0 == strncmp(debug_var, "1", 1))
    debug = 1;
  else
    debug = 0;

  printf("Open devices.\n");
  close_devices();
  
  
  device_handle[0] = NULL;
  device_handle[1] = NULL;
  device_handle[2] = NULL;
  device_handle[3] = NULL;
  devs = NULL;
  
  // Initialize all transfers pointers and timeouts
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 7; j++) {
      transfer[i][j] = NULL;
      timeout[i][j] = 5000;
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
                                           name, sizeof(name));
        if (strstr(name, "Xingong")) {
          strcpy(name, "XGecu TL866II+");
        } else if (strstr(name, "MiniPro")) {
          strcpy(name, "Minipro TL866A/CS");
        }
        devices_found++;
        printf("Found USB device %u: VID_%04X, PID_%04X; %s\n", devices_found,
               desc.idVendor, desc.idProduct, name);

        // Xgpro T76 doesn't support multiple devices yet.
        if (devices_found == ((device_pid == T76_PID) ? 1 : 4))
          return 0;
      }
    }
  }
  return 0;
}

// Helper function to retrieve a transfer structure from a PipeID
struct libusb_transfer *get_transfer(UCHAR PipeID) {
  return transfer[(PipeID > 80 ? 1 : 0)][(PipeID & 0x7f) - 1];
}

// Helper functions to set/get timeout from PipeID
int get_timeout(UCHAR PipeID) {
  return timeout[(PipeID > 80 ? 1 : 0)][(PipeID & 0x7f) - 1];
}

void set_timeout(UCHAR PipeID, int ep_timeout) {
  timeout[(PipeID > 80 ? 1 : 0)][(PipeID & 0x7f) - 1] = ep_timeout;
}

// libusb transfer callback
void transfer_cb(struct libusb_transfer *transfer){
	*(int*)transfer->user_data = 1;
}


/*** Xgpro replacement functions. ***/

// USB transfer for WinUsb_ReadPipe/WinUsb_WritePipe.
// This function will run in a separate thread if overlapped
// transfer is specified.
void usb_transfer(Args *args) {

  int ret, completed = 0;
  struct libusb_transfer *tr = get_transfer(args->PipeID);
  tr = libusb_alloc_transfer(0);
  if (!tr) {
    printf("Out of memory!\n");
    free(args);
    return;
  }

  libusb_fill_bulk_transfer(tr, device_handle[(int)args->InterfaceHandle],
                            args->PipeID, args->Buffer, args->BufferLength,
                            (libusb_transfer_cb_fn)transfer_cb, &completed,
                            get_timeout(args->PipeID));

  ret = libusb_submit_transfer(tr);
  if (ret < 0) {
    printf("\nIO error: submit_transfer: %s\n", libusb_error_name(ret));
    free(args);
    libusb_free_transfer(tr);
    return;
  }

  while (!completed) {
    ret = libusb_handle_events_completed(NULL, &completed);
    if (ret < 0) {
      if (ret == LIBUSB_ERROR_INTERRUPTED)
        continue;
      libusb_cancel_transfer(tr);
      continue;
    }
  }

  if (tr->status != 0) {
    printf("\nIO Error: transfer failed: %s\n", libusb_error_name(tr->status));
    libusb_free_transfer(tr);
    free(args);
    return;
  }

  *args->LengthTransferred = tr->actual_length;
  libusb_free_transfer(tr);

  if (debug) {
    pthread_mutex_lock(&mylock);
    printf("%s %s %lu bytes on endpoint 0x%02X\n",
           (args->PipeID & 0x80) ? "Read" : "Write",
           args->Overlapped ? "Async" : "Normal", args->BufferLength,
           args->PipeID);
    print_hex(args->Buffer, *args->LengthTransferred);
    pthread_mutex_unlock(&mylock);
  }

  // If Overlapped (async) transfer was completed
  // signal the event to release the waiting object.
  if (args->Overlapped) {
    SetEvent(args->Overlapped->hEvent);
  }

  // Free the malloced args.
  free(args);
}

// WinUsb_ReadPipe/winUsb_WritePipe LibUsb implementation.
BOOL __stdcall WinUsb_Transfer(HANDLE InterfaceHandle, UCHAR PipeID,
                               PUCHAR Buffer, ULONG BufferLength,
                               PUINT LengthTransferred,
                               LPOVERLAPPED Overlapped) {

  // Check for usb handles
  if (InterfaceHandle == INVALID_HANDLE_VALUE)
    return FALSE;
  if (device_handle[(int)InterfaceHandle] == NULL)
    return FALSE;

  // Workaround for T76 endpoint 0x83 not used issue
  if (device_pid == T76_PID && PipeID == 0x83)
    return TRUE;

  // Workaround for Xgpro read BufferLength issue
  if ((PipeID > 0x80) && BufferLength < 64) {
    BufferLength = 64;
  }

  // Prepare args
  Args *args = malloc(sizeof(*args));
  args->InterfaceHandle = InterfaceHandle;
  args->PipeID = PipeID;
  args->Buffer = Buffer;
  args->BufferLength = BufferLength;
  args->LengthTransferred = LengthTransferred;
  args->Overlapped = Overlapped;

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
BOOL __stdcall WinUsb_SetPipePolicy(HANDLE InterfaceHandle, UCHAR PipeID,
                                    ULONG PolicyType, ULONG ValueLength,
                                    PVOID Value) {
  if (PolicyType == 0x03) {
    set_timeout(PipeID, *(int *)Value);
  }
  return TRUE;
}

// WinUsb_AbortPipe LibUsb implementation
BOOL __stdcall WinUsb_AbortPipe(HANDLE InterfaceHandle, UCHAR PipeID) {
  struct libusb_transfer *tr = get_transfer(PipeID);
  if (tr) {
    libusb_cancel_transfer(tr);
  }
  return TRUE;
}

// WinUsb unused but stubbbed functions.
BOOL __stdcall WinUsb_FlushPipe(HANDLE InterfaceHandle, UCHAR PipeID) {
  return TRUE;
}

BOOL __stdcall WinUsb_Initialize(HANDLE DeviceHandle, PVOID *InterfaceHandle) {
  return TRUE;
}

BOOL __stdcall WinUsb_Free(HANDLE InterfaceHandle) { return TRUE; }



/*** Minipro replacement functions ***/

// USB read implementation
int uread(HANDLE hDevice, unsigned char *data, unsigned int size) {
  unsigned int transferred = 0;
  set_timeout(LIBUSB_ENDPOINT_IN | 1, 20000);
  BOOL ret = WinUsb_Transfer(hDevice, LIBUSB_ENDPOINT_IN | 1, data, size,
                             &transferred, NULL);
  return (ret ? transferred : -1);
}

// USB write implementation
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

// Return the device count
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

  mon = udev_monitor_new_from_netlink(udev, "udev");
  if (!mon) {
    printf("NetLink not available!\n");
    return;
  }
  int count = get_device_count();
  if (count == -1) {
    printf("udev error.\n");
    return;
  }

  printf("Using Udev hotplug events.\n\n");
  udev_monitor_filter_add_match_subsystem_devtype(mon, "usb", NULL);
  udev_monitor_enable_receiving(mon);
  int fd = udev_monitor_get_fd(mon);

  cancel = FALSE;
  while (!cancel) {
    fd_set fds;
    struct timeval tv;
    int ret;

    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    ret = select(fd + 1, &fds, NULL, NULL, &tv);
    if (ret > 0 && FD_ISSET(fd, &fds)) {
      dev = udev_monitor_receive_device(mon);
      if (dev && !strcasecmp(udev_device_get_devtype(dev), "usb_device")) {
        int count_new;
        if (!strcasecmp(udev_device_get_action(dev), "add")) {
          count_new = get_device_count();
          if (count != count_new) {
            count = count_new;
            // printf("device added.\n");
            device_changed();
          }
        } else if (!strcasecmp(udev_device_get_action(dev), "remove")) {
          count_new = get_device_count();
          if (count != count_new) {
            count = count_new;
            // printf("device removed.\n");
            device_changed();
          }
        }
        udev_device_unref(dev);
      }
    }
    usleep(10000);
  }
  udev_monitor_unref(mon);
}

// Use libusb hotplug events.
#else
// LibUsb hotplug callback function
int hotplug_cb(struct libusb_context *ctx, struct libusb_device *dev,
               libusb_hotplug_event event, void *user_data) {

  // Notify the event loop thread
  // If we call the device_changed(); from here the code will crash.
  *(int *)user_data = event;
  return 0;
}

// LibUsb hotplug USB monitoring thread
void notifier_function() {
  printf("Using LibUsb hotplug events.\n\n");
  libusb_hotplug_callback_handle callback_handle;
  int changed = 0;
  libusb_init(NULL);
  int rc = libusb_hotplug_register_callback(
      NULL,
      LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, 0,
      device_vid, device_pid, LIBUSB_HOTPLUG_MATCH_ANY,
      (libusb_hotplug_callback_fn)hotplug_cb, &changed, &callback_handle);
  if (LIBUSB_SUCCESS != rc) {
    printf("LibUsb hotplug callback error.\n");
    return;
  }
  
  // Wait for cancel and handle hotplug events.
  while (!cancel) {
    libusb_handle_events_completed(NULL, NULL);
    usleep(10000);
    if (changed) {
      changed = 0;
      device_changed();
    }
  }

  libusb_hotplug_deregister_callback(NULL, callback_handle);
  libusb_exit(NULL);
}
#endif

// Notifier function. This will force the software to rescan devices.
void device_changed() {

  DEV_BROADCAST_DEVICEINTERFACE_W DevBi;
  DevBi.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE_W);
  DevBi.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
  DevBi.dbcc_classguid = m_guid;

  close_devices();
  usleep(100000);
  send_message(hWnd, WM_DEVICECHANGE, DBT_DEVICEARRIVAL, (LPARAM)&DevBi);
  usleep(100000);
  redraw_window(hWnd, NULL, NULL, RDW_INVALIDATE);
}

// RegisterDeviceNotifications WINAPI replacement
HANDLE __stdcall RegisterDeviceNotifications(HANDLE hRecipient,
                                             LPVOID NotificationFilter,
                                             DWORD Flags) {
  printf("RegisterDeviceNotifications hWnd = 0x%X4\n",
         (unsigned int)hRecipient);
  hWnd = hRecipient;
  h_thread = CreateThread(NULL, 0, (void *)notifier_function, NULL, 0, NULL);
  if (!h_thread)
    printf("Thread notifier failed.\n");
  return 0;
}

/*** Patcher functions ***/

// Dll redirect patch function
BOOL patch_function(char *library, char *func, void *funcaddress) {
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

  // Search for library in the IAT
  while (ImpDesc->Characteristics && ImpDesc->Name) {
    if (strcasecmp(BaseAddress + ImpDesc->Name, library) == 0) {
      break; // Found it!
    }
    ImpDesc++;
  }

  // check if the library was found in the IAT
  if (!ImpDesc->Characteristics) {
    printf("%s was not found in the IAT.\n", library);
    return FALSE; // nope, exit with error.
  }

  // Get the address of function in library
  DWORD_PTR ProcAddress =
      (DWORD_PTR)GetProcAddress(GetModuleHandleA(library), func);

  // Find the address in the thunk table
  PIMAGE_THUNK_DATA thunk =
      (PIMAGE_THUNK_DATA)(BaseAddress + ImpDesc->FirstThunk);
  while (thunk->u1.Function) {
    if ((DWORD_PTR)thunk->u1.Function == ProcAddress) {
      // if found, patch it to point to our custom function
      MEMORY_BASIC_INFORMATION info;
      VirtualQuery(&thunk->u1.Function, &info,
                   sizeof(MEMORY_BASIC_INFORMATION));
      VirtualProtect(info.BaseAddress, info.RegionSize, PAGE_READWRITE,
                     &dwOldProtection);
      func_addr = thunk->u1.Function;
      thunk->u1.Function = (DWORD_PTR)funcaddress;
      VirtualProtect(info.BaseAddress, info.RegionSize, info.Protect,
                     &dwOldProtection);
    }

    thunk++;
  }

  // check if the patch was ok.
  if (!func_addr) {
    printf("%s was not found in %s.\n", func, library);
    return FALSE; // nope, exit with error.
  }

  return TRUE;
}

// Inline helper patch function
static inline void patch(void *src, void *dest) {
  // push xxxx, ret; an absolute Jump replacement.
  *(BYTE *)src = 0x68;
  *((DWORD *)((BYTE *)src + 1)) = (DWORD)dest;
  *((BYTE *)src + 5) = 0xc3;
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
  GUID guid;
  unsigned char *version;
  if ((version = memmem(BaseAddress, NtHeader->OptionalHeader.SizeOfImage,
                        "Xgpro v", 7))) {
    device_vid = TL866II_VID;
    device_pid = TL866II_PID;
    guid = (GUID){0xE7E8BA13,
                  0x2A81,
                  0x446E,
                  {0xA1, 0x1E, 0x72, 0x39, 0x8F, 0xBD, 0xA8, 0x2F}};

  } else if ((version =
                  memmem(BaseAddress, NtHeader->OptionalHeader.SizeOfImage,
                         "Xgpro T76 v", 11))) {
    device_vid = T76_VID;
    device_pid = T76_PID;
    guid = (GUID){0x015DE341,
                  0x91CC,
                  0x8286,
                  {0x39, 0x64, 0x1A, 0x00, 0x6B, 0xC1, 0xF0, 0x0F}};

  } else {
    return FALSE;
  }

  memcpy(&m_guid, &guid, sizeof(GUID));
  printf("Found %s\n", version);

  // Set some function pointers
  HMODULE hmodule = LoadLibraryA("user32.dll");
  send_message = (pSendMessageA)GetProcAddress(hmodule, "SendMessageA");
  redraw_window = (pRedrawWindow)GetProcAddress(hmodule, "RedrawWindow");

  // Patch the Linux incompatible functions functions
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

  if (!patch_function("winusb.dll", "WinUsb_Free", &WinUsb_Free))
    return FALSE;

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
    return FALSE; // nope, exit with error.
  }

  // Print debug info.
  printf("Base Address = 0x%08lX\n", (DWORD)BaseAddress);
  printf("Code section = 0x%08lX,0x%08lX\n",
         (DWORD)BaseAddress + NtHeader->OptionalHeader.BaseOfCode,
         (DWORD)NtHeader->OptionalHeader.SizeOfCode);
  printf("Open Devices found at 0x%08lX\n", (DWORD)p_opendevices);
  printf("Close Devices found at 0x%08lX\n", (DWORD)p_closedevices);
  printf("Usb Handle found at 0x%08lX\n", (DWORD)p_usbhandle);
  printf("WinUsb Handle found at 0x%08lX\n", (DWORD)p_winusbhandle);
  printf("Devices count found at 0x%08lX\n", (DWORD)p_devicescount);

  // Patch all low level functions in Xgpro.exe to point to our custom
  // functions.
  DWORD dwOldProtection;

  // Initialize the usb handle address.
  usb_handle = p_usbhandle;
  winusb_handle = p_winusbhandle;
  devices_count = p_devicescount;
  VirtualProtect(BaseAddress + NtHeader->OptionalHeader.BaseOfCode,
                 NtHeader->OptionalHeader.SizeOfCode, PAGE_READWRITE,
                 &dwOldProtection); // unprotect the code memory section

  // patch Open_Devices function
  patch(p_opendevices, &open_devices);

  // patch close_devices function
  patch(p_closedevices, &close_devices);

  VirtualProtect(BaseAddress + NtHeader->OptionalHeader.BaseOfCode,
                 NtHeader->OptionalHeader.SizeOfCode, dwOldProtection,
                 &dwOldProtection); // restore the old protection
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
  if (!version)
    return FALSE;
  printf("Found %s\n", version);

  // Set some function pointers
  HMODULE hmodule = LoadLibraryA("user32.dll");
  message_box = (pMessageBoxA)GetProcAddress(hmodule, "MessageBoxA");
  get_foreground_window =
      (pGetForegroundWindow)GetProcAddress(hmodule, "GetForegroundWindow");
  send_message = (pSendMessageA)GetProcAddress(hmodule, "SendMessageA");
  redraw_window = (pRedrawWindow)GetProcAddress(hmodule, "RedrawWindow");

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
    return FALSE; // nope, exit with error.
  }

  // search for brick bug
  unsigned char *p_brickbug =
      memmem(BaseAddress + NtHeader->OptionalHeader.BaseOfCode,
             NtHeader->OptionalHeader.SizeOfCode, &brickbug_pattern,
             sizeof(brickbug_pattern));
  // Print debug info.
  printf("Base Address = 0x%08lX\n", (DWORD)BaseAddress);
  printf("Code section = 0x%08lX,0x%08lX\n",
         (DWORD)BaseAddress + NtHeader->OptionalHeader.BaseOfCode,
         (DWORD)NtHeader->OptionalHeader.SizeOfCode);
  printf("Open Devices found at 0x%08lX\n", (DWORD)p_opendevices);
  printf("Close Devices found at 0x%08lX\n", (DWORD)p_closedevices);
  printf("Usb Write found at 0x%08lX\n", (DWORD)p_usbwrite);
  printf("Usb Read found at 0x%08lX\n", (DWORD)p_usbread);
  printf("Usb Write2 found at 0x%08lX\n", (DWORD)p_usbwrite2);
  printf("Usb Read2 found at 0x%08lX\n", (DWORD)p_usbread2);
  printf("Usb Handle found at 0x%08lX\n", (DWORD)p_usbhandle);
  if (p_brickbug)
    printf("Patched brick bug at 0x%08lX\n", (DWORD)p_brickbug + 0x08);

  // Patch all low level functions in MiniPro.exe to point to our custom
  // functions.
  DWORD dwOldProtection;

  // Initialize the usb handle address.
  usb_handle = p_usbhandle;
  VirtualProtect(BaseAddress + NtHeader->OptionalHeader.BaseOfCode,
                 NtHeader->OptionalHeader.SizeOfCode, PAGE_READWRITE,
                 &dwOldProtection); // unprotect the code memory section

  // patch Open_Devices function
  patch(p_opendevices, &open_devices);

  // patch close_devices function
  patch(p_closedevices, &close_devices);

  // patch USB_Write function
  patch(p_usbwrite, &usb_write);

  // patch USB_Read function
  patch(p_usbread, &usb_read);

  // patch USB_Write2 function
  patch(p_usbwrite2, &usb_write2);

  // patch USB_Read2 function
  patch(p_usbread2, &usb_read2);

  // patch the brick bug
  if (p_brickbug)
    *(p_brickbug + 0x08) = 0xEB;

  VirtualProtect(BaseAddress + NtHeader->OptionalHeader.BaseOfCode,
                 NtHeader->OptionalHeader.SizeOfCode, dwOldProtection,
                 &dwOldProtection); // restore the old protection

  // Set the Minipro GUID
  const GUID guid = {0x85980D83,
                     0x32B9,
                     0x4BA1,
                     {0x8F, 0xDF, 0x12, 0xA7, 0x11, 0xB9, 0x9C, 0xA2}};
  memcpy(&m_guid, &guid, sizeof(GUID));

  // set vid/pid
  device_vid = TL866A_VID;
  device_pid = TL866A_PID;
  return TRUE;
}

/// DLLMAIN
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
  char buffer[MAX_PATH];
  GetModuleFileNameA(hinstDLL, buffer, sizeof(buffer));
  switch (fdwReason) {
  case DLL_PROCESS_ATTACH:
    DisableThreadLibraryCalls(hinstDLL);
    printf("%s loaded.\n", buffer);
    if (patch_xgpro() || patch_minipro())
      return TRUE;
    printf("%s unloaded.\n", buffer);
    return FALSE;
    break;
  case DLL_PROCESS_DETACH:
    cancel = TRUE;
    WaitForSingleObject(h_thread, 5000);
    printf("%s unloaded.\n", buffer);
    break;
  }
  return TRUE;
}


/// SetupApi redirected functions needed for the new wine >4.11 winex11.drv
/// calls. These functions must be specified in setupapi.spec file.
typedef BOOL(__stdcall *pSetupDiGetDeviceInterfaceDetailW)(HANDLE, HANDLE,
                                                           HANDLE, DWORD,
                                                           PDWORD, LPVOID);

typedef BOOL(__stdcall *pSetupDiGetDeviceRegistryPropertyW)(HANDLE, LPVOID,
                                                            DWORD, PDWORD,
                                                            PBYTE, DWORD,
                                                            PDWORD);
typedef BOOL(__stdcall *pSetupDiCallClassInstaller)(LPVOID, HANDLE, LPVOID);
typedef HANDLE(__stdcall *pSetupDiGetClassDevsA)(const GUID *, PCSTR, HWND,
                                                 DWORD);
typedef HANDLE(__stdcall *pSetupDiGetClassDevsW)(const GUID *, PCWSTR, HWND,
                                                 DWORD);
typedef BOOL(__stdcall *pSetupDiEnumDeviceInfo)(HANDLE, DWORD, LPVOID);
typedef BOOL(__stdcall *pSetupDiEnumDeviceInterfaces)(HANDLE, LPVOID,
                                                      const GUID *, DWORD,
                                                      HANDLE);
typedef BOOL(__stdcall *pSetupDiGetDevicePropertyW)(HANDLE, LPVOID,
                                                    const LPVOID *, LPVOID *,
                                                    PBYTE, DWORD, PDWORD,
                                                    DWORD);
typedef BOOL (*pSetupDiDestroyDeviceInfoList)(HANDLE DeviceInfoSet);
typedef HANDLE(__stdcall *pSetupDiCreateDeviceInfoList)(const GUID *ClassGuid,
                                                        HWND hwndParent);
typedef BOOL(__stdcall *pSetupDiSetDevicePropertyW)(HANDLE, LPVOID,
                                                    const LPVOID *, LPVOID,
                                                    const PBYTE, DWORD, DWORD);
typedef BOOL(__stdcall *pSetupDiCreateDeviceInfoW)(HANDLE, PCWSTR, const GUID *,
                                                   PCWSTR, HWND, DWORD, LPVOID);
typedef BOOL(__stdcall *pSetupDiOpenDeviceInfoW)(HANDLE, PCWSTR, HWND, DWORD,
                                                 LPVOID);
typedef BOOL(__stdcall *pSetupDiRegisterDeviceInfo)(HANDLE, LPVOID, DWORD,
                                                    LPVOID, PVOID, LPVOID);
typedef BOOL(__stdcall *pSetupDiSetDeviceRegistryPropertyW)(HANDLE, LPVOID,
                                                            DWORD, const BYTE *,
                                                            DWORD);
typedef HKEY(__stdcall *pSetupDiCreateDevRegKeyW)(HANDLE, LPVOID, DWORD, DWORD,
                                                  DWORD, HANDLE, PCWSTR);
typedef BOOL(__stdcall *pSetupDiRemoveDevice)(HANDLE, LPVOID);

FARPROC get_proc_address(LPCSTR lpProcName) {
  char sysdir[MAX_PATH];
  GetSystemDirectoryA(sysdir, MAX_PATH);
  strcat(sysdir, "\\setupapi.dll");
  HMODULE hmodule = LoadLibraryA(sysdir);
  FARPROC address = GetProcAddress(hmodule, lpProcName);
  // printf("%s : 0x%08lX\n", lpProcName, (uint32_t)address);
  return address;
}

__stdcall BOOL SetupDiGetDeviceInterfaceDetailW(
    HANDLE DeviceInfoSet, HANDLE DeviceInterfaceData,
    HANDLE DeviceInterfaceDetailData, DWORD DeviceInterfaceDetailDataSize,
    PDWORD RequiredSize, LPVOID DeviceInfoData) {
  pSetupDiGetDeviceInterfaceDetailW pfunc =
      (pSetupDiGetDeviceInterfaceDetailW)get_proc_address(
          "SetupDiGetDeviceInterfaceDetailW");
  return pfunc(DeviceInfoSet, DeviceInterfaceData, DeviceInterfaceDetailData,
               DeviceInterfaceDetailDataSize, RequiredSize, DeviceInfoData);
}

_stdcall BOOL SetupDiGetDeviceRegistryPropertyW(
    HANDLE DeviceInfoSet, LPVOID DeviceInfoData, DWORD Property,
    PDWORD PropertyRegDataType, PBYTE PropertyBuffer, DWORD PropertyBufferSize,
    PDWORD RequiredSize) {
  pSetupDiGetDeviceRegistryPropertyW pfunc =
      (pSetupDiGetDeviceRegistryPropertyW)get_proc_address(
          "SetupDiGetDeviceRegistryPropertyW");
  return pfunc(DeviceInfoSet, DeviceInfoData, Property, PropertyRegDataType,
               PropertyBuffer, PropertyBufferSize, RequiredSize);
}

__stdcall BOOL SetupDiCallClassInstaller(LPVOID InstallFunction,
                                         HANDLE DeviceInfoSet,
                                         LPVOID DeviceInfoDat) {
  pSetupDiCallClassInstaller pfunc =
      (pSetupDiCallClassInstaller)get_proc_address("SetupDiCallClassInstaller");
  return pfunc(InstallFunction, DeviceInfoSet, DeviceInfoDat);
}

__stdcall HANDLE SetupDiGetClassDevsA(const GUID *ClassGuid, PCSTR Enumerator,
                                      HWND hwndParent, DWORD Flags) {
  pSetupDiGetClassDevsA pfunc =
      (pSetupDiGetClassDevsA)get_proc_address("SetupDiGetClassDevsA");
  return pfunc(ClassGuid, Enumerator, hwndParent, Flags);
}

__stdcall HANDLE SetupDiGetClassDevsW(const GUID *ClassGuid, PCWSTR Enumerator,
                                      HWND hwndParent, DWORD Flags) {
  pSetupDiGetClassDevsW pfunc =
      (pSetupDiGetClassDevsW)get_proc_address("SetupDiGetClassDevsW");
  return pfunc(ClassGuid, Enumerator, hwndParent, Flags);
}

__stdcall BOOL SetupDiEnumDeviceInfo(HANDLE DeviceInfoSet, DWORD MemberIndex,
                                     LPVOID DeviceInfoData) {
  pSetupDiEnumDeviceInfo pfunc =
      (pSetupDiEnumDeviceInfo)get_proc_address("SetupDiEnumDeviceInfo");
  return pfunc(DeviceInfoSet, MemberIndex, DeviceInfoData);
}

_stdcall BOOL SetupDiEnumDeviceInterfaces(HANDLE DeviceInfoSet,
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

__stdcall BOOL
SetupDiGetDevicePropertyW(HANDLE DeviceInfoSet, LPVOID DeviceInfoData,
                          const LPVOID *PropertyKey, LPVOID *PropertyType,
                          PBYTE PropertyBuffer, DWORD PropertyBufferSize,
                          PDWORD RequiredSize, DWORD Flags)

{
  pSetupDiGetDevicePropertyW pfunc =
      (pSetupDiGetDevicePropertyW)get_proc_address("SetupDiGetDevicePropertyW");
  return pfunc(DeviceInfoSet, DeviceInfoData, PropertyKey, PropertyType,
               PropertyBuffer, PropertyBufferSize, RequiredSize, Flags);
}

__stdcall BOOL SetupDiDestroyDeviceInfoList(HANDLE DeviceInfoSet) {
  pSetupDiDestroyDeviceInfoList pfunc =
      (pSetupDiDestroyDeviceInfoList)get_proc_address(
          "SetupDiDestroyDeviceInfoList");
  return pfunc(DeviceInfoSet);
}

__stdcall HANDLE SetupDiCreateDeviceInfoList(const GUID *ClassGuid,
                                             HWND hwndParent) {
  pSetupDiCreateDeviceInfoList pfunc =
      (pSetupDiCreateDeviceInfoList)get_proc_address(
          "SetupDiCreateDeviceInfoList");
  return pfunc(ClassGuid, hwndParent);
}

__stdcall BOOL
SetupDiSetDevicePropertyW(HANDLE DeviceInfoSet, LPVOID DeviceInfoData,
                          const LPVOID *PropertyKey, LPVOID PropertyType,
                          const PBYTE PropertyBuffer, DWORD PropertyBufferSize,
                          DWORD Flags) {
  pSetupDiSetDevicePropertyW pfunc =
      (pSetupDiSetDevicePropertyW)get_proc_address("SetupDiSetDevicePropertyW");
  return pfunc(DeviceInfoSet, DeviceInfoData, PropertyKey, PropertyType,
               PropertyBuffer, PropertyBufferSize, Flags);
}

__stdcall BOOL SetupDiCreateDeviceInfoW(HANDLE DeviceInfoSet, PCWSTR DeviceName,
                                        const GUID *ClassGuid,
                                        PCWSTR DeviceDescription,
                                        HWND hwndParent, DWORD CreationFlags,
                                        LPVOID DeviceInfoData) {
  pSetupDiCreateDeviceInfoW pfunc =
      (pSetupDiCreateDeviceInfoW)get_proc_address("SetupDiCreateDeviceInfoW");
  return pfunc(DeviceInfoSet, DeviceName, ClassGuid, DeviceDescription,
               hwndParent, CreationFlags, DeviceInfoData);
}

__stdcall BOOL SetupDiOpenDeviceInfoW(HANDLE DeviceInfoSet,
                                      PCWSTR DeviceInstanceId, HWND hwndParent,
                                      DWORD OpenFlags, LPVOID DeviceInfoData) {
  pSetupDiOpenDeviceInfoW pfunc =
      (pSetupDiOpenDeviceInfoW)get_proc_address("SetupDiOpenDeviceInfoW");
  return pfunc(DeviceInfoSet, DeviceInstanceId, hwndParent, OpenFlags,
               DeviceInfoData);
}

__stdcall BOOL SetupDiRegisterDeviceInfo(HANDLE DeviceInfoSet,
                                         LPVOID DeviceInfoData, DWORD Flags,
                                         LPVOID CompareProc,
                                         PVOID CompareContext,
                                         LPVOID DupDeviceInfoData) {
  pSetupDiRegisterDeviceInfo pfunc =
      (pSetupDiRegisterDeviceInfo)get_proc_address("SetupDiRegisterDeviceInfo");
  return pfunc(DeviceInfoSet, DeviceInfoData, Flags, CompareProc,
               CompareContext, DupDeviceInfoData);
}

__stdcall BOOL
SetupDiSetDeviceRegistryPropertyW(HANDLE DeviceInfoSet, LPVOID DeviceInfoData,
                                  DWORD Property, const BYTE *PropertyBuffer,
                                  DWORD PropertyBufferSize)

{
  pSetupDiSetDeviceRegistryPropertyW pfunc =
      (pSetupDiSetDeviceRegistryPropertyW)get_proc_address(
          "SetupDiSetDeviceRegistryPropertyW");
  return pfunc(DeviceInfoSet, DeviceInfoData, Property, PropertyBuffer,
               PropertyBufferSize);
}

__stdcall HKEY SetupDiCreateDevRegKeyW(HANDLE DeviceInfoSet,
                                       LPVOID DeviceInfoData, DWORD Scope,
                                       DWORD HwProfile, DWORD KeyType,
                                       HANDLE InfHandle,
                                       PCWSTR InfSectionName) {
  pSetupDiCreateDevRegKeyW pfunc =
      (pSetupDiCreateDevRegKeyW)get_proc_address("SetupDiCreateDevRegKeyW");
  return pfunc(DeviceInfoSet, DeviceInfoData, Scope, HwProfile, KeyType,
               InfHandle, InfSectionName);
}

__stdcall BOOL SetupDiRemoveDevice(HANDLE DeviceInfoSet,
                                   LPVOID DeviceInfoData) {
  pSetupDiRemoveDevice pfunc =
      (pSetupDiRemoveDevice)get_proc_address("SetupDiRemoveDevice");
  return pfunc(DeviceInfoSet, DeviceInfoData);
}
