#ifndef NOTIFIER_WIN_H
#define NOTIFIER_WIN_H

#include <QWidget>


class Notifier : public QWidget
{
    Q_OBJECT
public:
    explicit Notifier(QWidget *parent = 0);

signals:
    void deviceChange(bool arrived);

private:
    void RegisterUsbNotifications();


private:
    bool winEvent(MSG *message, long *result);
#if QT_VERSION >= 0x050000
    bool nativeEvent(const QByteArray& eventType, void* message, long* result);
#endif

};

#endif // NOTIFIER_WIN_H
