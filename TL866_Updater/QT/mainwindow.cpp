/* Class MainWindow
*
* This file is part of the TL866 updater project.
*
* Copyright (C) radioman 2014
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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "editdialog.h"
#include "hexwriter.h"
#include "crc.h"
#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QResource>
#include <QFuture>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //setup UI
    ui->setupUi(this);
    setFixedSize(size());

    //initialise object pointers
    advdlg = new AdvDialog(this);
    usbNotifier = new Notifier();
    usb_device = new USB();
    timer = new QTimer(this);

    //initialise used signals
    connect(usbNotifier,SIGNAL(deviceChange(bool)),this,SLOT(DeviceChanged(bool)));
    connect(this, SIGNAL(update_gui(QString, bool, bool)),this,SLOT(gui_updated(QString, bool, bool)));
    connect(this, SIGNAL(reflash_status(bool)), this, SLOT(reflash_finished(bool)));
    connect(this, SIGNAL(dump_status(QString)), this, SLOT(dump_finished(QString)));
    connect(this, SIGNAL(update_progress(int)),ui->progressBar,SLOT(setValue(int)));
    connect(timer,SIGNAL(timeout()),this,SLOT(TimerUpdate()));

    connect(advdlg,SIGNAL(Refresh()),this,SLOT(Refresh()));
    connect(advdlg,SIGNAL(WriteBootloader(Firmware::BootloaderType)),SLOT(WriteBootloader(Firmware::BootloaderType)));
    connect(advdlg,SIGNAL(WriteConfig(bool)),this,SLOT(WriteConfig(bool)));
    connect(advdlg,SIGNAL(WriteInfo(QString,QString)),this,SLOT(WriteInfo(QString,QString)));

    //set used properties
    this->setProperty("device_code", "");
    this->setProperty("serial_number", "");
    this->setProperty("device_type", 0);

    //initialise main ui
    ui->btnAdvanced->setEnabled(false);
    ui->btnDump->setEnabled(false);
    leds_off();
    reset_flag=false;
    DeviceChanged(true);
}


//Class destructor
MainWindow::~MainWindow()
{
    if(watcher.isRunning())
    {
        watcher.cancel();
        watcher.waitForFinished();
    }
    delete timer;
    delete usb_device;
    delete usbNotifier;
    delete advdlg;
    delete ui;
}


//Turn off all leds
void MainWindow::leds_off()
{
    QPalette pal;
    pal.setColor(QPalette::Background, QColor::fromRgb(0,64,0));
    ui->LedNorm->setPalette(pal);
    ui->LedBoot->setPalette(pal);
    pal.setColor(QPalette::Background, QColor::fromRgb(64,64,0));
    ui->LedErase->setPalette(pal);
    pal.setColor(QPalette::Background, QColor::fromRgb(64,0,0));
    ui->LedWrite->setPalette(pal);
    ui->LedNorm->setProperty("state",false);
    ui->LedBoot->setProperty("state",false);
    ui->LedErase->setProperty("state",false);
    ui->LedWrite->setProperty("state",false);
    ui->LedErase->setProperty("blink",false);
    ui->LedWrite->setProperty("blink",false);
}

/* LEDs on/off toggle routines */
void MainWindow::setNled(bool state)
{
    ui->LedNorm->setProperty("state", state);
    QPalette pal;
    pal.setColor(QPalette::Background,state ? QColor::fromRgb(0,255,0) :  QColor::fromRgb(0,64,0));
    ui->LedNorm->setPalette(pal);
}

void MainWindow::setBled(bool state)
{
    ui->LedBoot->setProperty("state", state);
    QPalette pal;
    pal.setColor(QPalette::Background,state ? QColor::fromRgb(0,255,0) :  QColor::fromRgb(0,64,0));
    ui->LedBoot->setPalette(pal);
}

void MainWindow::setEled(bool state)
{
    ui->LedErase->setProperty("state", state);
    QPalette pal;
    pal.setColor(QPalette::Background,state ? QColor::fromRgb(255,255,0) :  QColor::fromRgb(64,64,0));
    ui->LedErase->setPalette(pal);
}

void MainWindow::setWled(bool state)
{
    ui->LedWrite->setProperty("state", state);
    QPalette pal;
    pal.setColor(QPalette::Background,state ? QColor::fromRgb(255,0,0) :  QColor::fromRgb(64,0,0));
    ui->LedWrite->setPalette(pal);
}


