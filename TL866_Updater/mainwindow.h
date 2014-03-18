#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtConcurrentMap>
#include <QFutureWatcher>
#include <QDebug>
#include <QLineEdit>
#include "advdialog.h"
#include "firmware.h"
#include "notifier.h"
#include "tl866_global.h"

#ifdef Q_OS_WIN32
#include "usb_win.h"
#endif

#ifdef Q_OS_LINUX
#include "usb_linux.h"
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
    USB *usb_device;

    void set_default(QLineEdit *devcode, QLineEdit *serial);
    void Refresh();
    void WriteBootloader(BootloaderType type);
    void WriteConfig(bool copy_protect);
    void WriteInfo(QString device_code, QString serial_number);



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
    void reflash_finished(bool success);
    void dump_finished(QString succes);
    void DeviceChanged(bool arrived);
    void gui_updated(QString message, bool eraseLed, bool writeLed);


signals:
    void reflash_status(bool success);
    void dump_status(QString result);
    void update_gui(QString message, bool eraseLed, bool writeLed);
    void update_progress(int value);


private:
    enum WorkerJob{REFLASH, DUMP};
    Ui::MainWindow *ui;
    AdvDialog* advdlg;
    Firmware firmware;
    Notifier *usbNotifier;
    QList<WorkerJob> job_list;
    QFutureWatcher<void> watcher;
    QByteArray get_resource(QString resource_path, int size);
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
    ushort BootloaderCRC();
    bool reflash();
    QString dump();
    void reset();
    bool wait_for_device();
    void DoWork(WorkerJob job);

#define A_FIRMWARE_RESOURCE     ":/firmware/firmwareA.bin"
#define CS_FIRMWARE_RESOURCE    ":/firmware/firmwareCS.bin"
#define DUMPER_RESOURCE         ":/firmware/dumper.bin"

    struct WorkerWrapper
    {
        MainWindow *instance;
        WorkerWrapper(MainWindow *w): instance(w) {}
        void operator()(WorkerJob job) {
            instance->DoWork(job);
        }
    };

};

#endif // MAINWINDOW_H
