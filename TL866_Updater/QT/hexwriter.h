#ifndef HEXWRITER_H
#define HEXWRITER_H
#include <QTextStream>

class HexWriter
{
public:
    explicit HexWriter(QIODevice *file);
    void WriteHex(QByteArray data);

private:
    QString GetHexLine(uchar *data, uchar size, ushort address ,uchar recordtype);
    QTextStream outStream;

    enum RECORD_TYPE
    {
        DATA_RECORD = 0,
        SEGMENT_RECORD = 4,// Extended Linear Address Record
        EOF_RECORD = 1
    };
};

#endif // HEXWRITER_H