//simple wait routine
void MainWindow::wait_ms(unsigned long time)
{
    QWaitCondition wc;
    QMutex mutex;
    QMutexLocker locker(&mutex);
    wc.wait(&mutex, time);
}


//Led blinking fired by timer event.
void MainWindow::TimerUpdate()
{
    if(ui->LedErase->property("blink").toBool())
        setEled(!ui->LedErase->property("state").toBool());
    if(ui->LedWrite->property("blink").toBool())
        setWled(!ui->LedWrite->property("state").toBool());
}


//browse for update.dat file
void MainWindow::on_btnInput_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Update.dat" ,NULL, "dat files (*.dat);;All files (*.*)");
    if(fileName.isEmpty())
        return;

    int ret=firmware.open(fileName);
    if(ret == Firmware::NoError)
    {
        ui->txtInput->setText(fileName);
        ui->lblVersion->setText(QString("[V:%1]").arg(firmware.GetFirmwareVersion()));
        return;
    }

    switch(ret)
    {
    case Firmware::OpenError:
        QMessageBox::warning(this,"TL866",QString("Cannot read file %1").arg(fileName));
        break;
    case Firmware::FilesizeError:
        QMessageBox::warning(this,"TL866",QString("%1\n\nFilesize error!").arg(fileName));
        break;
    case Firmware::CRCError:
        QMessageBox::warning(this,"TL866",QString("%1\n\nData CRC error!").arg(fileName));
        break;
    case Firmware::DecryptionError:
        QMessageBox::warning(this,"TL866","Firmware decryption error!");
        break;
    }
    ui->lblVersion->clear();

}


//show advanced dialog
void MainWindow::on_btnAdvanced_clicked()
{
    Refresh();
    advdlg->show();
}


//show edit device code and serial number dialog
void MainWindow::on_btnEdit_clicked()
{
    EditDialog dlg(this);
    QString devcode =ui->txtDevcode->text();
    QString serial =ui->txtSerial->text();
    dlg.SetText(devcode,serial);
    if(dlg.exec()==QDialog::Accepted)
    {
        dlg.GetResult(&devcode, &serial);
        ui->txtDevcode->setText(devcode);
        ui->txtSerial->setText(serial);
    }
}


//set default device code and serial number
void MainWindow::on_btnDefault_clicked()
{
    ui->txtDevcode->setText("00000000");
    ui->txtSerial->setText("000000000000000000000000");
}


//Read device info
void MainWindow::Refresh()
{
    reset_flag=false;
    DeviceChanged(true);
}


//clone serial and device code from connected device
void MainWindow::on_btnClone_clicked()
{
    Refresh();
    ui->txtDevcode->setText(this->property("device_code").toString());
    ui->txtSerial->setText(this->property("serial_number").toString());
}


//reset device button
void MainWindow::on_btnReset_clicked()
{

    if(watcher.isRunning())
        return;
    if(!CheckDevices(this))
        return;

    if(usb_device->open_device(0))
    {
        reset();
        usb_device->close_device();
    }
}


