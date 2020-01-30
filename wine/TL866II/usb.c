#define __WINESRC__
#define __CYGWIN__
#define _GNU_SOURCE

#include <glob.h>
#include <libudev.h>
#include <libusb-1.0/libusb.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <dbt.h>
#include <winbase.h>
#include <windef.h>
#include <winnt.h>
//#include <setupapi.h>

#define TL866_VID 0xA466
#define TL866_PID 0x0A53

typedef struct {
  HANDLE InterfaceHandle;
  UCHAR PipeID;
  PUCHAR Buffer;
  ULONG BufferLength;
  PULONG LengthTransferred;
  LPOVERLAPPED Overlapped;
} Args;

// replacement functions for Xgpro. Function prototypes and calling convention
// must be the same as in Xgpro.exe, otherwise the application will crash.
int open_devices(int *error);
void close_devices();
HANDLE __stdcall RegisterDeviceNotifications(HANDLE hRecipient,
                                             LPVOID NotificationFilter,
                                             DWORD Flags);
BOOL __stdcall WinUsb_SetPipePolicy(HANDLE InterfaceHandle, UCHAR PipeID,
                                    ULONG PolicyType, ULONG ValueLength,
                                    PVOID Value);
BOOL __stdcall WinUsb_Transfer(HANDLE InterfaceHandle, UCHAR PipeID,
                               PUCHAR Buffer, ULONG BufferLength,
                               PULONG LengthTransferred,
                               LPOVERLAPPED Overlapped);

// helper functions
BOOL patch_xgpro();
BOOL patch_function(char *library, char *func, void *funcaddress);
void async_transfer(Args *args);
void notifier_function();
int get_device_count();

// Global variables
libusb_device_handle *device_handle[4];
libusb_device **devs;

HANDLE h_thread;

HWND hWnd;
BOOL cancel;
HANDLE *usb_handle;
HANDLE *winusb_handle;
int *devices_count;

// These are functions signature extracted from Xgpro.exe and should be
// compatible from V7.0 and above.
const unsigned char open_devices_pattern[] = {
    0x53, 0x57, 0x6A, 0x00, 0x68, 0x80, 0x00, 0x00,
    0x40, 0x6A, 0x03, 0x6A, 0x00, 0x6A, 0x03, 0x68};

BOOL patch_function(char *library, char *func, void *funcaddress) {
  DWORD dwOldProtection;
  DWORD func_addr = 0;

  void *BaseAddress = GetModuleHandleA(NULL);
  PIMAGE_NT_HEADERS NtHeader = (PIMAGE_NT_HEADERS)(
      (PBYTE)BaseAddress + ((PIMAGE_DOS_HEADER)BaseAddress)->e_lfanew);
  PIMAGE_IMPORT_DESCRIPTOR ImpDesc = (PIMAGE_IMPORT_DESCRIPTOR)(
      (PBYTE)BaseAddress +
      NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]
          .VirtualAddress);

  // Search for library in the IAT
  while (ImpDesc->Characteristics && ImpDesc->Name) {
    if (strcasecmp(BaseAddress + ImpDesc->Name, library) == 0) {
      break;  // Found it!
    }
    ImpDesc++;
  }

  // check if the library was found in the IAT
  if (!ImpDesc->Characteristics) {
    printf("%s was not found in the IAT.\n", library);
    return FALSE;  // nope, exit with error.
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
    return FALSE;  // nope, exit with error.
  }

  return TRUE;
}

