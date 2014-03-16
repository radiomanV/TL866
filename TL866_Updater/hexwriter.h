#ifndef HEXWRITER_H
#define HEXWRITER_H
#include <QTextStream>

class HexWriter
{
public:
    void WriteHex(QTextStream &outStream, const uchar *data, uint size);

private:
    QString GetHexLine(const uchar *data, ushort size, ushort address ,uchar recordtype);

    enum RECORD_TYPE
    {
        DATA_RECORD = 0,
        SEGMENT_RECORD = 4,// Extended Linear Address Record
        EOF_RECORD = 1
    };
};

#endif // HEXWRITER_H
