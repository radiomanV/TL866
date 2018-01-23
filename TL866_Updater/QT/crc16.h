#ifndef CRC16_H
#define CRC16_H

#include <QtGlobal>

class CRC16
{
public:
    CRC16();
    ushort crc16(const uchar *data, unsigned int length, ushort initial);

private:
   ushort table[256];
};

#endif // CRC16_H