// Patcher function. Called from DllMain. Return TRUE if patch was ok and
// continue with program loading or FALSE to exit with error.
BOOL patch_xgpro() {
  // Load the original setupapi.dll for new wine 4.11+
  char sysdir[MAX_PATH];
  GetSystemDirectoryA(sysdir, MAX_PATH);
  strcat(sysdir, "\\setupapi.dll");
  HMODULE hmodule = LoadLibraryA(sysdir);

  // Get the BaseAddress, NT Header and Image Import Descriptor
  void *BaseAddress = GetModuleHandleA(NULL);
  PIMAGE_NT_HEADERS NtHeader = (PIMAGE_NT_HEADERS)(
      (PBYTE)BaseAddress + ((PIMAGE_DOS_HEADER)BaseAddress)->e_lfanew);

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

  // Searching for functions signature in code section.
  void *p_opendevices =
      memmem(BaseAddress + NtHeader->OptionalHeader.BaseOfCode,
             NtHeader->OptionalHeader.SizeOfCode, &open_devices_pattern,
             sizeof(open_devices_pattern)) -
      0x1D;
  void *p_closedevices =
      (void *)(*(int *)((unsigned char *)p_opendevices + 5)) +
      (DWORD)((unsigned char *)p_opendevices + 9);
  void *p_winusbhandle =
      (void *)(*(int *)((unsigned char *)p_closedevices + 0x12));
  void *p_usbhandle = (void *)(*(int *)((unsigned char *)p_closedevices + 0x2));
  void *p_devicescount =
      (void *)(*(int *)((unsigned char *)p_opendevices + 0xAF));

  // check if all pointers are o.k.
  if (!p_opendevices) {
    printf("Functions signature not found! Unknown Xgpro version.\n");
    return FALSE;  // nope, exit with error.
  }

  // Print debug info.
  unsigned char *version =
      memmem(BaseAddress, NtHeader->OptionalHeader.SizeOfImage, "Xgpro v", 7);
  if (version) printf("Found %s\n", version);
  printf("Base Address = 0x%08X\n", (DWORD)BaseAddress);
  printf("Code section = 0x%08X,0x%08X\n",
         (DWORD)BaseAddress + NtHeader->OptionalHeader.BaseOfCode,
         (DWORD)NtHeader->OptionalHeader.SizeOfCode);
  printf("Open Devices found at 0x%08X\n", (DWORD)p_opendevices);
  printf("Close Devices found at  0x%08X\n", (DWORD)p_closedevices);
  printf("Usb Handle found at  0x%08X\n", (DWORD)p_usbhandle);
  printf("WinUsb Handle found at  0x%08X\n", (DWORD)p_winusbhandle);
  printf("Devices count found at  0x%08X\n", (DWORD)p_devicescount);

  // Patch all low level functions in Xgpro.exe to point to our custom
  // functions.
  BYTE t[] = {0x68, 0, 0,
              0,    0, 0xc3};  // push xxxx, ret; an absolute Jump replacement.
  DWORD *p_func = (DWORD *)&t[1];
  DWORD dwOldProtection;

  // Initialize the usb handle address.
  usb_handle = p_usbhandle;
  winusb_handle = p_winusbhandle;
  devices_count = p_devicescount;
  VirtualProtect(BaseAddress + NtHeader->OptionalHeader.BaseOfCode,
                 NtHeader->OptionalHeader.SizeOfCode, PAGE_READWRITE,
                 &dwOldProtection);  // unprotect the code memory section

  // patch Open_Devices function
  *p_func = (DWORD)&open_devices;
  memcpy(p_opendevices, t, 6);

  // patch close_devices function
  *p_func = (DWORD)&close_devices;
  memcpy(p_closedevices, t, 6);

  VirtualProtect(BaseAddress + NtHeader->OptionalHeader.BaseOfCode,
                 NtHeader->OptionalHeader.SizeOfCode, dwOldProtection,
                 &dwOldProtection);  // restore the old protection
  return TRUE;
}

