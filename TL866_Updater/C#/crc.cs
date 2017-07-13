using System;

namespace TL866
{
    public class crc32
    {
        private readonly uint[] table;


        public crc32()
        {
            const uint poly = 0xedb88320u;
            table = new uint[256];
            for (uint i = 0; i <= table.Length - 1; i++)
            {
                uint temp = i;
                for (int j = 8; j >= 1; j += -1)
                    if ((temp & 1) == 1)
                        temp = Convert.ToUInt32((temp >> 1) ^ poly);
                    else
                        temp >>= 1;
                table[i] = temp;
            }
        }

        public uint GetCRC32(byte[] bytes, uint initial)
        {
            for (int i = 0; i <= bytes.Length - 1; i++)
                initial = Convert.ToUInt32((initial >> 8) ^ table[Convert.ToByte((initial & 0xff) ^ bytes[i])]);
            return initial;
        }
    }

    public class Crc16
    {
        private readonly ushort[] table;

        public Crc16()
        {
            const ushort polynomial = 0xa001;
            table = new ushort[256];
            int r = 0;
            for (ushort i = 0; i <= table.Length - 1; i++)
            {
                ushort value = 0;
                ushort temp = i;
                for (byte j = 0; j <= 7; j++)
                {
                    if (((value ^ temp) & 0x1) != 0)
                        value = Convert.ToUInt16((value >> 1) ^ polynomial);
                    else
                        value >>= 1;
                    temp >>= 1;
                }
                table[i] = value;
                r += 1;
                r = r % 16;
            }
        }

        public ushort GetCRC16(byte[] bytes, ushort initial)
        {
            for (int i = 0; i <= bytes.Length - 1; i++)
                initial = Convert.ToUInt16((initial >> 8) ^ table[Convert.ToByte((initial ^ bytes[i]) & 0xff)]);
            return initial;
        }
    }
}