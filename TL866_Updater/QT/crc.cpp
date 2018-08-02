/* Class CRC
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


#include "crc.h"

CRC::CRC()
{
    const uint   poly32 = 0xEDB88320;
    const ushort poly16 = 0xA001;
    for (ushort i = 0; i < 256; i++)
    {
        uint   t32 = i;
        ushort t16 = i;
        for (uint j = 0; j < 8; j++)
        {
            if ((t32 & 1) == 1)
                t32 = (t32 >> 1) ^ poly32;
            else
                t32 >>= 1;
            if ((t16 & 1) == 1)
                t16 = static_cast<ushort>((t16 >> 1) ^ poly16);
            else
                t16 >>= 1;
        }
        table32[i] = t32;
        table16[i] = t16;
    }
}


uint CRC::crc32(const uchar *data, uint length, uint initial)
{
    while(length--)
    {
        initial = ((initial>>8)^table32[(initial^*data++)&0xFF]);
    }
    return initial;
}


ushort CRC::crc16(const uchar *data, uint length, ushort initial)
{
    while(length--)
    {
        initial=((initial>>8)^table16[(initial^*data++)&0xFF]);
    }
    return initial;
}
