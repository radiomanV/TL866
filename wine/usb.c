#define __WINESRC__
#define __CYGWIN__


#include <glob.h>
#include <libusb-1.0/libusb.h>
#include <libudev.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#include <windef.h>
#include <winbase.h>
#include <winnt.h>
#include <dbt.h>


//Low level function addresses in minipro.exe

#define MINIPRO_VERSION					650

#if MINIPRO_VERSION == 650

//Minipro v6.50
#define MINIPRO_USB_OPEN_DEVICES_ADDRESS                0x4A850
#define MINIPRO_CLOSE_DEVICES_ADDRESS                   0x61AB0
#define MINIPRO_USB_WRITE_ADDRESS                       0x61920
#define MINIPRO_USB_READ_ADDRESS                        0x619C0
#define MINIPRO_USB_WRITE2_ADDRESS                      0x61970
#define MINIPRO_USB_READ2_ADDRESS                       0x61A70
#define MINIPRO_REGISTER_DEVICE_NOTIFICATIONS_ADDRESS   0xA36D4
#define MINIPRO_USB_HANDLE_ADDRESS                      0xE752C


//Extracted from minipro PE header
#define MINIPRO_CODE_SECTION_SIZE                       0xA2000
#define MINIPRO_RDATA_SECTION_OFFSET                    0xA3000
#define MINIPRO_RDATA_SECTION_SIZE                      0x33000

#elif MINIPRO_VERSION == 613

//Minipro v6.13/6.16
#define MINIPRO_USB_OPEN_DEVICES_ADDRESS                0x4A400
#define MINIPRO_CLOSE_DEVICES_ADDRESS                   0x61120
#define MINIPRO_USB_WRITE_ADDRESS                       0x60F90
#define MINIPRO_USB_READ_ADDRESS                        0x61030
#define MINIPRO_USB_WRITE2_ADDRESS                      0x60FE0
#define MINIPRO_USB_READ2_ADDRESS                       0x610E0
#define MINIPRO_REGISTER_DEVICE_NOTIFICATIONS_ADDRESS   0xA26D4
#define MINIPRO_USB_HANDLE_ADDRESS                      0xE52CC


//Extracted from minipro PE header
#define MINIPRO_CODE_SECTION_SIZE                       0xA1000
#define MINIPRO_RDATA_SECTION_OFFSET                    0xA2000
#define MINIPRO_RDATA_SECTION_SIZE                      0x32000

#else
//Minipro V6.00/6.10

#define MINIPRO_USB_OPEN_DEVICES_ADDRESS                0x4A3D0
#define MINIPRO_CLOSE_DEVICES_ADDRESS                   0x610F0
#define MINIPRO_USB_WRITE_ADDRESS                       0x60F60
#define MINIPRO_USB_READ_ADDRESS                        0x61000
#define MINIPRO_USB_WRITE2_ADDRESS                      0x60FB0
#define MINIPRO_USB_READ2_ADDRESS                       0x610B0
#define MINIPRO_REGISTER_DEVICE_NOTIFICATIONS_ADDRESS   0xA26D4
#define MINIPRO_USB_HANDLE_ADDRESS                      0xE52CC


//Extracted from minipro PE header
#define MINIPRO_CODE_SECTION_SIZE                       0xA1000
#define MINIPRO_RDATA_SECTION_OFFSET                    0xA2000
#define MINIPRO_RDATA_SECTION_SIZE                      0x32000

#endif

#define TL866_VID 0x04d8
#define TL866_PID 0xe11c

HANDLE *usb_handle;


