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


#include "notifier_linux.h"
#include "tl866_global.h"
#include <QDebug>
#include <libudev.h>

Notifier::Notifier()
{
    socket_notifier=NULL;
    RegisterUsbNotifications();
}

Notifier::~Notifier()
{
    if(socket_notifier !=NULL)
        delete socket_notifier;
}


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
