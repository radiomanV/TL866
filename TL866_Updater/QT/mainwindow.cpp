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

#include <QUrl>
#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QResource>
#include <QDragEnterEvent>
#include <QMimeData>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "editdialog.h"
#include "hexwriter.h"
#include "crc.h"



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
    firmware = new Firmware();
    worker = new QFuture<void>;
    timer = new QTimer(this);

    //initialise used signals
    connect(usbNotifier,SIGNAL(deviceChange(bool)),this,SLOT(DeviceChanged(bool)));
    connect(this, SIGNAL(update_gui(QString, bool, bool)),this,SLOT(gui_updated(QString, bool, bool)));
    connect(this, SIGNAL(reflash_status(QString)), this, SLOT(reflash_finished(QString)));
    connect(this, SIGNAL(dump_status(QString)), this, SLOT(dump_finished(QString)));
    connect(this, SIGNAL(update_progress(int)),ui->progressBar,SLOT(setValue(int)));
    connect(timer,SIGNAL(timeout()),this,SLOT(TimerUpdate()));

    connect(advdlg,SIGNAL(DeviceChange(bool)),this,SLOT(DeviceChanged(bool)));
    connect(advdlg,SIGNAL(WriteBootloader(Firmware::BootloaderType)),SLOT(WriteBootloader(Firmware::BootloaderType)));
    connect(advdlg,SIGNAL(WriteConfig(bool)),this,SLOT(WriteConfig(bool)));
    connect(advdlg,SIGNAL(WriteInfo(QString,QString)),this,SLOT(WriteInfo(QString,QString)));

    ui->btnReset->setProperty("state", false);

    //initialise main ui
    ui->btnAdvanced->setEnabled(false);
    ui->btnDump->setEnabled(false);
    leds_off();
    reset_flag=false;
    DeviceChanged(true);
    setAcceptDrops(true);
}

//Class destructor
MainWindow::~MainWindow()
{
    if(worker->isRunning())
        worker->cancel();
    worker->waitForFinished();
    delete worker;
    delete firmware;
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
    QString fileName = QFileDialog::getOpenFileName(this, "Update.dat" ,nullptr, "dat files (*.dat);;All files (*.*)");
    if(fileName.isEmpty())
        return;
    OpenFile(fileName);
}

//Open the file update.dat
void MainWindow::OpenFile(const QString &fileName)
{
    int ret=firmware->open(fileName);
    if(ret == Firmware::NoError)
    {
        ui->txtInput->setText(fileName);
        ui->lblVersion->setText(QString("[V:%1]").arg(firmware->GetFirmwareVersion()));
        QApplication::beep();
        return;
    }

    switch(ret)
    {
    case Firmware::OpenError:
        QMessageBox::warning(this,"TL866",QString("Cannot read file %1").arg(fileName));
        break;
    case Firmware::FilesizeError:
        QMessageBox::warning(this,"TL866",QString("%1\n\nFile size error!").arg(fileName));
        break;
    case Firmware::CRCError:
        QMessageBox::warning(this,"TL866",QString("%1\n\nData CRC error!").arg(fileName));
        break;
    case Firmware::DecryptionError:
        QMessageBox::warning(this,"TL866","Firmware decryption error!");
        break;
    }
    ui->lblVersion->clear();
    ui->txtInput->clear();
}

//Drag and drop events
void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    if(event->mimeData()->urls().size() == 1 && event->mimeData()->urls()[0].isLocalFile())
    {
        OpenFile(event->mimeData()->urls()[0].toLocalFile());
    }
}

//show advanced dialog
void MainWindow::on_btnAdvanced_clicked()
{
    DeviceChanged(true);
    advdlg->show();
}