//reflash device button
void MainWindow::on_btnReflash_clicked()
{
    if(watcher.isRunning())
        return;

    if(!CheckDevices(this))
        return;

    if(!firmware.isValid())
    {
        QMessageBox::warning(this, "TL866", "No firmware file loaded!\nPlease load the update.dat file.");
        return;
    }

    if(QMessageBox::warning(this, "TL866", "Warning! this operation will reflash the device.\nDo you want to continue?",
                            QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
        return;

    int index = -1;
    if(ui->radioA->isChecked())
        index = Firmware::FIRMWARE_A;
    if(ui->radioCS->isChecked())
        index = Firmware::FIRMWARE_CS;
    if(ui->radioDump->isChecked())
        index = Firmware::FIRMWARE_CUSTOM;
    if(index == -1)
        return;
    job_list.clear();
    job_list.append(REFLASH);
    watcher.setProperty("firmware_version", index);
    ui->progressBar->setMaximum(ENCRYPTED_FIRMWARE_SIZE/BLOCK_SIZE-1);
    watcher.setFuture(QtConcurrent::map(job_list, WorkerWrapper(this)));
}


//dump device button
void MainWindow::on_btnDump_clicked()
{
    if(!watcher.isRunning())
    {
        if(!firmware.isValid())
        {
            QMessageBox::warning(this, "TL866", "No firmware file loaded!\nPlease load the update.dat file.");
            return;
        }

        QString fileName=QFileDialog::getSaveFileName(this,"Save firmware hex file",NULL,"hex files (*.hex);;All files (*.*)");
        if(!fileName.isEmpty())
        {
            job_list.clear();
            job_list.append(DUMP);
            watcher.setProperty("hex_path", fileName);
            ui->progressBar->setMaximum(FLASH_SIZE - 1);
            watcher.setFuture(QtConcurrent::map(job_list, WorkerWrapper(this)));
        }
    }
}


//save hex button
void MainWindow::on_btnSave_clicked()
{
    QString fileName=QFileDialog::getSaveFileName(this,"Save firmware hex file",NULL,"hex files (*.hex);;All files (*.*)");
    if(!fileName.isEmpty())
    {
        if(!fileName.endsWith(".hex", Qt::CaseInsensitive))
            fileName += ".hex";
        QFile file(fileName);
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QMessageBox::critical(this,"TL866",QString("Error creating file %1\n%2.").arg(fileName).arg(file.errorString()));
            return;
        }
        QTextStream fileStream(&file);

        QByteArray b =get_resource(ui->radiofA->isChecked() ? A_FIRMWARE_RESOURCE :  CS_FIRMWARE_RESOURCE, FLASH_SIZE);

        uchar *temp = new uchar[FLASH_SIZE];//128K byte array
        memcpy(temp,b.data(),FLASH_SIZE);//copy entire firmware to array

        if(ui->optionBoot->isChecked())
            memset(temp+BOOTLOADER_SIZE, 0xFF, UNENCRYPTED_FIRMWARE_SIZE);//if the option bootloader only is selected then clear the main firmware area(0x1800-0x1FBFF)

        uchar *key = new uchar[BLOCK_SIZE];//for holding serial and dev code
        firmware.decrypt_serial(key, temp);//decrypt the serial key from temp array to key array
        memset(key,' ',32);//add trailing spaces

        memcpy(key, ui->txtDevcode->text().toLatin1().data(), ui->txtDevcode->text().size());//copy devcode to key array
        memcpy(key+8, ui->txtSerial->text().toLatin1().data(), ui->txtSerial->text().size());//copy serial to key array

        firmware.encrypt_serial(key, temp);//encrypt the devcode and serial
        memcpy(temp + SERIAL_OFFSET ,key,BLOCK_SIZE);//copy the new devcode and serial to temp array

        HexWriter *hexwriter = new HexWriter;
        hexwriter->WriteHex(fileStream,temp,FLASH_SIZE);//write temp array to fileStream in Intel hex format
        delete hexwriter;
        delete[] key;
        delete[] temp;
        file.close();//done!
    }
}

//Helper function to get a binary resource
QByteArray MainWindow::get_resource(QString resource_path, int size)
{
    QResource res(resource_path);
    QByteArray ba;
    ba = (res.isCompressed() ? qUncompress(res.data(), size) : QByteArray((const char*)res.data(), size));
    return ba;
}


//Background worker dispatch routine. Notice that this function is executed in a separate thread.
void MainWindow::DoWork(WorkerJob job)
{
    switch(job)
    {
    case REFLASH:
        if(usb_device->open_device(0))
        {
            bool success =reflash();
            usb_device->close_device();
            emit reflash_status(success);
        }
        break;

    case DUMP:
        if(usb_device->open_device(0))
        {
            QString result = dump();
            usb_device->close_device();
            emit dump_status(result);
        }
        break;
    }
}


//Send the Reset command to the TL866.
void MainWindow::reset()
{

    uchar data[4] = {RESET_COMMAND, 0, 0, 0};
    reset_flag=true;
    usb_device->usb_write(data, 4);
}


//wait for device to reset
bool MainWindow::wait_for_device()
{
    int cnt = 50;//5 seconds
    while(usb_device->get_devices_count())//wait for device to leave
    {
        wait_ms(100);
        if(! --cnt)
            return false;//reset error
    }

    cnt = 50;//5 seconds
    while(! usb_device->get_devices_count())//wait for device to arrive
    {
        wait_ms(100);
        if(! --cnt)
            return false;//reset error
    }
    return true;//device reset ok
}


//Reflash function. This routine is executed in a separate thread.
bool MainWindow::reflash()
{
    uchar buffer[BLOCK_SIZE+7];
    uchar data[ENCRYPTED_FIRMWARE_SIZE];

   Firmware::TL866_REPORT report;

    //read the device to determine his satus
    memset((uchar*)&report,0, sizeof(Firmware::TL866_REPORT));
    report.echo = REPORT_COMMAND;//0 anyway
    usb_device->usb_write((uchar *)&report, 5);
    usb_device->usb_read((uchar*)&report, sizeof(Firmware::TL866_REPORT));
    if(report.device_status == Firmware::NORMAL_MODE)//if the device is not in bootloader mode reset it.
    {
        reset();
        emit update_gui(QString("<resetting...>"), false, false);
        if(!wait_for_device())
            return false;//reset failed
    }
    wait_ms(500);

    //read the device again to see the true device version as reported by the bootloader
    memset((uchar*)&report,0, sizeof(Firmware::TL866_REPORT));
    report.echo = REPORT_COMMAND;//0 anyway
    usb_device->usb_write((uchar *)&report, 5);
    usb_device->usb_read((uchar*)&report, sizeof(Firmware::TL866_REPORT));
    int device_version = report.device_version;


    //Erase device first
    memset(buffer,0,sizeof(buffer));
    buffer[0]=ERASE_COMMAND;
    buffer[7]=firmware.GetEraseParammeter(device_version);
    emit update_gui(QString("<erasing...>"), true, false);
    usb_device->usb_write(buffer, 20);
    usb_device->usb_read(data, 32);
    if(data[0] != ERASE_COMMAND)
        return false;//erase failed

    //Write device.
    emit update_gui(QString("<erasing...>"), false, false);
    wait_ms(500);
    emit update_gui(QString("<writing...>"), false, true);


    //Get the encrypted firmware.
    switch(watcher.property("firmware_version").toInt())
    {
    case Firmware::FIRMWARE_A:
    default:
        firmware.get_firmware(data, device_version, Firmware::A_KEY);
        break;
    case Firmware::FIRMWARE_CS:
        firmware.get_firmware(data, device_version, Firmware::CS_KEY);
        break;
    case Firmware::FIRMWARE_CUSTOM:
        QByteArray b = get_resource(DUMPER_RESOURCE, UNENCRYPTED_FIRMWARE_SIZE);
        firmware.encrypt_firmware((const uchar*)b.data(), data, device_version);
    }


    /*      prepare data by adding 7 bytes header on each 80 bytes data block and send it over the usb.
     *
     *      7 bytes header+80 bytes data like this: | Command | lenght | address |  data  |
     *                                                2bytes    2bytes   3bytes    80bytes
     *
     */

    quint32 address = BOOTLOADER_SIZE;
    for (int i = 0; i<ENCRYPTED_FIRMWARE_SIZE; i += BLOCK_SIZE)
    {
        buffer[0] = WRITE_COMMAND;//command LSB
        buffer[1] = 0x00;//command MSB
        buffer[2] = BLOCK_SIZE;//Block size without header(LSB)
        buffer[3] = 0x00;//Block size MSB
        buffer[4] = address & 0xff;//24 bit address which will be written (3 bytes in little endian order)
        buffer[5] = (address & 0xff00)>>8;
        buffer[6] = (address & 0xff0000)>>16;
        memcpy(&buffer[7], &data[i], BLOCK_SIZE);

        if (usb_device->usb_write(buffer, sizeof(buffer)) != sizeof(buffer))
            return false;//write failed
        address+=64;//next data block
        emit update_progress(i/BLOCK_SIZE);
    }

    //Reset the device back in normal working mode
    emit update_gui(QString("<writing...>"), false, false);
    wait_ms(500);
    emit update_gui(QString("<resetting...>"), false, false);
    reset();
    if (! wait_for_device())
        return false;//reset failed

    //read the device to determine his satus
    memset((uchar*)&report,0, sizeof(Firmware::TL866_REPORT));
    report.echo = REPORT_COMMAND;//0 anyway
    usb_device->usb_write((uchar *)&report, 5);
    usb_device->usb_read((uchar*)&report, sizeof(Firmware::TL866_REPORT));

    if(report.device_status != Firmware::NORMAL_MODE)//reflash failed
        return false;

    return true;//reflash ok

}

//Dump function. This function is executed in separate thread.
QString MainWindow::dump()
{
    uchar temp[FLASH_SIZE];//128Kbyte buffer
    uchar w[5];
    QFile file(watcher.property("hex_path").toString());

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return file.errorString();

    QTextStream fileStream(&file);

    for(int i = 0; i < FLASH_SIZE; i += 64)
    {
        w[0] = DUMPER_READ_FLASH;
        w[1] = 64;//packet size
        w[2] = i & 0xff;//24bit address in little endian order
        w[3] = (i & 0xff00)>>8;
        w[4] = (i & 0xff0000)>>16;

        usb_device->usb_write(w, sizeof(w));
        if(usb_device->usb_read(&temp[i],64) != 64)
            return QString("USB read error.");
        emit update_progress(i);
    }

    firmware.decrypt_firmware(&temp[BOOTLOADER_SIZE], this->property("device_type").toInt());

    HexWriter *hexwriter = new HexWriter;
    hexwriter->WriteHex(fileStream,temp,sizeof(temp));//write temp array to fileStream in Intel hex format.
    delete hexwriter;
    file.close();
    return QString("");
}



//Reflash finished SLOT
void MainWindow::reflash_finished(bool success)
{
    Refresh();
    if(success)
        QMessageBox::information(this, "TL866", "          Reflash OK!          ");
    else
        QMessageBox::critical(this, "TL866", "     Reflash Failed!     ");
    emit update_progress(0);
}


//Dump finished SLOT
void MainWindow::dump_finished(QString result)
{
    Refresh();
    if(result.isEmpty())
        QMessageBox::information(this, "TL866", "Firmware dump complete!");
    else
        QMessageBox::critical(this, "TL866", QString("Error creating dump.hex file\n%1.").arg(result));
    emit update_progress(0);
}


//Gui update SLOT
void MainWindow::gui_updated(QString message, bool eraseLed, bool writeLed)
{
    ui->LedErase->setProperty("blink", eraseLed);
    ui->LedWrite->setProperty("blink", writeLed);
    if(eraseLed || writeLed)
        timer->start(300);
    else
        timer->stop();
    setEled(eraseLed);
    setWled(writeLed);

    QStringList list(ui->txtInfo->toPlainText().split("\n"));
    list.removeAt(1);
    list.insert(1,"Device status: Bootloader mode " + message);
    ui->txtInfo->clear();
    ui->txtInfo->append(list.join("\n"));
}

//This procedure is called automatically by the usb device change. Call this function to refresh the info.
void MainWindow::DeviceChanged(bool arrived)
{
    if(!arrived && reset_flag)//ignore unplug event if the device was resetted by us.
        return;

    reset_flag=false;
    ui->txtInfo->clear();
    int devtype = 0;
    int count=usb_device->get_devices_count();
    this->setWindowTitle(QString("TL866 firmware updater (%1 %2 connected)").arg(count).arg(count == 1 ? "device" : "devices"));

    if(count)
    {
        Firmware::TL866_REPORT report;
        if(usb_device->open_device(0))
        {
            memset((uchar*)&report,0, sizeof(Firmware::TL866_REPORT));
            report.echo = REPORT_COMMAND;//0 anyway
            usb_device->usb_write((uchar *)&report, 5);
            usb_device->usb_read((uchar*)&report, sizeof(Firmware::TL866_REPORT));

            switch(report.device_version)
            {
            case Firmware::VERSION_TL866A:
                ui->txtInfo->append("Device version: TL866A");
                devtype = Firmware::VERSION_TL866A;
                break;
            case Firmware::VERSION_TL866CS:
                ui->txtInfo->append("Device version: TL866CS");
                devtype = Firmware::VERSION_TL866CS;
                break;
            default:
                ui->txtInfo->append("Device version: Unknown");
                devtype = 0;
            }


            switch(report.device_status)
            {
            case Firmware::NORMAL_MODE:
                setNled(true);
                setBled(false);
                ui->txtInfo->append("Device status: Normal working mode.");
                break;
            case Firmware::BOOTLOADER_MODE:
                setNled(false);
                setBled(true);
                ui->txtInfo->append("Device status: Bootloader mode <waiting for update.>");
                break;
            default:
                setNled(false);
                setBled(false);
                ui->txtInfo->append("Device status: Unknown.");
            }
            QString s_devcode = (QString::fromLatin1((const char*)&report.device_code,8));
            QString s_serial = (QString::fromLatin1((const char*)&report.serial_number,24));
            bool isDumperActive = (s_devcode.toLower() == "codedump" && s_serial == "000000000000000000000000");


            if(isDumperActive)
            {

                Firmware::DUMPER_REPORT dumper_report;
                uchar b[] = {DUMPER_INFO};
                usb_device->usb_write(b, 1);
                usb_device->usb_read((uchar*)&dumper_report, sizeof(Firmware::DUMPER_REPORT));
                devtype = dumper_report.bootloader_version;

                s_devcode = (QString::fromLatin1((const char*)&dumper_report.device_code,8));
                s_serial = (QString::fromLatin1((const char*)&dumper_report.serial_number,24));

                advdlg->SetSerial(s_devcode, s_serial);

                QString info;
                info.append(QString("Device code: %1\n").arg(s_devcode.trimmed()   + (Firmware::IsBadCrc((uchar*)s_devcode.toLatin1().data(), (uchar*)s_serial.toLatin1().data()) ? " (Bad device code)" : "")));
                info.append(QString("Serial number: %1\n").arg(s_serial.trimmed()  + (Firmware::IsBadCrc((uchar*)s_devcode.toLatin1().data(), (uchar*)s_serial.toLatin1().data()) ? " (Bad serial code)" : "")));
                info.append(QString("Bootloader version: %1\n").arg((devtype == Firmware::VERSION_TL866A) ? "A" : "CS"));
                info.append(QString("Code Protection bit: %1\n").arg(dumper_report.cp_bit ? "No" : "Yes"));

                advdlg->SetInfo(info);
                advdlg->SetUi(dumper_report.cp_bit == 0, devtype);

                ui->btnAdvanced->setEnabled(true);
                ui->btnDump->setEnabled(true);
            }
            else//dumper is not active
            {
                ui->btnAdvanced->setEnabled(false);
                ui->btnDump->setEnabled(false);
                advdlg->SetSerial("", "");
                advdlg->SetInfo("");
                advdlg->hide();
            }

            ui->txtInfo->append("Device code: " + s_devcode.trimmed() + (Firmware::IsBadCrc((uchar*)s_devcode.toLatin1().data(), (uchar*)s_serial.toLatin1().data()) ? " (Bad device code)" : ""));
            ui->txtInfo->append("Serial number: " + s_serial.trimmed() + (Firmware::IsBadCrc((uchar*)s_devcode.toLatin1().data(), (uchar*)s_serial.toLatin1().data()) ? " (Bad serial code)" : ""));
            this->setProperty("device_code", s_devcode);
            this->setProperty("serial_number", s_serial);
            ui->txtInfo->append(isDumperActive ? "Firmware version: Firmware dumper" :
                                                 report.device_status == Firmware::NORMAL_MODE ? QString("Firmware version: %1.%2.%3")
                                                                                                .arg(report.hardware_version)
                                                                                                .arg(report.firmware_version_major)
                                                                                                .arg(report.firmware_version_minor):
                                                                                                "Firmware version: Bootloader");

            if(!watcher.isRunning())
                usb_device->close_device();//do not close device if an upgrade is in progress.
        }
        else//error oppening device
            SetBlank();

    }
    else//no device connected
        SetBlank();

    this->setProperty("device_type", devtype);//save global property for later usage.
}


//Helper function
void MainWindow::SetBlank()
{
    leds_off();
    ui->btnAdvanced->setEnabled(false);
    ui->btnDump->setEnabled(false);
    //ui->txtDevcode->setText("");
    //ui->txtSerial->setText("");
    this->setProperty("device_code", "");
    this->setProperty("serial_number", "");
    advdlg->SetSerial("", "");
    advdlg->SetInfo("");
    advdlg->hide();
}

//Helper function
bool MainWindow::CheckDevices(QWidget *parent)
{
    if (usb_device->get_devices_count() == 0)
    {
        QMessageBox::warning(parent, "TL866", "No device detected!\nPlease connect one and try again.");
        return false;
    }
    if (usb_device->get_devices_count() > 1)
    {
        QMessageBox::warning(parent, "TL866", "Multiple devices detected!\nPlease connect only one device.");
        return false;
    }
    return true;
}




/*Advanced functions*/


//Write bootloader
void MainWindow::WriteBootloader(Firmware::BootloaderType type)
{
    if(!CheckDevices(advdlg))
        return;
    if(!AdvQuestion())
        return;
    if(usb_device->open_device(0))
    {
        uint crc = BootloaderCRC();
        if(!((crc == A_BOOTLOADER_CRC) || (crc == CS_BOOTLOADER_CRC)))
        {
            usb_device->close_device();
            QMessageBox::warning(advdlg, "TL866",
                                 "The bootloader CRC of your device version doesn't match!\nAs a safety measure, nothing will be written.");
            return;
        }
        uchar b[2]={DUMPER_WRITE_BOOTLOADER, (uchar) (type ==  Firmware::A_BOOTLOADER ? Firmware::VERSION_TL866A : Firmware::VERSION_TL866CS)};
        usb_device->usb_write(b, 2);
        b[0] = 0;
        usb_device->usb_read(b, 1);
        usb_device->close_device();
        Refresh();
        if(b[0] == DUMPER_WRITE_BOOTLOADER)
            QMessageBox::information(advdlg, "TL866", "Bootloader was successfully written.");
        else
            QMessageBox::critical(advdlg, "TL866", "Bootloader writing failed.");
    }
}


//write copy protect bit
void MainWindow::WriteConfig(bool copy_protect)
{
    if(!CheckDevices(advdlg))
        return;
    if(!AdvQuestion())
        return;
    if(usb_device->open_device(0))
    {
        uchar b[2]={DUMPER_WRITE_CONFIG, (uchar)(copy_protect ? 1 : 0)};
        usb_device->usb_write(b, 2);
        b[0] = 0;
        usb_device->usb_read(b, 1);
        usb_device->close_device();
        Refresh();
        if(b[0] == DUMPER_WRITE_CONFIG)
            QMessageBox::information(advdlg, "TL866", "Code protection bit was successfully written.");
        else
            QMessageBox::critical(advdlg, "TL866", "Writing code protect bit failed.");
    }

}

//write serial number and device code
void MainWindow::WriteInfo(QString device_code, QString serial_number)
{
    if(!CheckDevices(advdlg))
        return;
    if(!AdvQuestion())
        return;
    if(usb_device->open_device(0))
    {
        uchar b[34];
        memset(b,' ',34);//add trailing spaces
        b[0] = DUMPER_WRITE_INFO;
        memcpy(b+1, device_code.toLatin1().data(), device_code.size());//copy devcode to b array
        memcpy(b+9, serial_number.toLatin1().data(), serial_number.size());//copy serial to key array
        usb_device->usb_write(b, 34);
        b[0] = 0;
        usb_device->usb_read(b, 1);
        usb_device->close_device();
        Refresh();
        if(b[0] == DUMPER_WRITE_INFO)
            QMessageBox::information(advdlg, "TL866", "Device info was successfully written.");
        else
            QMessageBox::critical(advdlg, "TL866", "Writing device info failed.");
    }

}


//read bootloader and compute crc16
uint MainWindow::BootloaderCRC()
{
    uchar buffer[BOOTLOADER_SIZE];//6Kbyte
    uchar w[5];

    for(int i=0;i<BOOTLOADER_SIZE;i+=64)
    {
        w[0]=DUMPER_READ_FLASH;
        w[1]=64;//packet size
        w[2]=i & 0xff;//24bit address in little endian order
        w[3]=(i & 0xff00)>>8;
        w[4]=(i & 0xff0000)>>16;
        usb_device->usb_write(w, sizeof(w));
        if(usb_device->usb_read(&buffer[i],64) != 64)
        {
            QMessageBox::warning(advdlg, "TL866", "    USB read error!    ");
            return 0;
        }
    }
    CRC crc;
    return ~crc.crc32(buffer, sizeof(buffer), 0xFFFFFFFF);
}


//helper function
bool MainWindow::AdvQuestion()
{
    return(QMessageBox::warning(advdlg, "TL866", "Warning! this operation may brick your device.\nDo you want to continue?",
                                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes);
}