//replacement functions for minipro. Function prototypes and calling convention must be the same as in minipro.exe, otherwise the application will crash.
int  open_devices(GUID guid, int *devices);
void close_devices();
BOOL  usb_write(unsigned char *lpInBuffer, unsigned int nInBufferSize);
unsigned int  usb_read(unsigned char *lpOutBuffer, unsigned int nBytesToRead, unsigned int nOutBufferSize);
BOOL  usb_write2(HANDLE hDevice, unsigned char *lpInBuffer, unsigned int nInBufferSize);
unsigned int  usb_read2(HANDLE hDevice, unsigned char *lpOutBuffer, unsigned int nBytesToRead, unsigned int nOutBufferSize);
HANDLE __stdcall RegisterDeviceNotifications(HANDLE hRecipient,LPVOID NotificationFilter,DWORD Flags);


//helper functions
unsigned int  uread(HANDLE hDevice, unsigned char *data, size_t size);
BOOL  uwrite(HANDLE hDevice, unsigned char *data, size_t size);
void  notifier_function();
int get_device_count();

libusb_device_handle *device_handle[4];
libusb_context *ctx;
libusb_device **devs;

pthread_t notifier_id;
pthread_mutex_t lock;

HWND hWnd;
BOOL cancel;

//Patcher function. Called from DllMain
void patch_minipro()
{
    BYTE t[] = {0x68, 0, 0, 0, 0, 0xc3};// push xxxx, ret
    DWORD dwOldProtection;
    LPVOID baseAddress = GetModuleHandleA(NULL);
    usb_handle = baseAddress + MINIPRO_USB_HANDLE_ADDRESS;
    VirtualProtect(baseAddress, MINIPRO_CODE_SECTION_SIZE, PAGE_EXECUTE_READWRITE, &dwOldProtection);//unprotect the code memory section

    //patch Open_Devices function
    *((DWORD *) &t[1]) = (DWORD)&open_devices;
    memcpy(baseAddress + MINIPRO_USB_OPEN_DEVICES_ADDRESS, t, 6);

    //patch close_devices function
    *((DWORD *) &t[1]) = (DWORD)&close_devices;
    memcpy(baseAddress + MINIPRO_CLOSE_DEVICES_ADDRESS, t, 6);

    //patch USB_Write function
    *((DWORD *) &t[1]) = (DWORD)&usb_write;
    memcpy(baseAddress + MINIPRO_USB_WRITE_ADDRESS, t, 6);

    //patch USB_Read function
    *((DWORD *) &t[1]) = (DWORD)&usb_read;
    memcpy(baseAddress + MINIPRO_USB_READ_ADDRESS, t, 6);

    //patch USB_Write2 function
    *((DWORD *) &t[1]) = (DWORD)&usb_write2;
    memcpy(baseAddress + MINIPRO_USB_WRITE2_ADDRESS, t, 6);

    //patch USB_Read2 function
    *((DWORD *) &t[1]) = (DWORD)&usb_read2;
    memcpy(baseAddress + MINIPRO_USB_READ2_ADDRESS, t, 6);

    VirtualProtect(baseAddress, MINIPRO_CODE_SECTION_SIZE, dwOldProtection, &dwOldProtection);//restore the old protection

    //patch RegisterDeviceNotifications function
    VirtualProtect(baseAddress + MINIPRO_RDATA_SECTION_OFFSET, MINIPRO_RDATA_SECTION_SIZE, PAGE_EXECUTE_READWRITE, &dwOldProtection);//unprotect the .rdata memory section
    *((DWORD *) &t[1]) = (DWORD)&RegisterDeviceNotifications;
    memcpy(baseAddress + MINIPRO_REGISTER_DEVICE_NOTIFICATIONS_ADDRESS, &t[1], 4);
    VirtualProtect(baseAddress + MINIPRO_RDATA_SECTION_OFFSET, MINIPRO_RDATA_SECTION_SIZE, dwOldProtection, &dwOldProtection);//restore the old protection
}


