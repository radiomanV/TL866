/* Class HexWriter
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

#include "hexwriter.h"


HexWriter::HexWriter(QIODevice *file)
{
   outStream.setDevice(file);
}

void HexWriter::WriteHex(QByteArray data)
{
    uchar temp[2];
    ushort segment=0;
    ushort address=0;
    int size = data.size();

    while(size>16)
    {
        if(!address)//address is zero, insert extended linear address record
        {
            temp[0]=(segment>>8);
            temp[1]=(segment&0xff);
            segment++;
            outStream << GetHexLine(temp,2,0,SEGMENT_RECORD) << endl;
        }
        outStream << GetHexLine(reinterpret_cast<uchar*>(&data.data()[(segment-1)*0x10000+address]),16,address,DATA_RECORD) << endl;
        address+=16;
        size-=16;
    }
   outStream << GetHexLine(reinterpret_cast<uchar*>(&data.data()[(segment-1)*0x10000+address]), static_cast<uchar>(size/2),address,DATA_RECORD) << endl;
   size/=2;
   address+=size;
   outStream << GetHexLine(reinterpret_cast<uchar*>(&data.data()[(segment-1)*0x10000+address]),static_cast<uchar>(size),address,DATA_RECORD) << endl;
   outStream << GetHexLine(nullptr,0,0,EOF_RECORD) << endl;
}


QString HexWriter::GetHexLine(uchar *data, uchar size, ushort address, uchar recordtype)
{
    QString s=(QString(":%1%2%3").arg(size & 0xff, 2, 16, QChar('0')).arg(address, 4, 16, QChar('0')).arg(recordtype, 2, 16, QChar('0')).toUpper()).toLocal8Bit();
    uchar cs = size;
    cs += recordtype;
    cs += (address >> 8);
    cs += (address & 0xff);
    if(data!=nullptr)
    {
        for(int i=0;i<size;i++)
        {
            s.append(QString("%1").arg(data[i], 2, 16, QChar('0')).toUpper());
            cs += data[i];
        }
    }
    cs = (~cs + 1) & 0xff;
    s.append(QString("%1").arg(cs, 2, 16, QChar('0')).toUpper());
    return s;
}
