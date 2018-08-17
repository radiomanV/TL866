/* Class Firmware
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


#include "firmware.h"
#include "crc.h"
#include <QFile>
#include<QTime>
#include <QDebug>

const unsigned char Firmware::XortableA[] = {
    0xA4, 0x1E, 0x42, 0x8C, 0x3C, 0x76, 0x14, 0xC7, 0xB8, 0xB5, 0x81, 0x4A, 0x13, 0x37, 0x7C, 0x0A,
    0xFE, 0x3B, 0x63, 0xC1, 0xD5, 0xFD, 0x8C, 0x39, 0xD1, 0x1F, 0x22, 0xC7, 0x7F, 0x4D, 0x2F, 0x15,
    0x71, 0x21, 0xF9, 0x25, 0x33, 0x44, 0x92, 0x93, 0x80, 0xD7, 0xAB, 0x1B, 0xB6, 0x11, 0xA9, 0x5A,
    0x88, 0x29, 0xFB, 0xD9, 0xF3, 0x76, 0xAA, 0x47, 0x73, 0xD5, 0x31, 0x06, 0x76, 0x4B, 0x90, 0xEA,
    0x11, 0xEB, 0x9C, 0x3D, 0xF2, 0xFA, 0x99, 0x06, 0x96, 0x52, 0x0A, 0x8A, 0xBC, 0x04, 0xC8, 0x14,
    0x19, 0x41, 0x52, 0xF2, 0x4D, 0x7B, 0x64, 0xC0, 0x16, 0xC7, 0xCB, 0xE9, 0xC3, 0x86, 0x77, 0x6A,
    0xEC, 0x44, 0xD2, 0xD9, 0x61, 0xE0, 0x50, 0xA6, 0x60, 0xED, 0x47, 0xA2, 0x0B, 0x59, 0x02, 0xBD,
    0x18, 0x4C, 0x11, 0x14, 0xCB, 0x53, 0xE2, 0x2B, 0x21, 0xBE, 0x96, 0x76, 0x4F, 0x47, 0x0D, 0x1F,
    0x6A, 0xF4, 0x43, 0x03, 0x68, 0x3E, 0xE0, 0xFE, 0x47, 0x72, 0x0A, 0x68, 0x8C, 0x58, 0x7E, 0xDF,
    0xEF, 0x13, 0xDF, 0x47, 0x55, 0x48, 0x4D, 0x10, 0xFE, 0x82, 0x3A, 0xB7, 0x00, 0xD5, 0x79, 0x90,
    0xF4, 0xC2, 0x98, 0xC2, 0xEF, 0x5B, 0x70, 0x93, 0xB4, 0xA7, 0xFA, 0xE6, 0x27, 0x48, 0x65, 0x01,
    0x05, 0x5B, 0x65, 0x94, 0xD3, 0xA0, 0xCD, 0xF7, 0x14, 0xDB, 0x60, 0xB4, 0xBF, 0x7A, 0xE4, 0x45,
    0xF0, 0x77, 0x79, 0x1F, 0xDE, 0x80, 0x29, 0xEF, 0x0D, 0x56, 0xC0, 0x23, 0xC5, 0x73, 0xDE, 0xAC,
    0xC2, 0xEF, 0x4A, 0x02, 0x2D, 0xA4, 0x89, 0x69, 0xCB, 0x91, 0xB0, 0x74, 0x75, 0x7C, 0x76, 0xC7,
    0xC8, 0xDB, 0x8D, 0x20, 0x1D, 0xF5, 0x33, 0x99, 0xBB, 0x45, 0x04, 0x27, 0x4C, 0x1F, 0x12, 0x67,
    0x8E, 0x96, 0x37, 0x9A, 0x4B, 0x9C, 0xAA, 0xED, 0x8B, 0x6B, 0xD1, 0xFF, 0x08, 0x24, 0x56, 0x9D
};

const unsigned char Firmware::XortableCS[] = {
    0x0B, 0x08, 0x07, 0x18, 0xEC, 0xC7, 0xDF, 0x8C, 0xD6, 0x76, 0xCE, 0x10, 0x9F, 0x61, 0x7C, 0xF5,
    0x61, 0x09, 0xFB, 0x59, 0xD0, 0x24, 0xB4, 0x4F, 0xCA, 0xE4, 0xA1, 0x3A, 0x30, 0x7C, 0xBD, 0x7A,
    0xF5, 0xE1, 0xB9, 0x4B, 0x74, 0xCD, 0xF1, 0xE9, 0x07, 0x0A, 0x9E, 0xF9, 0xD5, 0xED, 0x4D, 0x24,
    0xEB, 0x21, 0x90, 0x05, 0x8F, 0xA5, 0xF3, 0x45, 0xD0, 0x18, 0x31, 0x04, 0x62, 0x35, 0xA8, 0x7B,
    0xA9, 0x9A, 0x0B, 0xE0, 0x14, 0xCD, 0x57, 0x8A, 0xAC, 0x80, 0x08, 0x56, 0xED, 0x14, 0x8C, 0x49,
    0xD4, 0x5D, 0xF8, 0x77, 0x39, 0xA5, 0xFA, 0x23, 0x5F, 0xF3, 0x0E, 0x27, 0xCA, 0x8D, 0xF5, 0x97,
    0x50, 0xBB, 0x64, 0xA1, 0x73, 0xCE, 0xF9, 0xB7, 0xEE, 0x61, 0x72, 0xF1, 0x8E, 0xDF, 0x21, 0xAC,
    0x43, 0x45, 0x9B, 0x78, 0x77, 0x29, 0xB1, 0x31, 0x9E, 0xFC, 0xA1, 0x6B, 0x0F, 0x8C, 0x8D, 0x13,
    0x12, 0xCC, 0x2B, 0x54, 0x3A, 0xD8, 0xBF, 0xB8, 0xF5, 0x34, 0x46, 0x90, 0x61, 0x54, 0xF4, 0x95,
    0x61, 0x62, 0xE1, 0xCF, 0xF1, 0x3B, 0x00, 0xB6, 0xB6, 0xBB, 0x50, 0x98, 0xD9, 0x3A, 0x56, 0x3A,
    0x16, 0x56, 0xCA, 0xC2, 0x10, 0xF3, 0x91, 0xD4, 0xE8, 0x81, 0xEB, 0xFC, 0x0D, 0x7E, 0xEE, 0x4C,
    0x56, 0x3B, 0x33, 0x46, 0x4E, 0xE2, 0xCF, 0xFC, 0xCF, 0xB8, 0x84, 0x75, 0xD2, 0xA0, 0x39, 0x53,
    0x85, 0xE1, 0xA8, 0xB3, 0x9E, 0x28, 0x57, 0x55, 0xEF, 0xD1, 0xC9, 0xFD, 0x3B, 0x62, 0xF5, 0x18,
    0x49, 0x58, 0xF7, 0xA3, 0x36, 0x27, 0x06, 0x49, 0x0F, 0x7C, 0xA6, 0xCB, 0xA0, 0xC5, 0x1E, 0xA5,
    0x86, 0xF3, 0x2D, 0xEF, 0x8C, 0x7E, 0xF9, 0x81, 0x34, 0xAA, 0x48, 0x5A, 0x93, 0x0A, 0xF2, 0x43,
    0x62, 0x42, 0x97, 0xAF, 0x53, 0x10, 0x8D, 0xE6, 0xA1, 0x8E, 0x1C, 0x62, 0xEB, 0xB1, 0xEE, 0x79
};

Firmware::Firmware()
{
    m_isValid=false;
    m_eraseA=0;
    m_eraseCS=0;
    qsrand(QDateTime::currentDateTime().toTime_t());

}

//Open update.dat file and decrypt it.
int Firmware::open(const QString &filename)
{
    m_isValid=false;
    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
        return OpenError;

    if (file.size()!=UPDATE_DAT_SIZE)
    {
        file.close();
        return FilesizeError;
    }

    UpdateDat upd;
    if(file.read(reinterpret_cast<char*>(&upd),sizeof(upd))!=UPDATE_DAT_SIZE)
    {
        file.close();
        return OpenError;
    }
    file.close();
    m_eraseA = upd.A_erase;
    m_eraseCS = upd.CS_erase;
    m_version = upd.header[0];

    unsigned int i;
    //Decrypt firmwares (first step).
    for(i=0;i<sizeof(m_firmwareA);i++)
    {
        //Try to understand these ;)
        m_firmwareA[i]   = upd.A_Firmware[i]  ^ upd.A_Xortable2[(i+upd.A_Index)&0x3FF]   ^ upd.A_Xortable1[(i/80)&0xFF];
        m_firmwareCS[i]  = upd.CS_Firmware[i] ^ upd.CS_Xortable2[(i+upd.CS_Index)&0x3FF] ^ upd.CS_Xortable1[(i/80)&0xFF];
    }
    CRC crc;
    //Check if decryption is ok
    if((upd.A_CRC32!=~crc.crc32(m_firmwareA,sizeof(m_firmwareA), 0xFFFFFFFF))||(upd.CS_CRC32!=~crc.crc32(m_firmwareCS,sizeof(m_firmwareCS), 0xFFFFFFFF)))
        return CRCError;

    //Check if decrypted firmware signatures are O.K.
    unsigned char ta [UNENCRYPTED_FIRMWARE_SIZE];
    unsigned char tcs [UNENCRYPTED_FIRMWARE_SIZE];
    decrypt_firmware(ta,VERSION_TL866A);
    decrypt_firmware(tcs,VERSION_TL866CS);
    unsigned int sa = static_cast<unsigned int>((ta[FIRMWARE_SIGNATURE_OFFSET+3] << 24) | (ta[FIRMWARE_SIGNATURE_OFFSET+2] << 16) | (ta[FIRMWARE_SIGNATURE_OFFSET+1] << 8) | ta[FIRMWARE_SIGNATURE_OFFSET]);
    unsigned int scs = static_cast<unsigned int>((tcs[FIRMWARE_SIGNATURE_OFFSET+3] << 24) | (tcs[FIRMWARE_SIGNATURE_OFFSET+2] << 16) | (tcs[FIRMWARE_SIGNATURE_OFFSET+1] << 8) | tcs[FIRMWARE_SIGNATURE_OFFSET]);
    if(sa != FIRMWARE_SIGNATURE || scs != FIRMWARE_SIGNATURE)
        return DecryptionError;

    m_isValid=true;
    return NoError;
}


//Get a magic number used in erase command
unsigned char Firmware::GetEraseParammeter(int type)
{
    return (type == VERSION_TL866A ? m_eraseA : m_eraseCS);
}

unsigned char Firmware::GetFirmwareVersion()
{
    return m_version;
}

//Get the status of the firmware
bool Firmware::isValid()
{
    return m_isValid;
}

//Get encrypted firmware
void Firmware::get_firmware(unsigned char *data_out, int type, int key)
{
    if(type == key)
    {
        memcpy(data_out, type == VERSION_TL866A ? m_firmwareA : m_firmwareCS, ENCRYPTED_FIRMWARE_SIZE);
        return;
    }
    unsigned char data[UNENCRYPTED_FIRMWARE_SIZE];
    decrypt_firmware(data,key);
    encrypt_firmware(data,data_out,type);
}


//Encrypt firmware
void Firmware::encrypt_firmware(const unsigned char *data_in, unsigned char *data_out, int key)
{
    unsigned char data[BLOCK_SIZE];
    unsigned char index = key == VERSION_TL866A ? m_eraseA : m_eraseCS;
    const unsigned char* xortable = key == VERSION_TL866A ? Firmware::XortableA : Firmware::XortableCS;

    //encrypt firmware
    for(int i=0;i<UNENCRYPTED_FIRMWARE_SIZE;i+=BLOCK_SIZE-16)
    {
        memcpy(data,data_in+i,BLOCK_SIZE-16);
        encrypt_block(data,xortable,index);
        memcpy(data_out,data,BLOCK_SIZE);
        data_out+=BLOCK_SIZE;
        index+=4;
        index&=0xFF;
    }
}

//decrypt firmware
void Firmware::decrypt_firmware(unsigned char *data_out, int type)
{
    unsigned char data[BLOCK_SIZE];
    unsigned char index = type == VERSION_TL866A ? m_eraseA : m_eraseCS;
    const unsigned char* xortable = type == VERSION_TL866A ? Firmware::XortableA : Firmware::XortableCS;
    unsigned char* pEnc = type == A_KEY ? m_firmwareA : m_firmwareCS;

    //decrypt firmware

    for(int i=0;i<ENCRYPTED_FIRMWARE_SIZE;i+=BLOCK_SIZE)
    {
        memcpy(data,pEnc+i,BLOCK_SIZE);
        decrypt_block(data,xortable,index);
        memcpy(data_out,data,BLOCK_SIZE-16);
        data_out+=BLOCK_SIZE-16;
        index+=4;
        index&=0xFF;
    }
}

//encrypt a block of 80 bytes
void Firmware::encrypt_block(unsigned char *data, const unsigned char *xortable, unsigned char index)
{
    //First step, fill the last 16 bytes of data buffer with random generated values.
    for(int i=0;i<16;i++){
        data[i+64]=static_cast<unsigned char>(qrand() % 0x100);
    }

    /* Second step, data scrambling. We swap the first byte with the last, the fourth from the beginning with the fourth from the end and so on.
    So, we have the following 10 swaps:(0-79),(4-75),(8-71),(12-67),(16-63),(20-59),(24-55),(28-51),(32-47),(36-43).
    */
    for(int i=0;i<BLOCK_SIZE/2;i+=4){
        unsigned char t = data[i];
        data[i]=data[BLOCK_SIZE-i-1];
        data[BLOCK_SIZE-i-1]=t;
    }
    //Next step, left shifting whole array by 3 bits.
    for(int i=0;i<BLOCK_SIZE-1;i++){
        data[i] = ((data[i] << 3) & 0xF8) | data[i + 1] >> 5;
    }
    data[BLOCK_SIZE-1] = (data[BLOCK_SIZE-1] << 3) & 0xF8;

    //Last step, xoring each data value with a random number from xortable. Index is incremented modulo 256
    for(int i=0;i<BLOCK_SIZE;i++){
        data[i]^=xortable[index++];
    }
}

