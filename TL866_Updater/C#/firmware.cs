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

        public enum DEVICE_VERSION
        {
            VERSION_TL866A = 1,
            VERSION_TL866CS = 2
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
        public const int XOR_TABLE_START = 0x1EEDF;
        public const int XOR_TABLE_OFFSET = 0x1FC00;

        public const int SERIAL_OFFSET = 0x1FD00;
        public const int FLASH_SIZE = 0x20000;
        public const int BOOTLOADER_SIZE = 0x1800;
        public const int ENCRYPTED_FIRMWARE_SIZE = 0x25D00;
        public const int UNENCRYPTED_FIRMWARE_SIZE = 0x1E400;
        public const int FIRMWARE_SIGNATURE_OFFSET = 0x1E3FC;
        public const uint FIRMWARE_SIGNATURE = 0x5AA5AA55;
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
        private byte[] m_xortableA;
        private byte[] m_xortableCS;


        public byte Version { get; private set; }
        public bool IsValid { get; private set; }


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
            fsin.Read(inbuffer, 0, (int) fsin.Length);
            fsin.Close();

            m_eraseA = inbuffer[9];
            m_eraseCS = inbuffer[17];
            Version = inbuffer[0];

            //Decrypt firmwares (stage 1)
            m_firmwareA = new byte[ENCRYPTED_FIRMWARE_SIZE];
            m_firmwareCS = new byte[ENCRYPTED_FIRMWARE_SIZE];
            for (uint i = 0; i < ENCRYPTED_FIRMWARE_SIZE; i++)
            {
                m_firmwareA[i] = (byte) (inbuffer[0xA1C + i] ^
                                         inbuffer[
                                             0x118 +
                                             ((BitConverter.ToInt32(inbuffer, 0x14) + i) & 0x3FF)] ^
                                         inbuffer[0x18 + ((i / 80) & 0xFF)]);
                m_firmwareCS[i] = (byte) (inbuffer[0x2671C + i] ^
                                          inbuffer[
                                              0x61C +
                                              ((BitConverter.ToInt32(inbuffer, 0x518) + i) & 0x3FF)] ^
                                          inbuffer[0x51C + ((i / 80) & 0xFF)]);
            }

            //extract xor table arrays
            byte[] ta = new byte[ENCRYPTED_FIRMWARE_SIZE];
            byte[] tcs = new byte[ENCRYPTED_FIRMWARE_SIZE];
            Array.Copy(m_firmwareA, ta, ENCRYPTED_FIRMWARE_SIZE);
            Array.Copy(m_firmwareCS, tcs, ENCRYPTED_FIRMWARE_SIZE);
            m_xortableA = new byte[XOR_TABLE_SIZE];
            m_xortableCS = new byte[XOR_TABLE_SIZE];
            for (uint i = 0; i < ENCRYPTED_FIRMWARE_SIZE; i++)
            {
                ta[i] = (byte) ~ta[i];
                tcs[i] = (byte) ~tcs[i];
            }
            for (uint i = 0; i < 16; i++)
            {
                Array.Copy(ta, XOR_TABLE_START + i * 320, m_xortableA, i * 16, 16);
                Array.Copy(tcs, XOR_TABLE_START + i * 320, m_xortableCS, i * 16, 16);
            }

            //Check if decryption was O.K.
            CRC32 crc32 = new CRC32();
            uint crca = ~crc32.GetCRC32(m_firmwareA, 0xFFFFFFFF);
            uint crccs = ~crc32.GetCRC32(m_firmwareCS, 0xFFFFFFFF);
            if (crca != BitConverter.ToUInt32(inbuffer, 4) || crccs != BitConverter.ToUInt32(inbuffer, 12))
                throw new Exception(UpdateDat_Path + "\nData CRC error!");

            //Check if decrypted firmware signatures are O.K.
            if (BitConverter.ToUInt32(GetUnencryptedFirmware((int) FIRMWARE_TYPE.FIRMWARE_A),
                    FIRMWARE_SIGNATURE_OFFSET) != FIRMWARE_SIGNATURE ||
                BitConverter.ToUInt32(GetUnencryptedFirmware((int) FIRMWARE_TYPE.FIRMWARE_CS),
                    FIRMWARE_SIGNATURE_OFFSET) != FIRMWARE_SIGNATURE)
                throw new Exception("Firmware decryption error!");
            IsValid = true;
        }


        public byte GetEraseParametter(int type)
        {
            return type == (int) PROGRAMMER_TYPE.TL866A ? m_eraseA : m_eraseCS;
        }


        public byte[] GetEncryptedFirmware(int type, int key)
        {
            byte[] buffer = new byte[ENCRYPTED_FIRMWARE_SIZE];
            if (type == key)
            {
                Array.Copy(type == (int) FIRMWARE_TYPE.FIRMWARE_A ? m_firmwareA : m_firmwareCS, buffer, buffer.Length);
                return buffer;
            }
            Array.Copy(Encrypt_Firmware(GetUnencryptedFirmware(type), key), buffer, buffer.Length);
            return buffer;
        }


        public byte[] GetUnencryptedFirmware(int type)
        {
            byte[] data = new byte[UNENCRYPTED_FIRMWARE_SIZE];
            byte[] buffer = new byte[BLOCK_SIZE];
            byte index = 0x15;
            int block = 0;
            //Decrypt each data block
            for (uint i = 0; i < ENCRYPTED_FIRMWARE_SIZE; i += BLOCK_SIZE)
            {
                Array.Copy(type == (int) FIRMWARE_TYPE.FIRMWARE_A ? m_firmwareA : m_firmwareCS, i, buffer, 0,
                    BLOCK_SIZE);
                Decrypt_Block(buffer, type == (int) FIRMWARE_TYPE.FIRMWARE_A ? m_xortableA : m_xortableCS, index);
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
            byte index = 0x15;
            int block = 0;
            //Encrypt each data block
            for (uint i = 0; i < firmware.Length; i += BLOCK_SIZE - 16)
            {
                Array.Copy(firmware, i, buffer, 0, BLOCK_SIZE - 16);
                Encrypt_Block(buffer, type == (int) FIRMWARE_TYPE.FIRMWARE_A ? m_xortableA : m_xortableCS, index);
                Array.Copy(buffer, 0, data, block, BLOCK_SIZE);
                block += BLOCK_SIZE;
                index += 4;
            }
            return data;
        }

        private void Encrypt_Block(byte[] data, byte[] xortable, byte index)
        {
            for (int i = data.Length - 16; i < data.Length; i++)
                data[i] = (byte) Utils.Generator.Next(0, 0xFF);
            //step1: swap data bytes
            for (uint i = 0; i < data.Length / 2; i += 4)
            {
                byte t = data[i];
                data[i] = data[data.Length - i - 1];
                data[data.Length - i - 1] = t;
            }
            //step2: shift the whole array three bits left
            for (uint i = 0; i < data.Length - 1; i++)
                data[i] = (byte) (((data[i] << 3) & 0xF8) | (data[i + 1] >> 5));
            data[data.Length - 1] = (byte) ((data[data.Length - 1] << 3) & 0xF8);
            //step3: xor
            for (uint i = 0; i < data.Length; i++)
                data[i] = (byte) (data[i] ^ xortable[index++]);
        }


        private void Decrypt_Block(byte[] data, byte[] xortable, byte index)
        {
            //step1: xor
            for (uint i = 0; i < data.Length; i++)
                data[i] = (byte) (data[i] ^ xortable[index++]);
            //step2: shift the whole array three bits right
            for (int i = data.Length - 1; i > 0; i--)
                data[i] = (byte) (((data[i] >> 3) & 0x1F) | (data[i - 1] << 5));
            data[0] = (byte) ((data[0] >> 3) & 0x1F);
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
            //step1
            byte index = 0x0A;
            for (uint i = 0; i < info.Length; i++)
                info[i] = (byte) (info[i] ^ firmware[XOR_TABLE_OFFSET + index++]);
            //step2
            for (int i = info.Length - 1; i > 0; i--)
                info[i] = (byte) (((info[i] >> 3) & 0x1F) | (info[i - 1] << 5));
            info[0] = (byte) ((info[0] >> 3) & 0x1F);
            //step3
            for (int i = 0; i < info.Length / 2; i += 4)
            {
                byte t = info[i];
                info[i] = info[info.Length - i - 1];
                info[info.Length - i - 1] = t;
            }
        }

        public void EncryptSerial(byte[] info, byte[] firmware)
        {
            byte index = 0x0A;
            if (GetKeyCRC(info) != 0)
                Make_CRC(info);
            //step1
            for (int i = 0; i < info.Length / 2; i += 4)
            {
                byte t = info[i];
                info[i] = info[info.Length - i - 1];
                info[info.Length - i - 1] = t;
            }
            //step2
            for (int i = 0; i < info.Length - 1; i++)
                info[i] = (byte) (((info[i] << 3) & 0xF8) | (info[i + 1] >> 5));
            info[info.Length - 1] = (byte) ((info[info.Length - 1] << 3) & 0xF8);
            //step3
            for (int i = 0; i < info.Length; i++)
                info[i] = (byte) (info[i] ^ firmware[XOR_TABLE_OFFSET + index++]);
        }

        public ushort GetKeyCRC(byte[] data)
        {
            CRC16 crc16 = new CRC16();
            return crc16.GetCRC16(data, 0);
        }


        private void Make_CRC(byte[] data)
        {
            byte[] b = new byte[data.Length - 2];
            ushort crc;
            Array.Copy(data, 0, b, 0, b.Length);
            do
            {
                for (int i = 32; i < b.Length; i++)
                    b[i] = (byte) Utils.Generator.Next(0, 255);
                crc = GetKeyCRC(b);
            } while (!(crc < 0x2000));
            Array.Copy(b, 0, data, 0, b.Length);
            data[data.Length - 1] = (byte) (crc >> 8);
            data[data.Length - 2] = (byte) (crc & 0xFF);
        }

        public bool Calc_CRC(string DevCode, string Serial)
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