//Minipro replacement functions
int open_devices(GUID guid, int *devices)
{
    printf("Open devices.\n");
    close_devices();
    device_handle[0] = NULL;
    device_handle[1] = NULL;
    device_handle[2] = NULL;
    device_handle[3] = NULL;
    devs = NULL;

    libusb_init(&ctx);//initialize a new session
    libusb_set_debug(ctx, 3);//set verbosity level


    *usb_handle = INVALID_HANDLE_VALUE;
    *(usb_handle + 1) = INVALID_HANDLE_VALUE;
    *(usb_handle + 2) = INVALID_HANDLE_VALUE;
    *(usb_handle + 3) = INVALID_HANDLE_VALUE;

    int devices_found = 0, i, ret;
    struct libusb_device_descriptor desc;
    int count = libusb_get_device_list(ctx, &devs);

    if(count < 0) {
        return -1;
    }


    for(i = 0; i < count; i++) {
        ret = libusb_get_device_descriptor(devs[i], &desc);
        if (ret < 0) {
            return 0;
        }

        if(TL866_PID == desc.idProduct && TL866_VID == desc.idVendor)
        {
            if (libusb_open(devs[i], &device_handle[devices_found]) == 0)
            {
                *(usb_handle+devices_found) = (HANDLE)devices_found;
                devices_found++;
                if (devices_found == 4)
                    return 1;
            }
        }

    }
    return 1;
}



void close_devices()
{
    printf("Close devices.\n");
    if(devs != NULL)
    {

        int i;
        pthread_mutex_lock(&lock);
        for(i = 0; i<4; i++)
        {
            if(device_handle[i] != NULL)
            {
                libusb_close(device_handle[i]);
                device_handle[i] = NULL;
            }
        }
        libusb_free_device_list(devs, 1);
        libusb_exit(ctx);//close session
        devs = NULL;
        pthread_mutex_unlock(&lock);
    }
}


BOOL usb_write(unsigned char *lpInBuffer, unsigned int nInBufferSize)
{
    pthread_mutex_lock(&lock);
    BOOL ret = uwrite(0, lpInBuffer, nInBufferSize);
    pthread_mutex_unlock(&lock);
    return ret;
}


unsigned int usb_read(unsigned char *lpOutBuffer, unsigned int nBytesToRead, unsigned int nOutBufferSize)
{
    pthread_mutex_lock(&lock);
    unsigned int ret = uread(0, lpOutBuffer, nBytesToRead);
    pthread_mutex_unlock(&lock);
    if(ret == 0xFFFFFFFF)
        MessageBoxA(GetForegroundWindow(), "Read error!", "TL866", MB_ICONWARNING);
    return ret;
}


BOOL usb_write2(HANDLE hDevice, unsigned char *lpInBuffer, unsigned int nInBufferSize)
{
    pthread_mutex_lock(&lock);
    BOOL ret = uwrite(hDevice, lpInBuffer, nInBufferSize);
    pthread_mutex_unlock(&lock);
    return ret;
}


unsigned int usb_read2(HANDLE hDevice, unsigned char *lpOutBuffer, unsigned int nBytesToRead, unsigned int nOutBufferSize)
{
    pthread_mutex_lock(&lock);
    unsigned int ret = uread(hDevice, lpOutBuffer, nBytesToRead);
    pthread_mutex_unlock(&lock);
    return ret;
}


HANDLE __stdcall RegisterDeviceNotifications(HANDLE hRecipient,LPVOID NotificationFilter,DWORD Flags)
{

    printf("RegisterDeviceNotifications hWnd=%X4\n", (unsigned int)hRecipient);
    hWnd = hRecipient;
    int tr = pthread_create(&notifier_id, NULL, (void*)notifier_function, NULL);
    if (tr)
        printf("Thread notifier failed.\n");

    return 0;
}


//Libusb functions

unsigned int  uread(HANDLE hDevice, unsigned char *data, size_t size)
{
    if(hDevice == INVALID_HANDLE_VALUE)
        return 0;
    if(device_handle[(int)hDevice] == NULL)
        return 0;
    size_t bytes_read;
    if(libusb_claim_interface(device_handle[(int)hDevice], 0) < 0)
        return 0;
    int ret = libusb_bulk_transfer(device_handle[(int)hDevice], LIBUSB_ENDPOINT_IN | 1, data, size, &bytes_read, 3000);
    libusb_release_interface(device_handle[(int)hDevice], 0);
    return (ret == LIBUSB_SUCCESS ? bytes_read : 0xFFFFFFFF);
}


