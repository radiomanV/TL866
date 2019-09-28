#ifndef NOTIFIER_MACOS_H
#define NOTIFIER_MACOS_H

#include <QWidget>
#include <QSocketNotifier>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOMessage.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>

class Notifier : public QWidget
{
    Q_OBJECT

    public:
    explicit Notifier();
    ~Notifier();
    void DeviceAdded(io_iterator_t iterator);

    class NotificationObj {
        public:
        NotificationObj(Notifier *aParent) { parent = aParent; }
        void DeviceNotification(io_service_t service, natural_t messageType, void *messageArgument);

        io_object_t             notification;
        IOUSBDeviceInterface    **deviceInterface;
        CFStringRef             deviceName;
        UInt32                  locationID;

        private:
        Notifier*               parent;         // workaround for only having one ref pointer in callback
    };

    signals:
    void deviceChange(bool arrived);

    public slots:

    private slots:

    private:

    QSocketNotifier *socket_notifier;
    QStringList nodes;

    IONotificationPortRef    notifyPort;
    io_iterator_t            addedIter;
    CFRunLoopRef             runLoop;

    std::vector <NotificationObj*> notObjVec;
};

#endif // NOTIFIER_MACOS_H
