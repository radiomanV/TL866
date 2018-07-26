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
    if(file.read((char*)&upd,sizeof(upd))!=UPDATE_DAT_SIZE)
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
    unsigned int sa = (ta[FIRMWARE_SIGNATURE_OFFSET+3] << 24) | (ta[FIRMWARE_SIGNATURE_OFFSET+2] << 16) | (ta[FIRMWARE_SIGNATURE_OFFSET+1] << 8) | ta[FIRMWARE_SIGNATURE_OFFSET];
    unsigned int scs = (tcs[FIRMWARE_SIGNATURE_OFFSET+3] << 24) | (tcs[FIRMWARE_SIGNATURE_OFFSET+2] << 16) | (tcs[FIRMWARE_SIGNATURE_OFFSET+1] << 8) | tcs[FIRMWARE_SIGNATURE_OFFSET];
    if(sa != (unsigned int)FIRMWARE_SIGNATURE || scs != (unsigned int)FIRMWARE_SIGNATURE)
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
    int index = key == VERSION_TL866A ? m_eraseA : m_eraseCS;
    unsigned char* xortable = key == VERSION_TL866A ? m_xortableA : m_xortableCS;

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
    int index = type == VERSION_TL866A ? m_eraseA : m_eraseCS;
    unsigned char* xortable = type == VERSION_TL866A ? m_xortableA : m_xortableCS;
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
void Firmware::encrypt_block(unsigned char *data, unsigned char *xortable, int index)
{
    unsigned char o1,o2;
    //First step, fill the last 16 bytes of data buffer with random generated values.
    for(int i=0;i<16;i++){
        data[i+64]=(unsigned char) qrand() % 0x100;
    }

    /* Second step, data scrambling. We swap the first byte with the last, the fourth from the beginning with the fourth from the end and so on.
    So, we have the following 10 swaps:(0-79),(4-75),(8-71),(12-67),(16-63),(20-59),(24-55),(28-51),(32-47),(36-43).
    */
    for(int i=0;i<BLOCK_SIZE/2;i+=4){
        o1=data[i];
        data[i]=data[BLOCK_SIZE-i-1];
        data[BLOCK_SIZE-i-1]=o1;
    }
    //Next step, left shifting whole array by 3 bits.
    for(int i=0;i<BLOCK_SIZE-1;i++){
        o1=(data[i]<<3) & 0xF8;
        o2=data[i+1]>>5;
        data[i]=o2 | o1;
    }
    data[BLOCK_SIZE-1]<<=3;
    data[BLOCK_SIZE-1]&=0xF8;

    //Last step, xoring each data value with a random number from xortable. Index is incremented modulo 256
    for(int i=0;i<BLOCK_SIZE;i++){
        data[i]^=xortable[index++];
        index&=0xFF;
    }
}

//decrypt a block of 80 bytes
void Firmware::decrypt_block(unsigned char *data, unsigned char *xortable, int index)
{
    unsigned char o1,o2;
    //first step, xoring each element with a random value from xortable. Index is incremented modulo 256
    for(int i=0;i<BLOCK_SIZE;i++){
        data[i]^=xortable[index++];
        index&=0xFF;
    }

    //next step, right shifting whole array by 3 bits.
    for(int i=0;i<BLOCK_SIZE-1;i++){
        o1=(data[BLOCK_SIZE-i-1]>>3) & 0x1F;
        o2=data[BLOCK_SIZE-i-2]<<5;
        data[BLOCK_SIZE-i-1]=o1 | o2;
    }
    data[0]>>=3;
    data[0]&=0x1F;

    //Last step, descrambling data; put each element in the right position. At the end we have the decrypted data block ;)
    for(int i=0;i<BLOCK_SIZE/2;i+=4){
        o1=data[i];
        data[i]=data[BLOCK_SIZE-i-1];
        data[BLOCK_SIZE-i-1]=o1;
    }
}

