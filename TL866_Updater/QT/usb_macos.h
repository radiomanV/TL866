#ifndef USB_MACOS_H
#define USB_MACOS_H

#include <glob.h>
#include <QList>
#include <libusb-1.0/libusb.h>

class USB
{
public:
    USB();
    ~USB();

    int     get_devices_count();
    bool    open_device(int index);
    bool    isOpen();
    void close_device();
    size_t  usb_read(unsigned char *data, size_t size);
    size_t  usb_write(unsigned char *data, size_t size);


private:
    libusb_context *ctx;
    libusb_device_handle *device_handle;
    QList<libusb_device*> devices;
    libusb_device **devs;
};

#endif // USB_MACOS_H
