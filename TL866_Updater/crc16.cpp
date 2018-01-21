/* Class CRC16
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


#include "crc16.h"

CRC16::CRC16()
{
    const ushort poly = 0xA001;
    for (ushort i = 0; i < 256; i++)
    {
        ushort temp = i;
        for (uchar j = 0; j < 8; j++)
            if ((temp & 1) == 1)
                temp = (ushort)((temp >> 1) ^ poly);
            else
                temp >>= 1;
        table[i] = temp;
    }
}


ushort CRC16::crc16(const uchar *data, uint length, ushort initial)
{
    while(length--)
    {
        initial=((initial>>8)^table[(initial^*data++)&0xFF]);
    }
    return initial;
}