//show edit device code and serial number dialog
void MainWindow::on_btnEdit_clicked()
{
    QString devcode =ui->txtDevcode->text();
    QString serial =ui->txtSerial->text();
    EditDialog dlg(this, devcode, serial);
    if(dlg.exec()==QDialog::Accepted)
    {
        dlg.GetResult(devcode, serial);
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


//clone serial and device code from connected device
void MainWindow::on_btnClone_clicked()
{
    DeviceInfo info = DeviceChanged(true);
    ui->txtDevcode->setText(info.device_code);
    ui->txtSerial->setText(info.device_serial);
}


//reset device button
void MainWindow::on_btnReset_clicked()
{
    if(ui->btnReset->property("state").toBool())
        return;
    if(worker->isRunning())
        return;

    if(!CheckDevices(this))
        return;
    *worker = QtConcurrent::run(this, &MainWindow::reset);
    ui->btnReset->setProperty("state", true);
}


//reflash device button
void MainWindow::on_btnReflash_clicked()
{
    if(worker->isRunning())
        return;
    if(!CheckDevices(this))
        return;

    if(!firmware->isValid())
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
    ui->progressBar->setMaximum(ENCRYPTED_FIRMWARE_SIZE/BLOCK_SIZE-1);
    *worker = QtConcurrent::run(this, &MainWindow::reflash, index);
}


//dump device button
void MainWindow::on_btnDump_clicked()
{
    if(worker->isRunning())
        return;
    if(!CheckDevices(this))
        return;

    if(!firmware->isValid())
    {
        QMessageBox::warning(this, "TL866", "No firmware file loaded!\nPlease load the update.dat file.");
        return;
    }

    QString ext;
    QString fileName = QFileDialog::getSaveFileName(this,"Save firmware hex file",nullptr,"hex files (*.hex);;All files (*.*)",&ext);
    if(!fileName.isEmpty())
    {
        if(ext.contains("hex") && !fileName.endsWith(".hex", Qt::CaseInsensitive))
            fileName += ".hex";
        ui->progressBar->setMaximum(FLASH_SIZE - 1);
        DeviceInfo info = DeviceChanged(true);
        *worker = QtConcurrent::run(this, &MainWindow::dump, fileName, info.device_type);
    }
}


//save hex button
void MainWindow::on_btnSave_clicked()
{
    //Prepare data to be saved.
    QByteArray temp = get_resource(ui->radiofA->isChecked() ? A_FIRMWARE_RESOURCE :  CS_FIRMWARE_RESOURCE, FLASH_SIZE);

    //Writing the main firmware or empty on choice
    if(ui->optionBoot->isChecked())
        memset(&temp.data()[BOOTLOADER_SIZE], 0xFF, UNENCRYPTED_FIRMWARE_SIZE);//if the option bootloader only is selected then clear the main firmware area(0x1800-0x1FBFF)

    //Disable the CP0 bit if necessary
    if(!ui->cp0->isChecked())
        temp.data()[CP0_ADDRESS] |= 0x04;

    //Writing serial code
    memset(&temp.data()[SERIAL_OFFSET],' ',32);//add trailing spaces
    memcpy(&temp.data()[SERIAL_OFFSET], ui->txtDevcode->text().toLatin1().data(), static_cast<size_t>(ui->txtDevcode->text().size()));//copy devcode to key array
    memcpy(&temp.data()[SERIAL_OFFSET+8], ui->txtSerial->text().toLatin1().data(), static_cast<size_t>(ui->txtSerial->text().size()));//copy serial to key array
    firmware->encrypt_serial(reinterpret_cast<uchar*>(&temp.data()[SERIAL_OFFSET]), reinterpret_cast<const uchar*>(temp.data()));//encrypt the devcode and serial


    //Open the file save dialog
    QString ext;
    QString fileName = QFileDialog::getSaveFileName(this,"Save firmware hex file",nullptr,"hex files (*.hex);;All files (*.*)",&ext);
    if(!fileName.isEmpty())
    {
        if(ext.contains("hex") && !fileName.endsWith(".hex", Qt::CaseInsensitive))
            fileName += ".hex";
        QFile file(fileName);
        if(!file.open(ext.contains("hex") ? (QIODevice::WriteOnly | QIODevice::Text) : QIODevice::WriteOnly))
        {
            QMessageBox::critical(this,"TL866",QString("Error creating file %1\n%2.").arg(fileName).arg(file.errorString()));
            return;
        }

        if(fileName.endsWith(".hex", Qt::CaseInsensitive))
        {
            //write temp array to fileStream in Intel hex format
            HexWriter hexwriter(&file);
            hexwriter.WriteHex(temp);
        }
        else
        {
            //write temp array to fileStream in binary format
            file.write(temp);
        }
        file.close();//done!
        QApplication::beep();
    }
}

//Click the CP0 checkbox
void MainWindow::on_cp0_stateChanged(int arg1)
{
    if(arg1 == Qt::Unchecked)
        QMessageBox::warning(this, "TL866", "Disabling the CP0 bit will disable switch to bootloader function in the latest firmware versions!\nThis bit should be disabled only for debugging purposes.",
                             QMessageBox::Ok);
}

//Helper function to get a binary resource
QByteArray MainWindow::get_resource(const QString &resource_path, int size)
{
    QResource res(resource_path);
    QByteArray ba;
    ba = (res.isCompressed() ? qUncompress(res.data(), size) : QByteArray(reinterpret_cast<const char*>(res.data()), size));
    return ba;
}


//Send the Reset command to the TL866.
void MainWindow::reset()
{
    usb_device->close_device();
    if(usb_device->open_device(0))
    {
        uchar data[4] = {RESET_COMMAND, 0, 0, 0};
        reset_flag=true;
        usb_device->usb_write(data, 4);
        usb_device->close_device();
    }
}


//wait for device to reset
bool MainWindow::wait_for_device()
{
    int cnt = 100;//10 seconds
    while(usb_device->get_devices_count())//wait for device to leave
    {
        wait_ms(100);
        if(! --cnt)
            return false;//reset error
    }

    cnt = 100;//10 seconds
    while(! usb_device->get_devices_count())//wait for device to arrive
    {
        wait_ms(100);
        if(! --cnt)
            return false;//reset error
    }
    return true;//device reset ok
}


//Reflash function. This routine is executed in a separate thread.
void MainWindow::reflash(uint firmware_type)
{
    uchar buffer[BLOCK_SIZE+7];
    uchar data[ENCRYPTED_FIRMWARE_SIZE];

    Firmware::tl866_report report;

    usb_device->close_device();
    if(!usb_device->open_device(0))
    {
        emit reflash_status("   USB device error!    ");
        return;
    }

    //read the device to determine his satus
    memset(reinterpret_cast<uchar*>(&report),0, sizeof(report));
    report.echo = REPORT_COMMAND;//0 anyway
    usb_device->usb_write(reinterpret_cast<uchar*>(&report), 5);
    usb_device->usb_read(reinterpret_cast<uchar*>(&report), sizeof(report));
    if(report.device_status == Firmware::NORMAL_MODE)//if the device is not in bootloader mode reset it.
    {
        reset();
        emit update_gui(QString("<resetting...>"), false, false);
        if(!wait_for_device())
        {
            usb_device->close_device();
            emit reflash_status("   Reset failed!   ");
            return;//reset failed
        }
        if(!usb_device->open_device(0))
        {
            emit reflash_status("   USB device error!    ");
            return;
        }
    }

    wait_ms(500);
    //read the device again to see the true device version as reported by the bootloader
    memset(reinterpret_cast<uchar*>(&report),0, sizeof(report));
    report.echo = REPORT_COMMAND;//0 anyway
    usb_device->usb_write(reinterpret_cast<uchar*>(&report), 5);
    usb_device->usb_read(reinterpret_cast<uchar*>(&report), sizeof(report));
    int device_version = report.device_version;


    //Erase device first
    memset(buffer,0,sizeof(buffer));
    buffer[0]=ERASE_COMMAND;
    buffer[7]=firmware->GetEraseParammeter(device_version);
    emit update_gui(QString("<erasing...>"), true, false);
    usb_device->usb_write(buffer, 20);
    wait_ms(100);
    usb_device->usb_read(data, 32);
    if(data[0] != ERASE_COMMAND)
    {
        usb_device->close_device();
        emit reflash_status("   Erase failed!   ");
        return;//erase failed
    }

    //Write device.
    emit update_gui(QString("<erasing...>"), false, false);
    wait_ms(500);
    emit update_gui(QString("<writing...>"), false, true);


    //Get the encrypted firmware.
    switch(firmware_type)
    {
    case Firmware::FIRMWARE_A:
    default:
        firmware->get_firmware(data, device_version, Firmware::A_KEY);
        break;
    case Firmware::FIRMWARE_CS:
        firmware->get_firmware(data, device_version, Firmware::CS_KEY);
        break;
    case Firmware::FIRMWARE_CUSTOM:
        QByteArray b = get_resource(DUMPER_RESOURCE, UNENCRYPTED_FIRMWARE_SIZE);
        firmware->encrypt_firmware(reinterpret_cast<uchar*>(b.data()), data, device_version);
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
        if(worker->isCanceled())
            return;
        buffer[0] = WRITE_COMMAND;//command LSB
        buffer[1] = 0x00;//command MSB
        buffer[2] = BLOCK_SIZE;//Block size without header(LSB)
        buffer[3] = 0x00;//Block size MSB
        buffer[4] = address & 0xff;//24 bit address which will be written (3 bytes in little endian order)
        buffer[5] = (address & 0xff00)>>8;
        buffer[6] = (address & 0xff0000)>>16;
        memcpy(&buffer[7], &data[i], BLOCK_SIZE);

        if (usb_device->usb_write(buffer, sizeof(buffer)) != sizeof(buffer))
        {
            usb_device->close_device();
            emit reflash_status("   Reflash failed!   ");
            return;//write failed
        }
        address+=64;//next data block
        emit update_progress(i/BLOCK_SIZE);
    }

    //Reset the device back in normal working mode
    emit update_gui(QString("<writing...>"), false, false);
    wait_ms(500);
    emit update_gui(QString("<resetting...>"), false, false);
    reset();
    if (! wait_for_device())
    {
        usb_device->close_device();
        emit reflash_status("   Reset failed!   ");
        return;//reset failed
    }

    if(!usb_device->open_device(0))
    {
        emit reflash_status("   USB device error!    ");
        return;

    }

    //read the device to determine his satus
    memset(reinterpret_cast<uchar*>(&report),0, sizeof(report));
    report.echo = REPORT_COMMAND;//0 anyway
    usb_device->usb_write(reinterpret_cast<uchar*>(&report), 5);
    usb_device->usb_read(reinterpret_cast<uchar*>(&report), sizeof(report));

    if(report.device_status != Firmware::NORMAL_MODE)
    {
        usb_device->close_device();
        emit reflash_status("   Reflash failed!   ");
        return;//reflash failed
    }

    usb_device->close_device();
    emit reflash_status("");
    return;//reflash ok

}

//Dump function. This function is executed in separate thread.
void MainWindow::dump(const QString &fileName, uint device_type)
{
    //Try to open the device
    usb_device->close_device();
    if(!usb_device->open_device(0))
    {
        emit dump_status("USB device error!");
        return;
    }

    //Read data from the device.
    QByteArray temp(FLASH_SIZE, 0);
    uchar w[5];
    for(int i = 0; i < FLASH_SIZE; i += 64)
    {
        if(worker->isCanceled())
            return;
        w[0] = DUMPER_READ_FLASH;
        w[1] = 64;//packet size
        w[2] = i & 0xff;//24bit address in little endian order
        w[3] = (i & 0xff00)>>8;
        w[4] = (i & 0xff0000)>>16;

        usb_device->usb_write(w, sizeof(w));
        if(usb_device->usb_read(reinterpret_cast<uchar*>(&temp.data()[i]),64) != 64)
        {
            usb_device->close_device();
            emit dump_status("USB read error.");
            return;
        }
        emit update_progress(i);
    }
    usb_device->close_device();

    //Because the region 0x1800-0x1FBFF contains the dumper we overwrite it with the normal firmware from the update.dat file.
    firmware->decrypt_firmware(reinterpret_cast<uchar*>(&temp.data()[BOOTLOADER_SIZE]), static_cast<int>(device_type));


    //Write data to file.
    QFile file(fileName);
    if(!file.open(fileName.endsWith(".hex",Qt::CaseInsensitive) ? (QIODevice::WriteOnly | QIODevice::Text) : QIODevice::WriteOnly))
    {
        emit dump_status(file.errorString());
        return;
    }

    if(fileName.endsWith(".hex", Qt::CaseInsensitive))
    {
        //write temp array to fileStream in Intel hex format.
        HexWriter hexwriter(&file);
        hexwriter.WriteHex(temp);
    }
    else
    {
        //write temp array to fileStream in binary format
        file.write(temp);
    }
    file.close();
    emit dump_status("");
    return;
}



//Reflash finished SLOT
void MainWindow::reflash_finished(const QString &result)
{
    DeviceChanged(true);
    if(result.isEmpty())
        QMessageBox::information(this, "TL866", "     Reflash OK!          ");
    else
        QMessageBox::critical(this, "TL866", result);
    emit update_progress(0);
}


//Dump finished SLOT
void MainWindow::dump_finished(const QString &result)
{
    DeviceChanged(true);
    if(result.isEmpty())
        QMessageBox::information(this, "TL866", "Firmware dump complete!");
    else
        QMessageBox::critical(this, "TL866", QString("Error creating dump.hex file\n%1.").arg(result));
    emit update_progress(0);
}


//Gui update SLOT
void MainWindow::gui_updated(const QString &message, bool eraseLed, bool writeLed)
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
MainWindow::DeviceInfo MainWindow::DeviceChanged(bool arrived)
{
    DeviceInfo device_info = {"", "", 0};

    if(!arrived && reset_flag)//ignore unplug events if the device was resetted by us.
        return device_info;

    reset_flag=false;
    ui->txtInfo->clear();

    int count=usb_device->get_devices_count();
    this->setWindowTitle(QString("TL866 firmware updater %1 (%2 %3 connected)").arg(VERSION).arg(count).arg(count == 1 ? "device" : "devices"));

    if(count)
    {
        Firmware::tl866_report report;
        if(usb_device->open_device(0))
        {
            memset(reinterpret_cast<uchar*>(&report),0, sizeof(report));
            report.echo = REPORT_COMMAND;//0 anyway
            usb_device->usb_write(reinterpret_cast<uchar*>(&report), 5);
            usb_device->usb_read(reinterpret_cast<uchar*>(&report), sizeof(report));

            switch(report.device_version)
            {
            case Firmware::VERSION_TL866A:
                ui->txtInfo->append("Device version: TL866A");
                device_info.device_type = Firmware::VERSION_TL866A;
                break;
            case Firmware::VERSION_TL866CS:
                ui->txtInfo->append("Device version: TL866CS");
                device_info.device_type = Firmware::VERSION_TL866CS;
                break;
            default:
                ui->txtInfo->append("Device version: Unknown");
                device_info.device_type = 0;
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
            ui->btnReset->setProperty("state", false);
            device_info.device_code = QString::fromLatin1(reinterpret_cast<const char*>(&report.device_code),8);
            device_info.device_serial = QString::fromLatin1(reinterpret_cast<const char*>(&report.serial_number),24);
            bool isDumperActive = (device_info.device_code.toLower() == "codedump" && device_info.device_serial == "000000000000000000000000");


            if(isDumperActive)
            {
                Firmware::dumper_report dumper_report;
                uchar b[] = {DUMPER_INFO};
                usb_device->usb_write(b, 1);
                usb_device->usb_read(reinterpret_cast<uchar*>(&dumper_report), sizeof(dumper_report));
                device_info.device_type = dumper_report.bootloader_version;

                device_info.device_code = QString::fromLatin1(reinterpret_cast<const char*>(&dumper_report.device_code),8);
                device_info.device_serial = QString::fromLatin1(reinterpret_cast<const char*>(&dumper_report.serial_number),24);

                QString info;
                info.append(GetFormatedString(device_info.device_code, device_info.device_serial) + "\n");
                info.append(QString("Bootloader version: %1\n").arg((device_info.device_type == Firmware::VERSION_TL866A) ? "A" : "CS"));
                info.append(QString("Code Protection bit: %1\n").arg(dumper_report.cp_bit ? "No" : "Yes"));
                advdlg->SetUi(info, device_info.device_code, device_info.device_serial, dumper_report.cp_bit == 0, static_cast<int>(device_info.device_type));

                ui->btnAdvanced->setEnabled(true);
                ui->btnDump->setEnabled(true);
            }
            else//dumper is not active
            {
                ui->btnAdvanced->setEnabled(false);
                ui->btnDump->setEnabled(false);
                advdlg->hide();
            }

            ui->txtInfo->append(GetFormatedString(device_info.device_code, device_info.device_serial));
            ui->txtInfo->append(isDumperActive ? "Firmware version: Firmware dumper" :
                                                 report.device_status == Firmware::NORMAL_MODE ? QString("Firmware version: %1.%2.%3")
                                                                                                 .arg(report.hardware_version)
                                                                                                 .arg(report.firmware_version_major)
                                                                                                 .arg(report.firmware_version_minor):
                                                                                                 "Firmware version: Bootloader");
            uchar cs = 0;
            for (int i = 5; i < 8; i++)
                cs += report.device_code[i];
            for(int i = 0; i < 24; i++)
                cs+= report.serial_number[i];
            cs += report.b0;
            cs += report.b1;

            if (report.firmware_version_minor > 82 && cs != report.checksum && report.bad_serial == 0)
                ui->txtInfo->append("Bad serial checksum.");

            if (report.firmware_version_minor > 82 && report.bad_serial != 0)
                ui->txtInfo->append("Bad serial.");
            usb_device->close_device();
        }
        else//error oppening device
            SetBlank();

    }
    else//no device connected
        SetBlank();
    return device_info;
}

//Helper function to get formated device and serial code
const QString MainWindow::GetFormatedString(const QString &devcode, const QString &serial)
{
    return QString("Device code: %1\nSerial number: %2").arg(devcode.trimmed() +
                                                             (Firmware::IsBadCrc(devcode, serial) ? " (Bad device code)" : "")) .arg(serial.trimmed() +
                                                                                                                                     (Firmware::IsBadCrc(devcode, serial) ? " (Bad serial code)" : ""));
}


//Helper function
void MainWindow::SetBlank()
{
    leds_off();
    ui->btnAdvanced->setEnabled(false);
    ui->btnDump->setEnabled(false);
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
            usb_device->close_device();
            return;
        }
        uchar b[2]={DUMPER_WRITE_BOOTLOADER, static_cast<uchar>((type ==  Firmware::A_BOOTLOADER ? Firmware::VERSION_TL866A : Firmware::VERSION_TL866CS))};
        usb_device->usb_write(b, 2);
        b[0] = 0;
        usb_device->usb_read(b, 1);
        usb_device->close_device();
        DeviceChanged(true);
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
        uchar b[2]={DUMPER_WRITE_CONFIG, static_cast<uchar>((copy_protect ? 1 : 0))};
        usb_device->usb_write(b, 2);
        b[0] = 0;
        usb_device->usb_read(b, 1);
        usb_device->close_device();
        DeviceChanged(true);
        if(b[0] == DUMPER_WRITE_CONFIG)
            QMessageBox::information(advdlg, "TL866", "Code protection bit was successfully written.");
        else
            QMessageBox::critical(advdlg, "TL866", "Writing code protect bit failed.");
    }

}

//write serial number and device code
void MainWindow::WriteInfo(const QString &device_code, const QString &serial_number)
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
        memcpy(b+1, device_code.toLatin1().data(), static_cast<uint>(device_code.size()));//copy devcode to b array
        memcpy(b+9, serial_number.toLatin1().data(), static_cast<uint>(serial_number.size()));//copy serial to key array
        usb_device->usb_write(b, 34);
        b[0] = 0;
        usb_device->usb_read(b, 1);
        usb_device->close_device();
        DeviceChanged(true);
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

