/* Edit Dialog window class
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


#include "editdialog.h"
#include "ui_editdialog.h"
#include "firmware.h"
#include <QMessageBox>

EditDialog::EditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditDialog)
{
    ui->setupUi(this);
    connect(ui->buttonBox,SIGNAL(accepted()),this,SLOT(okButton_clicked()));
    setFixedSize(size());
}

EditDialog::~EditDialog()
{
    delete ui;
}

void EditDialog::GetResult(QString* devcode, QString* serial)
{
  *devcode = ui->txtDevcode->text();
  *serial = ui->txtSerial->text();
}

void EditDialog::SetText(QString devcode, QString serial)
{
    ui->txtDevcode->setText(devcode);
    ui->txtSerial->setText(serial);
}

void EditDialog::on_btnRndDev_clicked()
{
    int i;
    QString s;
    for(i=0;i<8;i++)
    {
        s.append(QString::number( qrand() % 10));
    }
    ui->txtDevcode->setText(s);
}

void EditDialog::on_btnRndSer_clicked()
{
    int i;
    QString s;
    for(i=0;i<24;i++)
    {
        s.append(QString::number(qrand()%16,16).toUpper());
    }
    ui->txtSerial->setText(s);
}

void EditDialog::okButton_clicked()
{
    if(ui->txtDevcode->text()=="codedump" && ui->txtSerial->text()=="000000000000000000000000")
    {
        QMessageBox::warning(this, "TL866", "Please enter another device and serial code!\nThese two are reserved.");
        return;
    }
    if(Firmware::IsBadCrc((uchar*)ui->txtDevcode->text().toLatin1().data(), (uchar*)ui->txtSerial->text().toLatin1().data()))
    {
         QMessageBox::warning(this, "TL866", "Bad Device and serial code!\nPlease try again.");
         return;
    }
  accept();
}