// Xgpro replacement functions
int open_devices(int *error) {
  printf("Open devices.\n");
  close_devices();
  device_handle[0] = NULL;
  device_handle[1] = NULL;
  device_handle[2] = NULL;
  device_handle[3] = NULL;
  devs = NULL;

  libusb_init(NULL);         // initialize a new session
  libusb_set_debug(NULL, 3);  // set verbosity level

  usb_handle[0] = INVALID_HANDLE_VALUE;
  usb_handle[1] = INVALID_HANDLE_VALUE;
  usb_handle[2] = INVALID_HANDLE_VALUE;
  usb_handle[3] = INVALID_HANDLE_VALUE;

  winusb_handle[0] = INVALID_HANDLE_VALUE;
  winusb_handle[1] = INVALID_HANDLE_VALUE;
  winusb_handle[2] = INVALID_HANDLE_VALUE;
  winusb_handle[3] = INVALID_HANDLE_VALUE;

  *devices_count = 0;

  int devices_found = 0, ret;
  struct libusb_device_descriptor desc;
  int count = libusb_get_device_list(NULL, &devs);

  if (count < 0) {
    return 0;
  }

  for (int i = 0; i < count; i++) {
    ret = libusb_get_device_descriptor(devs[i], &desc);
    if (ret != LIBUSB_SUCCESS) {
      return 0;
    }

    if (TL866_PID == desc.idProduct && TL866_VID == desc.idVendor) {
      if (libusb_open(devs[i], &device_handle[devices_found]) ==
              LIBUSB_SUCCESS &&
          libusb_claim_interface(device_handle[devices_found], 0) ==
              LIBUSB_SUCCESS) {
        usb_handle[devices_found] = (HANDLE)devices_found;
        winusb_handle[devices_found] = (HANDLE)devices_found;
        devices_found++;
        *devices_count = devices_found;
        if (devices_found == 4) return 0;
      }
    }
  }
  return 0;
}

void close_devices() {
  printf("Close devices.\n");
  if (devs != NULL) {
    for (int i = 0; i < 4; i++) {
      if (device_handle[i] != NULL) {
        libusb_release_interface(device_handle[i], 0);
        libusb_close(device_handle[i]);
        device_handle[i] = NULL;
      }
    }
    libusb_free_device_list(devs, 1);
    libusb_exit(NULL);  // close session
    devs = NULL;
    ;
  }
}

HANDLE __stdcall RegisterDeviceNotifications(HANDLE hRecipient,
                                             LPVOID NotificationFilter,
                                             DWORD Flags) {
  printf("RegisterDeviceNotifications hWnd=%X4\n", (unsigned int)hRecipient);
  hWnd = hRecipient;
  h_thread = CreateThread(NULL, 0, (void *)notifier_function, NULL, 0, NULL);
  if (!h_thread) printf("Thread notifier failed.\n");

  return 0;
}

// winusb implemented functions.
BOOL __stdcall WinUsb_SetPipePolicy(HANDLE InterfaceHandle, UCHAR PipeID,
                                    ULONG PolicyType, ULONG ValueLength,
                                    PVOID Value) {
  return TRUE;
}

// Asynchronous transfer for WinUsb_ReadPipe/WinUsb_WritePipe.
void async_transfer(Args *args) {
  libusb_bulk_transfer(device_handle[(int)args->InterfaceHandle], args->PipeID,
                       args->Buffer, args->BufferLength,
                       args->LengthTransferred, 20000);
  SetEvent(args->Overlapped
               ->hEvent);  // signal the event to release the waiting object.
  free(args);              // Free the malloced args.
}

// WinUsb_ReadPipe/winUsb_WritePipe LibUsb implementation.
BOOL __stdcall WinUsb_Transfer(HANDLE InterfaceHandle, UCHAR PipeID,
                               PUCHAR Buffer, ULONG BufferLength,
                               PULONG LengthTransferred,
                               LPOVERLAPPED Overlapped) {
  if (InterfaceHandle == INVALID_HANDLE_VALUE) return FALSE;
  if (device_handle[(int)InterfaceHandle] == NULL) return FALSE;
  int ret;
  if ((PipeID & 0x80) && (PipeID & 0x7F) > 1 && BufferLength < 64)
    BufferLength = 64;
  if (Overlapped != NULL)  // If an asynchronous transfer is needed then pack
                           // all the arguments to an Arg structure and pass them
                           // to a new thread and return immediately.
  {
    ResetEvent(Overlapped->hEvent);
    Args *args = malloc(sizeof(*args));
    args->InterfaceHandle = InterfaceHandle;
    args->PipeID = PipeID;
    args->Buffer = Buffer;
    args->BufferLength = BufferLength;
    args->LengthTransferred = LengthTransferred;
    args->Overlapped = Overlapped;
    CreateThread(NULL, 0, (void *)async_transfer, args, 0, NULL);
    return TRUE;
  } else  // Just an synchronous transfer is needed; just call the
          // libusb_bulk_transfer.
  {
    ret = libusb_bulk_transfer(device_handle[(int)InterfaceHandle], PipeID,
                               Buffer, BufferLength, LengthTransferred, 20000);
  }

  return (ret == LIBUSB_SUCCESS);
}

