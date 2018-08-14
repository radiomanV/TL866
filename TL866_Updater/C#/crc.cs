namespace TL866
{
    public class CRC32
    {
        private readonly uint[] table;


        public CRC32()
        {
            const uint poly = 0xEDB88320;
            table = new uint[256];
            for (uint i = 0; i < 256; i++)
            {
                uint temp = i;
                for (int j = 0; j < 8; j++)
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
                initial = (initial >> 8) ^ table[bytes[i] ^ initial & 0xFF];
            return initial;
        }
    }

    public class CRC16
    {
        private readonly ushort[] table;

        public CRC16()
        {
            const ushort poly = 0xA001;
            table = new ushort[256];
            for (ushort i = 0; i < 256; i++)
            {
                ushort temp = i;
                for (byte j = 0; j < 8; j++)
                    if ((temp & 1) == 1)
                        temp = (ushort)((temp >> 1) ^ poly);
                    else
                        temp >>= 1;
                table[i] = temp;
            }
        }

        public ushort GetCRC16(byte[] bytes, int length, ushort initial)
        {
            for (int i = 0; i < length; i++)
                initial = (ushort)((initial >> 8) ^ table[bytes[i] ^ initial & 0xFF]);
            return initial;
        }
    }
}