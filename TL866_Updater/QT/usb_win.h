#ifndef USB_WIN_H
#define USB_WIN_H
#include <QStringList>
#include <Windows.h>

class USB
{
public:
    USB();
    ~USB();

    int     get_devices_count();
    bool    open_device(int index);
    bool    isOpen();
    void close_device();
    size_t  usb_read(unsigned char *data, DWORD size);
    size_t  usb_write(unsigned char *data, DWORD size);

private:
    QStringList devices;
    HANDLE hDriver;
    CRITICAL_SECTION lock;
};

#endif // USB_WIN_H
