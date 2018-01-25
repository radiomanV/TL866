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


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

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
    void reflash_finished(QString result);
    void dump_finished(QString result);
    void DeviceChanged(bool arrived);
    void gui_updated(QString message, bool eraseLed, bool writeLed);
    void TimerUpdate();

    void Refresh();
    void WriteBootloader(Firmware::BootloaderType type);
    void WriteConfig(bool copy_protect);
    void WriteInfo(QString device_code, QString serial_number);


signals:
    void reflash_status(QString result);
    void dump_status(QString result);
    void update_gui(QString message, bool eraseLed, bool writeLed);
    void update_progress(int value);


private:
    enum WorkerJob{REFLASH, DUMP};
    Ui::MainWindow *ui;
    USB *usb_device;
    AdvDialog* advdlg;
    Firmware firmware;
    Notifier *usbNotifier;
    QByteArray get_resource(QString resource_path, int size);
    QTimer *timer;
    QFuture<void> worker;
    bool reset_flag;

    void leds_off();
    void setNled(bool state);
    void setBled(bool state);
    void setEled(bool state);
    void setWled(bool state);
    void wait_ms(unsigned long time);
    void SetBlank();
    bool CheckDevices(QWidget *parent);
    bool AdvQuestion();
    uint BootloaderCRC();
    void reflash(uint firmware_type);
    void dump(QString fileName, uint device_type);
    void reset();
    bool wait_for_device();
    bool IsBadCrc(const uchar *devcode, const uchar *serial);


#define A_FIRMWARE_RESOURCE     ":/firmware/firmwareA.bin"
#define CS_FIRMWARE_RESOURCE    ":/firmware/firmwareCS.bin"
#define DUMPER_RESOURCE         ":/firmware/dumper.bin"
};

#endif // MAINWINDOW_H
