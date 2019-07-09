#ifndef FIRMWARE_H
#define FIRMWARE_H

#include <QString>

#define UPDATE_DAT_SIZE             312348
#define BLOCK_SIZE                  80
#define XOR_TABLE_SIZE              0x100
#define XOR_TABLE_OFFSET            0x1FC00
#define FIRMWARE_SIGNATURE_OFFSET   0x1E3FC
#define CP0_ADDRESS                 0x1FFF9
#define FIRMWARE_SIGNATURE          0x5AA5AA55

#define SERIAL_OFFSET               0x1FD00
#define FLASH_SIZE                  0x20000
#define BOOTLOADER_SIZE             0x1800
#define ENCRYPTED_FIRMWARE_SIZE     0x25D00
#define UNENCRYPTED_FIRMWARE_SIZE   0x1E400

#define WRITE_COMMAND               0xAA
#define ERASE_COMMAND               0xCC
#define RESET_COMMAND               0xFF
#define REPORT_COMMAND              0x00

#define DUMPER_READ_FLASH           0x01
#define DUMPER_WRITE_BOOTLOADER     0x02
#define DUMPER_WRITE_CONFIG         0x03
#define DUMPER_WRITE_INFO           0x04
#define DUMPER_INFO                 0x05

#define A_BOOTLOADER_CRC            0x1B8960EF
#define CS_BOOTLOADER_CRC           0xFB3DED05
#define BAD_CRC                     0xC8C2F013

#define TL866_VID 0x04D8
#define TL866_PID 0xE11C


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
    static bool IsBadCrc(const QString &devcode, const QString &serial);


    struct tl866_report {
        uchar   echo;
        uchar   device_status;
        ushort  report_size;
        uchar   firmware_version_minor;
        uchar   firmware_version_major;
        uchar   device_version;
        uchar   device_code[8];
        uchar   serial_number[24];
        uchar   hardware_version;
        uchar   b0;
        uchar   b1;
        uchar   checksum;
        uchar   bad_serial;
        unsigned char buffer[20];//Increased the buffer size like in C# version for future autoelectric expansion.
    };


    struct dumper_report{
        uchar   device_code[8];
        uchar   serial_number[24];
        uchar   bootloader_version;
        uchar   cp_bit;
    };


    enum BootloaderType{A_BOOTLOADER, CS_BOOTLOADER};
    enum FirmwareType{FIRMWARE_A, FIRMWARE_CS, FIRMWARE_CUSTOM};
    enum DEVICE_VERSION{VERSION_TL866A = 1, VERSION_TL866CS = 2};
    enum DEVICE_STATUS{NORMAL_MODE = 1, BOOTLOADER_MODE = 2};


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

    struct UpdateDat {
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
    };

    void encrypt_block(unsigned char *data, const unsigned char *xortable, unsigned char index);
    void decrypt_block(unsigned char *data, const unsigned char *xortable, unsigned char index);


    unsigned char m_firmwareA[ENCRYPTED_FIRMWARE_SIZE ];
    unsigned char m_firmwareCS[ENCRYPTED_FIRMWARE_SIZE ];
    static const unsigned char XortableA[XOR_TABLE_SIZE];
    static const unsigned char XortableCS[XOR_TABLE_SIZE];
    unsigned char m_eraseA;
    unsigned char m_eraseCS;
    unsigned char m_version;
    bool m_isValid;
};

#endif // FIRMWARE_H
