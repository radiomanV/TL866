/* Class Notifier
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


#include "notifier.h"
#include "tl866_global.h"
#include <QDebug>

Notifier::Notifier(QWidget *parent) :
    QWidget(parent)
{
    this->hide();
    socket_notifier=NULL;
    RegisterUsbNotifications();
}

Notifier::~Notifier()
{
    if(socket_notifier !=NULL)
        delete socket_notifier;
}


/* Platform specific implementation of usb device change notification.
 * We use a hidden widget class because we need the winId (hwnd) for Windows implementation.
 */

//Usb Device change Windows implementation
#ifdef Q_OS_WIN32
#include <Windows.h>
#include <Dbt.h>

const GUID MINIPRO_GUID={0x85980D83,0x32B9,0x4BA1,{0x8F,0xDF,0x12,0xA7,0x11,0xB9,0x9C,0xA2}};

void Notifier::RegisterUsbNotifications()
{
    DEV_BROADCAST_DEVICEINTERFACE deviceInterface;
    ZeroMemory(&deviceInterface, sizeof(deviceInterface));
    deviceInterface.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    deviceInterface.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    deviceInterface.dbcc_classguid  = MINIPRO_GUID;
    HDEVNOTIFY m_notify_handle = RegisterDeviceNotification((HANDLE)this->winId(),&deviceInterface, DEVICE_NOTIFY_WINDOW_HANDLE);
    if(m_notify_handle==NULL)
    {
        qDebug() << "Failed to register device notification!";
        return;
    }

    qDebug() << "Register device notification O.K.";
}

bool Notifier::winEvent(MSG *message, long *result)
{
    Q_UNUSED(result);
    if (message->message==WM_DEVICECHANGE)
    {
        if (message->wParam==DBT_DEVICEARRIVAL)
            emit deviceChange(true);
        if (message->wParam==DBT_DEVICEREMOVECOMPLETE)
            emit deviceChange(false);
    }
    return false;
}

void Notifier::udev_event()
{
    //stub function
}

#endif


//Usb Device change Linux implementation
#ifdef Q_OS_LINUX

#include <libudev.h>

udev_monitor *mon;//Global variable
QStringList nodes;


void Notifier::udev_event()
{
    udev_device *dev = udev_monitor_receive_device(mon);
    if(dev)
    {
        QString devnode(udev_device_get_devnode(dev));
        QString vid(udev_device_get_sysattr_value(dev,"idVendor"));
        QString pid(udev_device_get_sysattr_value(dev,"idProduct"));
        QString action(udev_device_get_action(dev));
        if(action.toLower() == "add" && vid.toUShort(0,16) == TL866_VID && pid.toUShort(0,16) == TL866_PID)
        {
            if(!nodes.contains(devnode,Qt::CaseInsensitive))
            {
                nodes.append(devnode);
                //qDebug() << devnode <<" added";
            }
            emit deviceChange(true);
        }
        if(action.toLower() == "remove")
        {
            if(nodes.contains(devnode,Qt::CaseInsensitive))
            {

                //qDebug() << devnode << " removed";
                nodes.removeOne(devnode);
                emit deviceChange(false);
            }
        }

    }
    udev_device_unref(dev);
}


void Notifier::RegisterUsbNotifications()
{
    udev *udev = udev_new();
    if (!udev)
    {
        qDebug() << "udev error!" << endl;
        return;
    }

    udev_enumerate *enumerate;
    udev_list_entry *devices, *dev_list_entry;
    udev_device *dev;

    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "usb");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);
    nodes.clear();
    const char *path;
    udev_list_entry_foreach(dev_list_entry, devices)
    {

        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);
        QString devnode(udev_device_get_devnode(dev));
        QString vid(udev_device_get_sysattr_value(dev,"idVendor"));
        QString pid(udev_device_get_sysattr_value(dev,"idProduct"));
        //QString product(udev_device_get_sysattr_value(dev,"product"));
        if((vid.toUShort(0,16) == TL866_VID) && (pid.toUShort(0,16) == TL866_PID))
        {
            nodes.append(devnode);
            //qDebug()<< "Found" << devnode << vid << pid << product;
        }
        udev_device_unref(dev);
    }
    udev_enumerate_unref(enumerate);

    mon = udev_monitor_new_from_netlink(udev, "udev");
    if(!mon)
    {
        qDebug() << "Netlink not available!" << endl;
        return;
    }
    udev_monitor_filter_add_match_subsystem_devtype(mon, "usb", "usb_device");
    udev_monitor_enable_receiving(mon);
    int fd = udev_monitor_get_fd(mon);
    socket_notifier=new QSocketNotifier(fd,QSocketNotifier::Read);
    connect(socket_notifier,SIGNAL(activated(int)),this,SLOT(udev_event()));
    qDebug() << "Register device notification O.K.";
}
#endif

