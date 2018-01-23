#ifndef CRC_H
#define CRC_H

#include <QtGlobal>

class CRC
{
public:
    CRC();
    ushort crc16(const uchar *data, uint length, ushort initial);
    uint   crc32(const uchar *data, uint length, uint initial);


private:
   ushort table16[256];
   uint   table32[256];
};

#endif // CRC_H
