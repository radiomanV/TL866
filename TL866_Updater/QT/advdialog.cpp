/* Advanced Dialog window class
*
* This file is part of the TL866 updater project.
*
* Copyright (C) radioman 2013
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


#include "advdialog.h"
#include "ui_advdialog.h"
#include "editdialog.h"
#include "mainwindow.h"


AdvDialog::AdvDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AdvDialog)
{
    ui->setupUi(this);
    setFixedSize(size());
}

AdvDialog::~AdvDialog()
{
    delete ui;
}

void AdvDialog::SetSerial(QString devcode, QString serial)
{
   ui->txtDevcode->clear();
   ui->txtSerial->clear();
   ui->txtDevcode->setText(devcode);
   ui->txtSerial->setText(serial);
   this->setProperty("device_code",devcode);
   this->setProperty("serial_number",serial);
}

void AdvDialog::SetInfo(QString info)
{
   ui->txtInfo->clear();
   ui->txtInfo->setText(info);
}

void AdvDialog::SetUi(bool cp, int type)
{
  if(type == Firmware::VERSION_TL866A)
      ui->radioA->setChecked(true);
  if(type == Firmware::VERSION_TL866CS)
      ui->radioCS->setChecked(true);
    ui->optionCP->setChecked(cp);
}

void AdvDialog::on_btnEdit_clicked()
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


void AdvDialog::on_btnDefault_clicked()
{
    ui->txtDevcode->setText("00000000");
    ui->txtSerial->setText("000000000000000000000000");
}

void AdvDialog::on_btnClone_clicked()
{
    emit Refresh();
    ui->txtDevcode->setText(this->property("device_code").toString());
    ui->txtSerial->setText(this->property("serial_number").toString());
}

void AdvDialog::on_btnWriteBootloader_clicked()
{
    emit WriteBootloader(ui->radioA->isChecked() ? Firmware::A_BOOTLOADER : Firmware::CS_BOOTLOADER);
}

void AdvDialog::on_btnWriteConfig_clicked()
{
    emit WriteConfig(ui->optionCP->isChecked());
}

void AdvDialog::on_btnWriteInfo_clicked()
{
   emit WriteInfo(ui->txtDevcode->text(), ui->txtSerial->text());;
}
