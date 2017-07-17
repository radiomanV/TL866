namespace TL866
{
    public class CRC32
    {
        private readonly uint[] table;


        public CRC32()
        {
            const uint poly = 0xEDB88320;
            table = new uint[256];
            for (uint i = 0; i < table.Length; i++)
            {
                uint temp = i;
                for (int j = 8; j > 0; j--)
                    if ((temp & 1) == 1)
                        temp = (temp >> 1) ^ poly;
                    else
                        temp >>= 1;
                table[i] = temp;
            }
        }

        public uint GetCRC32(byte[] bytes, uint initial)
        {
            for (int i = 0; i < bytes.Length; i++)
                initial = (initial >> 8) ^ table[(byte) ((initial & 0xFF) ^ bytes[i])];
            return initial;
        }
    }

    public class CRC16
    {
        private readonly ushort[] table;

        public CRC16()
        {
            const ushort polynomial = 0xA001;
            table = new ushort[256];
            for (ushort i = 0; i < table.Length; i++)
            {
                ushort value = 0;
                ushort temp = i;
                for (byte j = 0; j < 8; j++)
                {
                    if (((value ^ temp) & 0x1) != 0)
                        value = (ushort) ((value >> 1) ^ polynomial);
                    else
                        value >>= 1;
                    temp >>= 1;
                }
                table[i] = value;
            }
        }

        public ushort GetCRC16(byte[] bytes, ushort initial)
        {
            for (int i = 0; i < bytes.Length; i++)
                initial = (ushort) ((initial >> 8) ^ table[(byte) ((initial ^ bytes[i]) & 0xff)]);
            return initial;
        }
    }
}