void notifier_function() {
  struct udev *udev;
  struct udev_monitor *mon;
  struct udev_device *dev;

  const GUID guid = {0xE7E8BA13,
                     0x2A81,
                     0x446E,
                     {0xA1, 0x1E, 0x72, 0x39, 0x8F, 0xBD, 0xA8, 0x2F}};

  DEV_BROADCAST_DEVICEINTERFACE_W DevBi;
  DevBi.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE_W);
  DevBi.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
  DevBi.dbcc_classguid = guid;

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
            close_devices();
            usleep(100000);
            SendMessageA(hWnd, WM_DEVICECHANGE, DBT_DEVICEARRIVAL,
                         (LPARAM)&DevBi);
            usleep(100000);
            RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
          }

        } else if (!strcasecmp(udev_device_get_action(dev), "remove")) {
          count_new = get_device_count();
          if (count != count_new) {
            count = count_new;
            // printf("device removed.\n");
            close_devices();
            usleep(100000);
            SendMessageA(hWnd, WM_DEVICECHANGE, DBT_DEVICEREMOVECOMPLETE,
                         (LPARAM)&DevBi);
            usleep(100000);
            RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
          }
        }
        udev_device_unref(dev);
      }
    }
    usleep(10000);
  }
  udev_monitor_unref(mon);
}

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
    if (!dev) return -1;

    const char *vid = udev_device_get_sysattr_value(dev, "idVendor");
    const char *pid = udev_device_get_sysattr_value(dev, "idProduct");
    ;
    if (vid && pid && strtoul(vid, NULL, 16) == TL866_VID &&
        strtoul(pid, NULL, 16) == TL866_PID)
      count++;
    udev_device_unref(dev);
  }
  udev_enumerate_unref(enumerate);
  udev_unref(udev);
  return count;
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
  switch (fdwReason) {
    case DLL_WINE_PREATTACH:
      return TRUE;
    case DLL_PROCESS_ATTACH:
      DisableThreadLibraryCalls(hinstDLL);
      printf("Dll Loaded.\n");
      if (!patch_xgpro()) {
        printf("Dll Unloaded.\n");
        return FALSE;
      }
      break;
    case DLL_PROCESS_DETACH:
      cancel = TRUE;
      WaitForSingleObject(h_thread, 5000);
      printf("Dll Unloaded.\n");
      break;
  }

  return TRUE;
}
// SetupApi redirected functions needed for the new wine 4.11+

typedef HANDLE(__stdcall *pSetupDiGetClassDevsW)(const GUID *, PCWSTR, HWND,
                                                 DWORD);
typedef BOOL(__stdcall *pSetupDiEnumDeviceInfo)(HANDLE, DWORD, LPVOID);
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
  return address;
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

__stdcall BOOL SetupDiGetDevicePropertyW(
    HANDLE DeviceInfoSet, LPVOID DeviceInfoData, const LPVOID *PropertyKey,
    LPVOID *PropertyType, PBYTE PropertyBuffer, DWORD PropertyBufferSize,
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

__stdcall BOOL SetupDiSetDevicePropertyW(
    HANDLE DeviceInfoSet, LPVOID DeviceInfoData, const LPVOID *PropertyKey,
    LPVOID PropertyType, const PBYTE PropertyBuffer, DWORD PropertyBufferSize,
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

__stdcall BOOL SetupDiSetDeviceRegistryPropertyW(HANDLE DeviceInfoSet,
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
