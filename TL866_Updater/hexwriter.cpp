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


void HexWriter::WriteHex(QTextStream &outStream, const uchar *data, uint size)
{
    uchar temp[2];
    ushort segment=0;
    ushort address=0;

    while(size>16)
    {
        if(!address)//address is zero, insert extended linear address record
        {
            temp[0]=(segment>>8);
            temp[1]=(segment&0xff);
            segment++;
            outStream << GetHexLine(temp,2,0,SEGMENT_RECORD) << endl;
        }
        outStream << GetHexLine(&data[(segment-1)*0x10000+address],16,address,DATA_RECORD) << endl;
        address+=16;
        size-=16;
    }
   outStream << GetHexLine(&data[(segment-1)*0x10000+address],size/2,address,DATA_RECORD) << endl;
   size/=2;
   address+=size;
   outStream << GetHexLine(&data[(segment-1)*0x10000+address],size,address,DATA_RECORD) << endl;
   outStream << GetHexLine(NULL,0,0,EOF_RECORD) << endl;
}


QString HexWriter::GetHexLine(const uchar *data, ushort size, ushort address ,uchar recordtype)
{
    QString s=(QString(":%1%2%3").arg(size & 0xff, 2, 16, QChar('0')).arg(address, 4, 16, QChar('0')).arg(recordtype, 2, 16, QChar('0')).toUpper()).toLocal8Bit();
    ushort checksum=size;
    checksum += recordtype;
    checksum += (address >> 8);
    checksum += (address & 0xff);
    if(data!=NULL)
    {
        for(int i=0;i<size;i++)
        {
            s.append(QString("%1").arg(data[i], 2, 16, QChar('0')).toUpper());
            checksum += data[i];
        }
    }
    checksum &=0xff;
    checksum ^=0xff;
    checksum ++;
    checksum &=0xff;
    s.append(QString("%1").arg(checksum, 2, 16, QChar('0')).toUpper());
    return s;
}
