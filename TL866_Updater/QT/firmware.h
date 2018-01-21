#ifndef FIRMWARE_H
#define FIRMWARE_H

#include <QString>
#include "tl866_global.h"

#define UPDATE_DAT_SIZE             312348
#define BLOCK_SIZE                  80
#define XOR_TABLE_SIZE              0x100
#define XOR_TABLE_START             0x1EEDF
#define XOR_TABLE_OFFSET            0x1FC00


class Firmware
{
public:

    Firmware();
    int open(const QString &filename);
    bool isValid();
    unsigned char GetEraseParammeter(int type);
    unsigned char GetFirmwareVersion();
    void decrypt_firmware(unsigned char *data_out, int type);
    void encrypt_firmware(const unsigned char *data_in, unsigned char *data_out, int key);
    void get_firmware(unsigned char *data_out, int type, int key);
    void encrypt_serial(unsigned char *key, const unsigned char *firmware);
    void decrypt_serial(unsigned char *key, const unsigned char *firmware);
    static bool IsBadCrc(uchar *devcode, uchar *serial);


    enum
       {
           NoError,
           OpenError,
           FilesizeError,
           CRCError,
           DecryptionError
       };


      enum ENCRYPTION_KEY
        {
          A_KEY  = VERSION_TL866A,
          CS_KEY = VERSION_TL866CS
        };

private:

    typedef struct {
        unsigned char header[4];//file header
        unsigned int  A_CRC32;//4 bytes
        unsigned char pad1;
        unsigned char A_erase;
        unsigned char pad2;
        unsigned char pad3;
        unsigned int  CS_CRC32;//4 bytes
        unsigned char pad4;
        unsigned char CS_erase;
        unsigned char pad5;
        unsigned char pad6;
        unsigned int  A_Index;//index used in A firmware decryption
        unsigned char A_Xortable1[256];//First xortable used in A firmware decryption
        unsigned char A_Xortable2[1024];//Second xortable used in A firmware decryption
        unsigned int  CS_Index;//index used in CS firmware decryption
        unsigned char CS_Xortable1[256];//First xortable used in CS firmware decryption
        unsigned char CS_Xortable2[1024];//Second xortable used in CS firmware decryption
        unsigned char A_Firmware[ENCRYPTED_FIRMWARE_SIZE];//Encrypted A firmware
        unsigned char CS_Firmware[ENCRYPTED_FIRMWARE_SIZE];//Encrypted CS firmware
    }UpdateDat;

unsigned int crc32(unsigned char *buffer, unsigned int length);
void encrypt_block(unsigned char *data, unsigned char *xortable, int index);
void decrypt_block(unsigned char *data, unsigned char *xortable, int index);


unsigned int crc32Table[256];
unsigned char m_firmwareA[ENCRYPTED_FIRMWARE_SIZE ];
unsigned char m_firmwareCS[ENCRYPTED_FIRMWARE_SIZE ];
unsigned char m_eraseA;
unsigned char m_eraseCS;
unsigned char m_version;
bool m_isValid;
};

#endif // FIRMWARE_H