//decrypt a block of 80 bytes
void Firmware::decrypt_block(unsigned char *data, const unsigned char *xortable, unsigned char index)
{
    //first step, xoring each element with a random value from xortable. Index is incremented modulo 256
    for(int i=0;i<BLOCK_SIZE;i++){
        data[i]^=xortable[index++];
    }

    //next step, right shifting whole array by 3 bits.
    for(int i=BLOCK_SIZE-1; i > 0; i--){
        data[i] = static_cast<unsigned char>((data[i] >> 3 & 0x1F) | data[i - 1] << 5);
    }
    data[0] = (data[0] >> 3) & 0x1F;

    //Last step, descrambling data; put each element in the right position. At the end we have the decrypted data block ;)
    for(int i=0;i<BLOCK_SIZE/2;i+=4){
        unsigned char t = data[i];
        data[i]=data[BLOCK_SIZE-i-1];
        data[BLOCK_SIZE-i-1] = t;
    }
}

//Encrypt 80 bytes data block containing dev code and serial
void Firmware::encrypt_serial(unsigned char *key, const unsigned char *firmware)
{
    unsigned char index;
    ushort crc16;
    CRC crc;
    for(int i=32;i<BLOCK_SIZE-2;i++)
    {
        key[i] = static_cast<unsigned char>(qrand() % 0x100);
    }
    key[34] = 0;
    for (int i = 5; i < 34; i++)
        key[34] += key[i];
    crc16 = crc.crc16(key,BLOCK_SIZE-2, 0);
    key[BLOCK_SIZE-2]=(crc16 & 0xff);
    key[BLOCK_SIZE-1]=(crc16 >> 8);


    /*Step 1
    *Data scrambling. We swap the first byte with the last, the fourth from the beginning with the fourth from the end and so on.
     So we have the following 10 swaps:(0-79),(4-75),(8-71),(12-67),(16-63),(20-59),(24-55),(28-51),(32-47),(36-43).
    */
    for(int i=0;i<BLOCK_SIZE/2;i+=4){
        index=key[i];
        key[i]=key[BLOCK_SIZE-i-1];
        key[BLOCK_SIZE-i-1]=index;
    }
    //Step 2 left rotate the whole array by 3 bits.
    index = key[0] >> 5;
    for(int i=0;i<BLOCK_SIZE-1;i++)
        key[i] = (key[i] << 3 & 0xF8) | key[i + 1] >> 5;

    key[BLOCK_SIZE-1] = (key[BLOCK_SIZE-1] << 3 & 0xF8) | index;

    //Last step, xoring each info table value with a random number from xortable. The start index in this table is 0x0A. Index is incremented modulo 256
    index=0x0A;
    for(int i=0;i<BLOCK_SIZE;i++){
        key[i]^=firmware[XOR_TABLE_OFFSET+index++];
    }


}

