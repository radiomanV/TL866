/* Class USB, macOS version
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

#include "usb_macos.h"
#include "firmware.h"
#include <QDebug>

USB::USB()
{
    device_handle = nullptr;
    devs = nullptr;
    libusb_init(nullptr);//initialize a new session
    libusb_set_option(nullptr, LIBUSB_OPTION_LOG_LEVEL, 3);//set verbosity level//set verbosity level
}

USB::~USB()
{
    if(isOpen())
        close_device();

    if(devs != nullptr)
        libusb_free_device_list(devs, 1);
    libusb_exit(nullptr); //close session
}


int USB::get_devices_count()
{
   if(devs != nullptr)
       libusb_free_device_list(devs, 1);
    devices.clear();

    int count = libusb_get_device_list(nullptr, &devs);
    if(count < 0) {
        return 0;
    }

    for(int i = 0; i < count; i++) {
        libusb_device_descriptor desc;
        int ret = libusb_get_device_descriptor(devs[i], &desc);
        if (ret < 0) {
            return 0;
        }
        if(desc.idProduct == TL866_PID && desc.idVendor == TL866_VID)
        {
            devices.append(devs[i]);
        }

    }
    return devices.size();
}


bool USB::open_device(int index)
{
    if(isOpen())
        close_device();

    return (libusb_open(devices[index], &device_handle) == 0);

}


bool USB::isOpen()
{
    return (device_handle !=nullptr);
}



void  USB::close_device()
{
    if(isOpen())
        libusb_close(device_handle);
    device_handle = nullptr;
}


size_t  USB::usb_read(unsigned char *data, size_t size)
{
    int bytes_read;
    if(libusb_claim_interface(device_handle, 0) < 0)
        return 0;
    int ret = libusb_bulk_transfer(device_handle, LIBUSB_ENDPOINT_IN | 1, data, static_cast<int>(size), &bytes_read, 0);
    libusb_release_interface(device_handle, 0);
    if(ret !=0)
        return 0;
    return static_cast<size_t>(bytes_read);
}


size_t  USB::usb_write(unsigned char *data, size_t size)
{
    int bytes_writen;
    if(libusb_claim_interface(device_handle, 0) < 0)
        return 0;
    int ret = libusb_bulk_transfer(device_handle, LIBUSB_ENDPOINT_OUT | 1, data, static_cast<int>(size), &bytes_writen, 0);
    libusb_release_interface(device_handle, 0);
    if(ret !=0)
        return 0;
    return static_cast<size_t>(bytes_writen);
}
