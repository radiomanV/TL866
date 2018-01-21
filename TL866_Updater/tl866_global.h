#ifndef TL866_GLOBAL_H
#define TL866_GLOBAL_H


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

 enum BootloaderType{A_BOOTLOADER, CS_BOOTLOADER};
 enum FirmwareType{FIRMWARE_A, FIRMWARE_CS, FIRMWARE_CUSTOM};
 enum DEVICE_VERSION{VERSION_TL866A = 1, VERSION_TL866CS = 2};
 enum DEVICE_STATUS{NORMAL_MODE = 1, BOOTLOADER_MODE = 2};

 typedef struct {
     uchar   echo;
     uchar   device_status;
     ushort  report_size;
     uchar   firmware_version_minor;
     uchar   firmware_version_major;
     uchar   device_version;
     uchar   device_code[8];
     uchar   serial_number[24];
     uchar   hardware_version;
 }TL866_REPORT;


 typedef struct{
     uchar   device_code[8];
     uchar   serial_number[24];
     uchar   bootloader_version;
     uchar   cp_bit;
 }DUMPER_REPORT;


#endif // TL866_GLOBAL_H
