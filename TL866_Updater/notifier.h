#ifndef NOTIFIER_H
#define NOTIFIER_H

#include <QWidget>
#include <QSocketNotifier>

class Notifier : public QWidget
{
    Q_OBJECT
public:
    explicit Notifier(QWidget *parent = 0);

 ~Notifier();
signals:
    void deviceChange(bool arrived);

public slots:

private slots:
        void udev_event();

private:
    QSocketNotifier *socket_notifier;
    void RegisterUsbNotifications();

#ifdef Q_OS_WIN32
private:
    bool winEvent(MSG *message, long *result);
#endif

};

#endif // NOTIFIER_H
