/* Class Notifier
*
* This file is part of the TL866 updater project.
*
* Copyright (C) radioman 2013
*
* macOS version by Troed Sångberg
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

#include <CoreFoundation/CoreFoundation.h>
#include "notifier_macos.h"
#include "firmware.h"
#include <QDebug>
#include <QWaitCondition>
#include <QMutex>
#include <QMutexLocker>

extern "C"
void device_callback(void *refcon, io_iterator_t iterator)
{
    Notifier *c = static_cast<Notifier *>(refcon);
    c->DeviceAdded(iterator);
}

extern "C"
void device_notification(void *refcon, io_service_t service, natural_t messageType, void *messageArgument)
{
    Notifier::NotificationObj *c = static_cast<Notifier::NotificationObj *>(refcon);
    c->DeviceNotification(service, messageType, messageArgument);
}

Notifier::Notifier()
{
    CFMutableDictionaryRef  matchingDict;
    CFRunLoopSourceRef      runLoopSource;
    CFNumberRef             numberRef;
    kern_return_t           kr;
    long                    usbVendor = TL866_VID;
    long                    usbProduct = TL866_PID;

    matchingDict = IOServiceMatching(kIOUSBDeviceClassName);    // Interested in instances of class
                                                                // IOUSBDevice and its subclasses
    if (matchingDict == NULL) {
        fprintf(stderr, "IOServiceMatching returned NULL.\n");
    }

    // We are interested in all USB devices (as opposed to USB interfaces).  The Common Class Specification
    // tells us that we need to specify the idVendor, idProduct, and bcdDevice fields, or, if we're not interested
    // in particular bcdDevices, just the idVendor and idProduct.  Note that if we were trying to match an 
    // IOUSBInterface, we would need to set more values in the matching dictionary (e.g. idVendor, idProduct, 
    // bInterfaceNumber and bConfigurationValue.
    
    // Create a CFNumber for the idVendor and set the value in the dictionary
    numberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &usbVendor);
    CFDictionarySetValue(matchingDict, 
                         CFSTR(kUSBVendorID), 
                         numberRef);
    CFRelease(numberRef);
    
    // Create a CFNumber for the idProduct and set the value in the dictionary
    numberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &usbProduct);
    CFDictionarySetValue(matchingDict, 
                         CFSTR(kUSBProductID), 
                         numberRef);
    CFRelease(numberRef);
    numberRef = NULL;
 
    // Create a notification port and add its run loop event source to our run loop
    // This is how async notifications get set up.
    
    notifyPort = IONotificationPortCreate(kIOMasterPortDefault);
    runLoopSource = IONotificationPortGetRunLoopSource(notifyPort);
    
    runLoop = CFRunLoopGetCurrent();
    CFRunLoopAddSource(runLoop, runLoopSource, kCFRunLoopDefaultMode);

    // Now set up a notification to be called when a device is first matched by I/O Kit.
    kr = IOServiceAddMatchingNotification(notifyPort,                  // notifyPort
                                          kIOFirstMatchNotification,    // notificationType
                                          matchingDict,                 // matching
                                          device_callback,                  // callback
                                          this,                         // refCon
                                          &addedIter                   // notification
                                          );        
                                            
    // Iterate once to get already-present devices and arm the notification    
    DeviceAdded(addedIter);
}


Notifier::~Notifier()
{
    for (auto p : notObjVec)
    {
        delete p;
    }
    notObjVec.clear();
}

void Notifier::NotificationObj::DeviceNotification(io_service_t service, natural_t messageType, void *messageArgument)
{
    kern_return_t   kr;
    
    if (messageType == kIOMessageServiceIsTerminated) {
        // Free the data we're no longer using now that the device is going away
        CFRelease(deviceName);
        
        if (deviceInterface) {
            kr = (*deviceInterface)->Release(deviceInterface);
        }
        
        kr = IOObjectRelease(notification);

        emit parent->deviceChange(false);
    }
}

void Notifier::DeviceAdded(io_iterator_t iterator)
{
    kern_return_t       kr;
    io_service_t        usbDevice;
    IOCFPlugInInterface **plugInInterface = NULL;
    SInt32              score;
    HRESULT             res;
    
    while ((usbDevice = IOIteratorNext(iterator))) {
        io_name_t       deviceName;
        CFStringRef     deviceNameAsCFString;   
        NotificationObj *notObj = NULL;
        UInt32          locationID;

        // Add some app-specific information about this device.
        // Create a buffer to hold the data.
        notObj = new NotificationObj(this);

        // Get the USB device's name.
        kr = IORegistryEntryGetName(usbDevice, deviceName);
        if (KERN_SUCCESS != kr) {
            deviceName[0] = '\0';
        }
        
        deviceNameAsCFString = CFStringCreateWithCString(kCFAllocatorDefault, deviceName, 
                                                         kCFStringEncodingASCII);
        
        // Save the device's name to our private data.
        notObj->deviceName = deviceNameAsCFString;
                                                
        // Now, get the locationID of this device. In order to do this, we need to create an IOUSBDeviceInterface 
        // for our device. This will create the necessary connections between our userland application and the 
        // kernel object for the USB Device.
        kr = IOCreatePlugInInterfaceForService(usbDevice, kIOUSBDeviceUserClientTypeID, kIOCFPlugInInterfaceID,
                                               &plugInInterface, &score);
 
        if ((kIOReturnSuccess != kr) || !plugInInterface) {
            fprintf(stderr, "IOCreatePlugInInterfaceForService returned 0x%08x.\n", kr);
            continue;
        }
 
        // Use the plugin interface to retrieve the device interface.
        res = (*plugInInterface)->QueryInterface(plugInInterface, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID),
                                                 (LPVOID*) &notObj->deviceInterface);
        
        // Now done with the plugin interface.
        (*plugInInterface)->Release(plugInInterface);
                    
        if (res || notObj->deviceInterface == NULL) {
            fprintf(stderr, "QueryInterface returned %d.\n", (int) res);
            continue;
        }
 
        // Now that we have the IOUSBDeviceInterface, we can call the routines in IOUSBLib.h.
        // In this case, fetch the locationID. The locationID uniquely identifies the device
        // and will remain the same, even across reboots, so long as the bus topology doesn't change.
        
        kr = (*notObj->deviceInterface)->GetLocationID(notObj->deviceInterface, &locationID);
        if (KERN_SUCCESS != kr) {
            fprintf(stderr, "GetLocationID returned 0x%08x.\n", kr);
            continue;
        }

        notObj->locationID = locationID;

        notObjVec.push_back(notObj);

        // Register for an interest notification of this device being removed. Use a reference to our
        // private data as the refCon which will be passed to the notification callback.
        kr = IOServiceAddInterestNotification(notifyPort,                       // notifyPort
                                              usbDevice,                        // service
                                              kIOGeneralInterest,               // interestType
                                              device_notification,              // callback
                                              notObj,                           // refCon
                                              &(notObj->notification)           // notification
                                              );
                                                
        if (KERN_SUCCESS != kr) {
            printf("IOServiceAddInterestNotification returned 0x%08x.\n", kr);
        }

        // Done with this USB device; release the reference added by IOIteratorNext
        kr = IOObjectRelease(usbDevice);

        // Extremely ugly solution to IOKit getting notification about new USB device long
        // before libusb. Proper solution would be to rewrite usb_macos.cpp to use IOKit as well
        QWaitCondition wc;
        QMutex mutex;
        QMutexLocker locker(&mutex);
        wc.wait(&mutex, 1500);

        emit deviceChange(true);

    }
}
