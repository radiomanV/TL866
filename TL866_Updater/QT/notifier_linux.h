#ifndef NOTIFIER_LINUX_H
#define NOTIFIER_LINUX_H

#include <libudev.h>
#include <QWidget>
#include <QSocketNotifier>

class Notifier : public QWidget
{
    Q_OBJECT
public:
    explicit Notifier();

 ~Notifier();
signals:
    void deviceChange(bool arrived);

public slots:

private slots:
        void udev_event();

private:
    QSocketNotifier *socket_notifier;
    QStringList nodes;
    udev_monitor *mon;
};

#endif // NOTIFIER_LINUX_H
