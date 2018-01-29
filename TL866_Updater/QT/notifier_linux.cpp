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
#include "firmware.h"
#include <QDebug>

Notifier::Notifier()
{
    socket_notifier=NULL;
    udev *udev = udev_new();
    if (!udev)
    {
        qDebug() << "udev error!" << endl;
        return;
    }

    //Building the initial device list.
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
        if((vid.toUShort(0,16) == TL866_VID) && (pid.toUShort(0,16) == TL866_PID))
        {
            nodes.append(devnode);
            //qDebug()<< "Found" << devnode << vid << pid << udev_device_get_sysattr_value(dev,"product");
        }
        udev_device_unref(dev);
    }
    udev_enumerate_unref(enumerate);

    //Rgistering usb notifications.
    mon = udev_monitor_new_from_netlink(udev, "udev");
    if(!mon)
    {
        qDebug() << "Netlink not available!" << endl;
        udev_unref(udev);
        return;
    }
    udev_monitor_filter_add_match_subsystem_devtype(mon, "usb", "usb_device");
    udev_monitor_enable_receiving(mon);
    int fd = udev_monitor_get_fd(mon);
    socket_notifier=new QSocketNotifier(fd,QSocketNotifier::Read);
    socket_notifier->setEnabled(true);
    connect(socket_notifier,SIGNAL(activated(int)),this,SLOT(udev_event()));
    udev_unref(udev);
    qDebug() << "Register device notification O.K.";
}


Notifier::~Notifier()
{
    udev_monitor_unref(mon);
    if(socket_notifier !=NULL)
        delete socket_notifier;
}

/* Called automatically when a usb device is inserted or removed.
 * Because we can get the idVendor and idProduct only for inserted
 * devices, for remove device notification we don't know if that
 * device was an TL866 or else. So we keep a list of devices to
 * track which device was removed.
 */
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
