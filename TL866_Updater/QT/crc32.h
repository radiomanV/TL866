#ifndef CRC32_H
#define CRC32_H

#include <QtGlobal>

class CRC32
{
public:
    CRC32();
    uint crc32(const uchar *data, uint length, uint initial);

private:
    uint table[256];
};

#endif // CRC32_H
