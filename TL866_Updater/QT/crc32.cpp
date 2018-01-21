/* Class CRC32
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


#include "crc32.h"

CRC32::CRC32()
{
    const uint poly=0xEDB88320;
    for (uint i = 0; i < 256; i++)
    {
        uint temp = i;
        for (uint j = 0; j < 8; j++)
            if ((temp & 1) == 1)
                temp = (temp >> 1) ^ poly;
            else
                temp >>= 1;
        table[i] = temp;
    }
}


uint CRC32::crc32(const uchar *data, uint length, uint initial)
{
    while(length--)
    {
        initial = ((initial>>8)^table[(initial^*data++)&0xFF]);
    }
    return initial;
}