//Decrypt 80 bytes data block containing dev code and serial
void Firmware::decrypt_serial(unsigned char *key, const unsigned char *firmware)
{
    unsigned char index;
    //first step, xoring each element from table with a random value from xortable. Starting index is 0x0A. Index is incremented modulo 256
    index=0x0A;
    for(int i=0;i<BLOCK_SIZE;i++){
        key[i]^=firmware[XOR_TABLE_OFFSET+index++];
    }

    //Step 2 right rotate the whole array by 3 bits.
    index = static_cast<unsigned char>(key[BLOCK_SIZE-1] << 5);
    for (int i = BLOCK_SIZE-1; i > 0; i--)
        key[i] = static_cast<unsigned char>((key[i] >> 3 & 0x1F) | key[i - 1] << 5);
    key[0] = (key[0] >> 3 & 0x1F) | index;

    //Last step, descrambling data; we put each element in the right position. At the end we have the decrypted serial and devcode ;)
    for(int i=0;i<BLOCK_SIZE/2;i+=4){
        index=key[i];
        key[i]=key[BLOCK_SIZE-i-1];
        key[BLOCK_SIZE-i-1]=index;
    }
}

bool Firmware::IsBadCrc(const QString &devcode, const QString &serial)
{
    CRC crc;
    unsigned int crc32 = crc.crc32(reinterpret_cast<uchar*>(serial.toLatin1().data()),24,
                          crc.crc32(reinterpret_cast<uchar*>(devcode.toLatin1().data()), 8, 0xFFFFFFFF));
    return (crc32 == BAD_CRC);

}
