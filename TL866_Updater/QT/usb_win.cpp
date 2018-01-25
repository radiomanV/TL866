/* Class USB, windows version
*
* This file is part of the TL866 updater project.
*
* Copyright (C) radioman 2013
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
* USA.
*/


#include "usb_win.h"
#include <QDebug>
#include <Windows.h>
#include <SetupAPI.h>

#define TL866_IOCTL_READ  0x222004
#define TL866_IOCTL_WRITE 0x222000

USB::USB()
{
   hDriver=INVALID_HANDLE_VALUE;
   InitializeCriticalSection(&lock);
}

USB::~USB()
{
   if(hDriver !=INVALID_HANDLE_VALUE)
       close_device();
   DeleteCriticalSection(&lock);
}
const GUID MINIPRO_GUID={0x85980D83,0x32B9,0x4BA1,{0x8F,0xDF,0x12,0xA7,0x11,0xB9,0x9C,0xA2}};

int USB::get_devices_count()
{
    DWORD idx = 0;
    devices.clear();
    HDEVINFO handle = SetupDiGetClassDevs(&MINIPRO_GUID, 0, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (INVALID_HANDLE_VALUE == handle)
    {
        qDebug() << "SetupDi failed";
        return 0;
    }

    while (1)
    {
        SP_DEVINFO_DATA deviceinfodata;
        deviceinfodata.cbSize = sizeof(SP_DEVINFO_DATA);

        if (SetupDiEnumDeviceInfo(handle, idx, &deviceinfodata))
        {
            SP_DEVICE_INTERFACE_DATA deviceinterfacedata;
            deviceinterfacedata.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

            if (SetupDiEnumDeviceInterfaces(handle, 0, &MINIPRO_GUID, idx, &deviceinterfacedata))
            {
                idx++;
                DWORD size = 0;

                SetupDiGetDeviceInterfaceDetail(handle, &deviceinterfacedata, NULL, 0, &size, NULL);
                PSP_DEVICE_INTERFACE_DETAIL_DATA deviceinterfacedetaildata = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(size * sizeof(TCHAR));
                deviceinterfacedetaildata->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
                DWORD datasize = size;

                if (SetupDiGetDeviceInterfaceDetail(handle, &deviceinterfacedata, deviceinterfacedetaildata, datasize , &size, NULL))
#ifdef UNICODE
                    devices.append(QString::fromWCharArray(deviceinterfacedetaildata->DevicePath));
#else
                    devices.append(deviceinterfacedetaildata->DevicePath);
#endif             free(deviceinterfacedetaildata);
            }
        }
        else
        {
            break;
        }
    }
    return devices.count();
}

bool USB::open_device(int index)
{
    hDriver = CreateFileA(devices.at(index).toLatin1().data(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    return (hDriver != INVALID_HANDLE_VALUE);
}

void USB::close_device()
{
    CloseHandle(hDriver);
    hDriver=NULL;
}


bool USB::isOpen()
{
   return (hDriver != INVALID_HANDLE_VALUE);
}

size_t USB::usb_read(unsigned char *data, DWORD size)
{
    if (hDriver == INVALID_HANDLE_VALUE)
        return 0;
    DWORD bytes_read;
    uchar *buffer = new uchar[5];
    EnterCriticalSection(&lock);
    bool ret = DeviceIoControl(hDriver, TL866_IOCTL_READ, buffer, 5, data, size, &bytes_read, NULL);
    LeaveCriticalSection(&lock);
    delete buffer;
    return (ret ? bytes_read : 0);
}

size_t USB::usb_write(unsigned char *data, DWORD size)
{
    if (hDriver == INVALID_HANDLE_VALUE)
        return 0;
    DWORD bytes_written;
    uchar *buffer = new uchar[256];
    EnterCriticalSection(&lock);
    bool ret = DeviceIoControl(hDriver, TL866_IOCTL_WRITE, data, size, buffer, 256, &bytes_written, NULL);
    LeaveCriticalSection(&lock);
    delete buffer;
    return (ret ? bytes_written : 0);
}
