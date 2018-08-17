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
#include "crc.h"
#include <QMessageBox>

EditDialog::EditDialog(QWidget *parent, QString devcode, QString serial) :
    QDialog(parent),
    ui(new Ui::EditDialog)
{
    ui->setupUi(this);
    connect(ui->buttonBox,SIGNAL(accepted()),this,SLOT(okButton_clicked()));
    setFixedSize(size());
    ui->txtDevcode->setText(devcode);
    ui->txtSerial->setText(serial);
}

EditDialog::~EditDialog()
{
    delete ui;
}

void EditDialog::GetResult(QString& devcode, QString& serial)
{
    devcode = ui->txtDevcode->text();
    serial = ui->txtSerial->text();
}

void EditDialog::on_btnRndDev_clicked()
{
    int i;
    QString s;
    for(i=0;i<8;i++)
    {
        s.append(QString::number(qrand() % 10));
    }
    ui->txtDevcode->setText(s);
    on_btnRndSer_clicked();
}

void EditDialog::on_btnRndSer_clicked()
{
    if(ui->txtDevcode->text().isEmpty())
        on_btnRndDev_clicked();
    int i;
    QString s;
    ushort crc16 = get_dev_crc();
    s.append(QString("%1").arg(crc16 >> 8, 2, 16, QLatin1Char('0')).toUpper());
    s.append(QString("%1").arg(qrand()%256, 2, 16, QLatin1Char('0')).toUpper());
    s.append(QString("%1").arg(crc16 & 0xFF, 2, 16, QLatin1Char('0')).toUpper());
    for(i=0;i<9;i++)
    {
        s.append(QString("%1").arg(qrand()%256, 2, 16, QLatin1Char('0')).toUpper());
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
    if(Firmware::IsBadCrc(ui->txtDevcode->text(), ui->txtSerial->text()))
    {
        QMessageBox::warning(this, "TL866", "Bad Device and serial code!\nPlease try again.");
        return;
    }
    accept();
}

ushort EditDialog::get_dev_crc()
{
    CRC crc;
    return crc.crc16(reinterpret_cast<uchar*>(ui->txtDevcode->text().toLatin1().data()),static_cast<uint>(ui->txtDevcode->text().size()),0);
}

void EditDialog::on_txtDevcode_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    if (ui->txtDevcode->text().isEmpty())
    {
        ui->txtSerial->clear();
        return;
    }
    if(ui->txtDevcode->text().length() && ui->txtSerial->text().length()<24)
        on_btnRndSer_clicked();
    if(ui->txtSerial->text().length()>5)
    {
        ushort crc;
        bool ok;
        crc = static_cast<ushort>(ui->txtSerial->text().left(2).toUShort(&ok,16) << 8);
        if(ok)
            crc += ui->txtSerial->text().mid(4,2).toUShort(&ok,16);
        if(ok && get_dev_crc() != crc)
           on_btnRndSer_clicked();
    }
}
