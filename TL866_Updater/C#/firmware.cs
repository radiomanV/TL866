using System;
using System.IO;
using System.Text;

namespace TL866
{
    public class Firmware
    {
        public enum BOOTLOADER_TYPE
        {
            A_BOOTLOADER = 1,
            CS_BOOTLOADER = 2
        }

        public enum DEVICE_STATUS
        {
            NORMAL_MODE = 1,
            BOOTLOADER_MODE = 2
        }

        public enum FIRMWARE_TYPE
        {
            FIRMWARE_A,
            FIRMWARE_CS,
            FIRMWARE_CUSTOM
        }

        public enum PROGRAMMER_TYPE
        {
            TL866A,
            TL866CS
        }

        public const int UPDATE_DAT_SIZE = 312348;
        public const int BLOCK_SIZE = 0x50;
        public const int XOR_TABLE_SIZE = 0x100;
        public const int XOR_TABLE_OFFSET = 0x1FC00;

        public const int SERIAL_OFFSET = 0x1FD00;
        public const int FLASH_SIZE = 0x20000;
        public const int BOOTLOADER_SIZE = 0x1800;
        public const int ENCRYPTED_FIRMWARE_SIZE = 0x25D00;
        public const int UNENCRYPTED_FIRMWARE_SIZE = 0x1E400;
        public const int FIRMWARE_SIGNATURE_OFFSET = 0x1E3FC;
        public const uint FIRMWARE_SIGNATURE = 0x5AA5AA55;
        public const uint CP0_ADDRESS = 0x1FFF9;
        public const int DEVCODE_LENGHT = 8;
        public const int SERIALCODE_LENGHT = 24;

        public const uint A_BOOTLOADER_CRC = 0x1B8960EF;
        public const uint CS_BOOTLOADER_CRC = 0xFB3DED05;
        public const uint BAD_CRC = 0xC8C2F013;


        public const byte WRITE_COMMAND = 0xAA;
        public const byte ERASE_COMMAND = 0xCC;
        public const byte RESET_COMMAND = 0xFF;
        public const byte REPORT_COMMAND = 0x00;

        public const byte DUMPER_READ_FLASH = 0x01;
        public const byte DUMPER_WRITE_BOOTLOADER = 0x02;
        public const byte DUMPER_WRITE_CONFIG = 0x03;
        public const byte DUMPER_WRITE_INFO = 0x04;
        public const byte DUMPER_INFO = 0x05;

        private byte m_eraseA;
        private byte m_eraseCS;
        private byte[] m_firmwareA;
        private byte[] m_firmwareCS;

        public byte Version { get; private set; }
        public bool IsValid { get; private set; }


