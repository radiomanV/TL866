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


#include "notifier_win.h"
#include "tl866_global.h"
#include <Windows.h>
#include <Dbt.h>
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

/*
 * We use a hidden widget class because we need the winId (hwnd) for Windows implementation.
 */




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

#if QT_VERSION >= 0x050000
bool Notifier::nativeEvent(const QByteArray& eventType, void* message, long* result)
{
    Q_UNUSED( result );
    Q_UNUSED( eventType );

    MSG* msg = reinterpret_cast<MSG*>(message);
    if(msg->message == WM_DEVICECHANGE)
    {
        switch(msg->wParam)
        {
        case DBT_DEVICEARRIVAL:
              emit deviceChange(true);
            break;

        case DBT_DEVICEREMOVECOMPLETE:
              emit deviceChange(false);
            break;
        }
    }

    return false;
}
#else
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
#endif
