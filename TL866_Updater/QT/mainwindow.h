#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#if QT_VERSION >= 0x050000
    #include <QtConcurrent/QtConcurrentMap>
    #include <QtConcurrent/QtConcurrentRun>
#else
    #include <QtConcurrentMap>
    #include <QtConcurrentRun>
#endif

#include <QDebug>
#include <QLineEdit>
#include <QTimer>
#include "advdialog.h"
#include "firmware.h"

#ifdef Q_OS_WIN32
#include "usb_win.h"
#include "notifier_win.h"
#endif

#ifdef Q_OS_LINUX
#include "usb_linux.h"
#include "notifier_linux.h"
#endif

#ifdef Q_OS_DARWIN
#include "usb_macos.h"
#include "notifier_macos.h"
#endif


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    struct DeviceInfo
    {
       QString device_code;
       QString device_serial;
       uint device_type;
    };

private slots:
    void on_btnInput_clicked();
    void on_btnAdvanced_clicked();
    void on_btnEdit_clicked();
    void on_btnDefault_clicked();
    void on_btnClone_clicked();
    void on_btnReflash_clicked();
    void on_btnReset_clicked();
    void on_btnSave_clicked();
    void on_btnDump_clicked();
    void reflash_finished(const QString &result);
    void dump_finished(const QString &result);
    DeviceInfo DeviceChanged(bool arrived);
    void gui_updated(const QString &message, bool eraseLed, bool writeLed);
    void TimerUpdate();
    void WriteBootloader(Firmware::BootloaderType type);
    void WriteConfig(bool copy_protect);
    void WriteInfo(const QString &device_code, const QString &serial_number);


    void on_cp0_stateChanged(int arg1);

signals:
    void reflash_status(const QString &result);
    void dump_status(const QString &result);
    void update_gui(const QString &message, bool eraseLed, bool writeLed);
    void update_progress(int value);


private:
    Ui::MainWindow *ui;
    USB *usb_device;
    AdvDialog* advdlg;
    Firmware *firmware;
    Notifier *usbNotifier;
    QTimer *timer;
    QFuture<void> *worker;
    bool reset_flag;

    QByteArray get_resource(const QString &resource_path, int size);
    void leds_off();
    void setNled(bool state);
    void setBled(bool state);
    void setEled(bool state);
    void setWled(bool state);
    void wait_ms(unsigned long time);
    void SetBlank();
    const QString GetFormatedString(const QString &devcode, const QString &serial);
    bool CheckDevices(QWidget *parent);
    bool AdvQuestion();
    uint BootloaderCRC();
    void reflash(uint firmware_type);
    void dump(const QString &fileName, uint device_type);
    void reset();
    bool wait_for_device();
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void OpenFile(const QString &fileName);

#define A_FIRMWARE_RESOURCE     ":/firmware/firmwareA.bin"
#define CS_FIRMWARE_RESOURCE    ":/firmware/firmwareCS.bin"
#define DUMPER_RESOURCE         ":/firmware/dumper.bin"
#define VERSION                 "2.52"
};

#endif // MAINWINDOW_H