        private readonly byte[] m_xortableA = {
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

        private readonly byte[] m_xortableCS = {
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

        public void Open(string UpdateDat_Path)
        {
            IsValid = false;
            FileStream fsin = null;
            try
            {
                fsin = File.OpenRead(UpdateDat_Path);
            }
            catch
            {
                if (fsin != null)
                    fsin.Close();
                throw new Exception("Error opening file " + UpdateDat_Path);
            }
            if (fsin.Length != UPDATE_DAT_SIZE)
            {
                fsin.Close();
                throw new Exception(UpdateDat_Path + "\nFile size error!");
            }
            byte[] inbuffer = new byte[fsin.Length + 1];
            fsin.Read(inbuffer, 0, (int)fsin.Length);
            fsin.Close();

            m_eraseA = inbuffer[9];
            m_eraseCS = inbuffer[17];
            Version = inbuffer[0];

            //Decrypt firmwares (stage 1)
            m_firmwareA = new byte[ENCRYPTED_FIRMWARE_SIZE];
            m_firmwareCS = new byte[ENCRYPTED_FIRMWARE_SIZE];
            for (uint i = 0; i < ENCRYPTED_FIRMWARE_SIZE; i++)
            {
                m_firmwareA[i] = (byte)(inbuffer[0xA1C + i] ^
                                         inbuffer[
                                             0x118 +
                                             ((BitConverter.ToInt32(inbuffer, 0x14) + i) & 0x3FF)] ^
                                         inbuffer[0x18 + ((i / 80) & 0xFF)]);
                m_firmwareCS[i] = (byte)(inbuffer[0x2671C + i] ^
                                          inbuffer[
                                              0x61C +
                                              ((BitConverter.ToInt32(inbuffer, 0x518) + i) & 0x3FF)] ^
                                          inbuffer[0x51C + ((i / 80) & 0xFF)]);
            }

            //Check if decryption was O.K.
            CRC32 crc32 = new CRC32();
            uint crca = ~crc32.GetCRC32(m_firmwareA, 0xFFFFFFFF);
            uint crccs = ~crc32.GetCRC32(m_firmwareCS, 0xFFFFFFFF);
            if (crca != BitConverter.ToUInt32(inbuffer, 4) || crccs != BitConverter.ToUInt32(inbuffer, 12))
                throw new Exception(UpdateDat_Path + "\nData CRC error!");
            //Check if decrypted firmware signatures are O.K.
            if (BitConverter.ToUInt32(GetUnencryptedFirmware((int)FIRMWARE_TYPE.FIRMWARE_A),
                    FIRMWARE_SIGNATURE_OFFSET) != FIRMWARE_SIGNATURE ||
                BitConverter.ToUInt32(GetUnencryptedFirmware((int)FIRMWARE_TYPE.FIRMWARE_CS),
                    FIRMWARE_SIGNATURE_OFFSET) != FIRMWARE_SIGNATURE)
                throw new Exception("Firmware decryption error!");
            IsValid = true;
        }


        public byte GetEraseParametter(int type)
        {
            return type == (int)PROGRAMMER_TYPE.TL866A ? m_eraseA : m_eraseCS;
        }


        public byte[] GetEncryptedFirmware(int type, int key)
        {
            byte[] buffer = new byte[ENCRYPTED_FIRMWARE_SIZE];
            if (type == key)
            {
                Array.Copy(type == (int)FIRMWARE_TYPE.FIRMWARE_A ? m_firmwareA : m_firmwareCS, buffer, buffer.Length);
                return buffer;
            }
            Array.Copy(Encrypt_Firmware(GetUnencryptedFirmware(type), key), buffer, buffer.Length);
            return buffer;
        }


        public byte[] GetUnencryptedFirmware(int type)
        {
            byte[] data = new byte[UNENCRYPTED_FIRMWARE_SIZE];
            byte[] buffer = new byte[BLOCK_SIZE];
            byte index = (byte)(type == (int)FIRMWARE_TYPE.FIRMWARE_A ? m_eraseA : m_eraseCS);
            int block = 0;
            //Decrypt each data block
            for (uint i = 0; i < ENCRYPTED_FIRMWARE_SIZE; i += BLOCK_SIZE)
            {
                Array.Copy(type == (int)FIRMWARE_TYPE.FIRMWARE_A ? m_firmwareA : m_firmwareCS, i, buffer, 0, BLOCK_SIZE);
                Decrypt_Block(buffer, type == (int)FIRMWARE_TYPE.FIRMWARE_A ? m_xortableA : m_xortableCS, index);
                Array.Copy(buffer, 0, data, block, BLOCK_SIZE - 16);
                block += BLOCK_SIZE - 16;
                index += 4;
            }
            return data;
        }

        public byte[] Encrypt_Firmware(byte[] firmware, int type)
        {
            byte[] data = new byte[ENCRYPTED_FIRMWARE_SIZE];
            byte[] buffer = new byte[BLOCK_SIZE];
            byte index = (byte)(type == (int)FIRMWARE_TYPE.FIRMWARE_A ? m_eraseA : m_eraseCS);
            int block = 0;
            //Encrypt each data block
            for (uint i = 0; i < firmware.Length; i += BLOCK_SIZE - 16)
            {
                Array.Copy(firmware, i, buffer, 0, BLOCK_SIZE - 16);
                Encrypt_Block(buffer, type == (int)FIRMWARE_TYPE.FIRMWARE_A ? m_xortableA : m_xortableCS, index);
                Array.Copy(buffer, 0, data, block, BLOCK_SIZE);
                block += BLOCK_SIZE;
                index += 4;
            }
            return data;
        }

        private void Encrypt_Block(byte[] data, byte[] xortable, byte index)
        {
            for (int i = data.Length - 16; i < data.Length; i++)
                data[i] = (byte)Utils.Generator.Next(0, 0xFF);
            //step1: swap data bytes
            for (uint i = 0; i < data.Length / 2; i += 4)
            {
                byte t = data[i];
                data[i] = data[data.Length - i - 1];
                data[data.Length - i - 1] = t;
            }
            //step2: shift the whole array three bits left
            for (uint i = 0; i < data.Length - 1; i++)
                data[i] = (byte)(((data[i] << 3) & 0xF8) | (data[i + 1] >> 5));
            data[data.Length - 1] = (byte)((data[data.Length - 1] << 3) & 0xF8);
            //step3: xor
            for (uint i = 0; i < data.Length; i++)
                data[i] = (byte)(data[i] ^ xortable[index++]);
        }


        private void Decrypt_Block(byte[] data, byte[] xortable, byte index)
        {
            //step1: xor
            for (uint i = 0; i < data.Length; i++)
                data[i] = (byte)(data[i] ^ xortable[index++]);
            //step2: shift the whole array three bits right
            for (int i = data.Length - 1; i > 0; i--)
                data[i] = (byte)(((data[i] >> 3) & 0x1F) | (data[i - 1] << 5));
            data[0] = (byte)((data[0] >> 3) & 0x1F);
            //step3: swap data bytes
            for (uint i = 0; i < data.Length / 2; i += 4)
            {
                byte t = data[i];
                data[i] = data[data.Length - i - 1];
                data[data.Length - i - 1] = t;
            }
        }


        public void DecryptSerial(byte[] info, byte[] firmware)
        {
            byte index = 0x0A;
            //Step1 xoring each element from table with a random value from xortable. Starting index is 0x0A. Index is incremented modulo 256
            for (uint i = 0; i < info.Length; i++)
                info[i] = (byte)(info[i] ^ firmware[XOR_TABLE_OFFSET + index++]);
            //Step 2 right rotate the whole array by 3 bits.
            byte m = (byte)(info[info.Length - 1] << 5);
            for (int i = info.Length - 1; i > 0; i--)
                info[i] = (byte)(info[i] >> 3 & 0x1F | info[i - 1] << 5);
            info[0] = (byte)(info[0] >> 3 & 0x1F | m);
            //Step3 descrambling data; we put each element in the right position. At the end we have the decrypted serial and devcode ;)
            for (int i = 0; i < info.Length / 2; i += 4)
            {
                byte t = info[i];
                info[i] = info[info.Length - i - 1];
                info[info.Length - i - 1] = t;
            }
        }

        public void EncryptSerial(byte[] info, byte[] firmware)
        {
            //Calculate the checksum and the CRC16 of the info block before the encryption.
            //Fill the empty info arrary with random values.
            for (int i = 32; i < info.Length - 2; i++)
                info[i] = (byte)Utils.Generator.Next(0, 255);
            //compute the 8Bit checksum of devcode and serial string and store it in the position 34.
            byte cs = 0;
            for (int i = 5; i < 34; i++)
                cs += info[i];
            info[34] = cs;
            //compute the CRC16 of the info array and store it in the last two position in little endian order.
            ushort crc;
            CRC16 crc16 = new CRC16();
            crc = crc16.GetCRC16(info, info.Length - 2, 0);
            info[info.Length - 1] = (byte)((crc >> 8));
            info[info.Length - 2] = (byte)(crc & 0xFF);

            //Now we encrypt the info block, ready to be inserted in the firmware.
            /*Step1
             * Data scrambling. We swap the first byte with the last, the fourth from the beginning with the fourth from the end and so on.
             * So we have the following 10 swaps:(0-79),(4-75),(8-71),(12-67),(16-63),(20-59),(24-55),(28-51),(32-47),(36-43).
             */
            for (int i = 0; i < info.Length / 2; i += 4)
            {
                byte t = info[i];
                info[i] = info[info.Length - i - 1];
                info[info.Length - i - 1] = t;
            }
            //Step 2, Left rotate the whole array by 3 bits.
            byte m = (byte)(info[0] >> 5);
            for (int i = 0; i < info.Length - 1; i++)
                info[i] = (byte)(info[i] << 3 & 0xF8 | info[i + 1] >> 5);
            info[info.Length - 1] = (byte)(info[info.Length - 1] << 3 & 0xF8 | m);
            //step3 xoring each info table value with a random number from xortable. The start index in this table is 0x0A. Index is incremented modulo 256
            byte index = 0x0A;
            for (int i = 0; i < info.Length; i++)
                info[i] = (byte)(info[i] ^ firmware[XOR_TABLE_OFFSET + index++]);
        }


        public static bool Calc_CRC(string DevCode, string Serial)
        {
            byte[] k = new byte[DEVCODE_LENGHT + SERIALCODE_LENGHT];
            Array.Copy(Encoding.ASCII.GetBytes(DevCode + new string(' ', DEVCODE_LENGHT - DevCode.Length)), 0, k, 0,
                DEVCODE_LENGHT);
            Array.Copy(Encoding.ASCII.GetBytes(Serial + new string(' ', SERIALCODE_LENGHT - Serial.Length)), 0, k,
                DEVCODE_LENGHT, SERIALCODE_LENGHT);
            CRC32 crc = new CRC32();
            return crc.GetCRC32(k, 0xFFFFFFFF) == BAD_CRC;
        }
    }


    public class TL866_Report
    {
        public byte[] buffer = new byte[64];

        public byte Device_Version
        {
            get { return buffer[6]; }
        }

        public byte Device_Status
        {
            get { return buffer[1]; }
        }

        public string DeviceCode
        {
            get { return Encoding.ASCII.GetString(buffer, 7, Firmware.DEVCODE_LENGHT).Trim(); }
        }

        public string SerialCode
        {
            get { return Encoding.ASCII.GetString(buffer, 15, Firmware.SERIALCODE_LENGHT).Trim(); }
        }

        public byte firmware_version_minor
        {
            get { return buffer[4]; }
        }

        public byte firmware_version_major
        {
            get { return buffer[5]; }
        }

        public byte hardware_version
        {
            get { return buffer[39]; }
        }
        public byte b0
        {
            get { return buffer[40]; }
        }

        public byte b1
        {
            get { return buffer[41]; }
        }

        public byte checksum
        {
            get { return buffer[42]; }
        }

        public byte bad_serial
        {
            get { return buffer[43]; }
        }
    }

    public class Dumper_Report
    {
        public byte[] buffer = new byte[34];

        public string DeviceCode
        {
            get { return Encoding.ASCII.GetString(buffer, 0, Firmware.DEVCODE_LENGHT).Trim(); }
        }

        public string SerialCode
        {
            get { return Encoding.ASCII.GetString(buffer, Firmware.DEVCODE_LENGHT, Firmware.SERIALCODE_LENGHT).Trim(); }
        }

        public byte bootloader_version
        {
            get { return buffer[32]; }
        }

        public byte cp_bit
        {
            get { return buffer[33]; }
        }
    }
}