BOOL uwrite(HANDLE hDevice, unsigned char *data, size_t size)
{
    if(hDevice == INVALID_HANDLE_VALUE)
        return 0;
    if(device_handle[(int)hDevice] == NULL)
        return 0;
    size_t bytes_writen;
    if(libusb_claim_interface(device_handle[(int)hDevice], 0) < 0)
        return 0;
    int ret = libusb_bulk_transfer(device_handle[(int)hDevice], LIBUSB_ENDPOINT_OUT | 1, data, size, &bytes_writen, 3000);
    libusb_release_interface(device_handle[(int)hDevice], 0);
    return (ret == LIBUSB_SUCCESS);
}



void notifier_function()
{

    struct udev *udev;
    struct udev_monitor *mon;
    struct udev_device *dev;
    cancel = FALSE;
    const GUID guid = {0x85980D83,0x32B9,0x4BA1,{0x8F,0xDF,0x12,0xA7,0x11,0xB9,0x9C,0xA2}};
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
    if(!mon)
    {
        printf("NetLink not available!\n");
        return;
    }
    int count = get_device_count();
    if(count == -1)
    {
        printf("udev error.\n");
        return;
    }

    udev_monitor_filter_add_match_subsystem_devtype(mon, "usb", NULL);
    udev_monitor_enable_receiving(mon);
    int fd = udev_monitor_get_fd(mon);

    while (!cancel) {
        fd_set fds;
        struct timeval tv;
        int ret;

        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 0;

        ret = select(fd+1, &fds, NULL, NULL, &tv);
        if (ret > 0 && FD_ISSET(fd, &fds)) {

            dev = udev_monitor_receive_device(mon);
            if(dev && !strcmp(udev_device_get_devtype(dev),"usb_device")){
                int count_new;
                if(!strcmp(udev_device_get_action(dev), "add"))
                {
                    count_new = get_device_count();
                    if(count != count_new)
                    {
                        count = count_new;
                        //printf("device added.\n");
                        close_devices();
                        usleep(100000);
                        SendMessageW(hWnd, WM_DEVICECHANGE, DBT_DEVICEARRIVAL, (LPARAM)&DevBi);
                        RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
                    }

                }
                else if(!strcmp(udev_device_get_action(dev), "remove"))
                {
                    count_new = get_device_count();
                    if(count != count_new)
                    {
                        count = count_new;
                        //printf("device removed.\n");
                        close_devices();
                        usleep(100000);
                        SendMessageW(hWnd, WM_DEVICECHANGE, DBT_DEVICEREMOVECOMPLETE, (LPARAM)&DevBi);
                        RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
                    }
                }
                udev_device_unref(dev);
            }
        }
        usleep(10000);
    }
}


int get_device_count()
{
    struct udev *udev = udev_new();
    if (!udev)
    {
        return -1;
    }

    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;

    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "usb");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);
    const char *path;
    int count = 0;
    udev_list_entry_foreach(dev_list_entry, devices)
    {

        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);
        if(!dev)
            return -1;

        const char * vid = udev_device_get_sysattr_value(dev,"idVendor");
        const char * pid = udev_device_get_sysattr_value(dev,"idProduct");
        if((vid && pid) && (!strcmp(vid, "04d8") && (!strcmp(pid, "e11c"))))
            count++;
        udev_device_unref(dev);
    }
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
    return count;
}


/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{

    switch (fdwReason)
    {
    case DLL_WINE_PREATTACH:
        return TRUE;
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);
        printf("Dll Loaded.\n");
        patch_minipro();
        break;
    case DLL_PROCESS_DETACH:
        cancel =TRUE;
        pthread_join(notifier_id, NULL);
        break;
    }

    return TRUE;
}

