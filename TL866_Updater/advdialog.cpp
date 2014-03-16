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
#include "tl866_global.h"


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
   device_code = devcode;
   serial_number = serial;
}

void AdvDialog::SetInfo(QString info)
{
   ui->txtInfo->clear();
   ui->txtInfo->setText(info);
}

void AdvDialog::SetUi(bool cp, int type)
{
  if(type == VERSION_TL866A)
      ui->radioA->setChecked(true);
  if(type == VERSION_TL866CS)
      ui->radioCS->setChecked(true);
    ui->optionCP->setChecked(cp);
}

void AdvDialog::on_btnEdit_clicked()
{
    EditDialog* dlg=new EditDialog(this);
    QString devcode =ui->txtDevcode->text();
    QString serial =ui->txtSerial->text();
    dlg->SetText(devcode,serial);
   if(dlg->exec()==QDialog::Accepted)
   {
     dlg->GetResult(&devcode, &serial);
     ui->txtDevcode->setText(devcode);
     ui->txtSerial->setText(serial);
   }
}


void AdvDialog::on_btnDefault_clicked()
{
    ((MainWindow*)this->parent())->set_default(ui->txtDevcode, ui->txtSerial);
}

void AdvDialog::on_btnClone_clicked()
{
    ((MainWindow*)this->parent())->Refresh();
    ui->txtDevcode->setText(device_code);
    ui->txtSerial->setText(serial_number);
}

void AdvDialog::on_btnWriteBootloader_clicked()
{
     ((MainWindow*)this->parent())->WriteBootloader(ui->radioA->isChecked() ? A_BOOTLOADER : CS_BOOTLOADER);
}

void AdvDialog::on_btnWriteConfig_clicked()
{
    ((MainWindow*)this->parent())->WriteConfig(ui->optionCP->isChecked());
}

void AdvDialog::on_btnWriteInfo_clicked()
{
    ((MainWindow*)this->parent())->WriteInfo(ui->txtDevcode->text(), ui->txtSerial->text());
}