//Encrypt 80 bytes data block containing dev code and serial
void Firmware::encrypt_serial(unsigned char *key, const unsigned char *firmware)
{
    int index=0x0A;
    unsigned char o1,o2;
    ushort crc16;
    CRC crc;
    //compute the right crc16. The last two bytes in the info table is the crc16 in little-endian order and must be max. 0x1FFF, otherwise the decryption will be wrong.
    //a little brute-force method to match the required CRC;
    do
    {
        for(int i=32;i<BLOCK_SIZE-2;i++)
        {
            key[i] = (unsigned char) (qrand() % 0x100);
        }
        key[34] = 0;
        for (int i = 5; i < 34; i++)
            key[34] += key[i];
        crc16 = crc.crc16(key,BLOCK_SIZE-2, 0);
    }while(crc16 > 0x1FFF);
    key[BLOCK_SIZE-2]=(crc16 & 0xff);
    key[BLOCK_SIZE-1]=(crc16 >> 8);


    /*Data scrambling. We swap the first byte with the last, the fourth from the beginning with the fourth from the end and so on.
     So we have the following 10 swaps:(0-79),(4-75),(8-71),(12-67),(16-63),(20-59),(24-55),(28-51),(32-47),(36-43).
    */
    for(int i=0;i<BLOCK_SIZE/2;i+=4){
        o1=key[i];
        key[i]=key[BLOCK_SIZE-i-1];
        key[BLOCK_SIZE-i-1]=o1;
    }
    //Next step, left shift whole array by 3 bits .
    for(int i=0;i<BLOCK_SIZE-1;i++){
        o1=(key[i]<<3) & 0xF8;
        o2=key[i+1]>>5;
        key[i]=o2 | o1;
    }
    key[BLOCK_SIZE-1]<<=3;
    key[BLOCK_SIZE-1]&=0xF8;

    //Last step, xoring each info table value with a random number from xortable. The start index in this table is 0x0A. Index is incremented modulo 256
    for(int i=0;i<BLOCK_SIZE;i++){
        key[i]^=firmware[XOR_TABLE_OFFSET+index];
        index++;
        index&=0xFF;
    }


}

//Decrypt 80 bytes data block containing dev code and serial
void Firmware::decrypt_serial(unsigned char *key, const unsigned char *firmware)
{
    int index=0x0A;
    unsigned char o1,o2;

    //first step, xoring each element from table with a random value from xortable. Starting index is 0x0A. Index is incremented modulo 256
    for(int i=0;i<BLOCK_SIZE;i++){
        key[i]^=firmware[XOR_TABLE_OFFSET+index];
        index++;
        index&=0xFF;
    }

    /*next step, right shift whole array by 3 bits. Because anding with 0x1F, the last byte from info table must be always <0x20 in the encryption step, greater values will be trimmed at decryption step;
     this is why the crc16 must be 0x1FFF max., the last byte from info table is MSB of crc16.
     */
    for(int i=0;i<BLOCK_SIZE-1;i++){
        o1=(key[BLOCK_SIZE-i-1]>>3) & 0x1F;
        o2=key[BLOCK_SIZE-i-2]<<5;
        key[BLOCK_SIZE-i-1]=o1 | o2;
    }
    key[0]>>=3;
    key[0]&=0x1F;

    //Last step, descrambling data; we put each element in the right position. At the end we have the decrypted serial and devcode ;)
    for(int i=0;i<BLOCK_SIZE/2;i+=4){
        o1=key[i];
        key[i]=key[BLOCK_SIZE-i-1];
        key[BLOCK_SIZE-i-1]=o1;
    }
}

bool Firmware::IsBadCrc(uchar *devcode, uchar *serial)
{
    CRC crc;
    unsigned int crc32 = crc.crc32(serial,24,crc.crc32(devcode, 8, 0xFFFFFFFF));
    return (crc32 == BAD_CRC);

}
