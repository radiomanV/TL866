using System;
using System.IO;

namespace TL866
{
    public class HexWriter
    {
        private readonly StreamWriter fstream;

        public HexWriter(StreamWriter stream)
        {
            if (stream != null)
                fstream = stream;
            else
                throw new Exception();
        }

        public void WriteHex(byte[] buffer)
        {
            int bl = 0;
            fstream.WriteLine(GetLine(2, 0, 4, new byte[] {0, 0}));
            for (int i = 0; i < buffer.Length; i += 16)
            {
                bl += 16;
                fstream.WriteLine(GetLine(buffer.Length - (i + 16) == 0 ? 8 : 16, i, 0, buffer));
                if (bl % 0x10000 == 0 && buffer.Length - (i + 16) != 0)
                    fstream.WriteLine(GetLine(2, 0, 4, new byte[] {0, 1}));
            }
            fstream.WriteLine(GetLine(8, 0x1FFF8, 0, buffer));
            fstream.WriteLine(GetLine(0, 0, 1, null));
            fstream.Close();
        }


        private string GetLine(int lenght, int address, byte recordtype, byte[] buffer)
        {
            string s = string.Format(":{0:X2}{1:X4}{2:X2}", lenght & 0xff, address & 0xffff, recordtype & 0xff);
            byte cs = (byte) lenght;
            cs += (byte) (((address & 0xffff) >> 8) & 0xff);
            cs += (byte) (address & 0xff);
            cs += (byte) (recordtype & 0xff);
            if (buffer != null)
                for (int k = 0; k < lenght; k++)
                {
                    s += buffer[address + k].ToString("X2");
                    cs += buffer[address + k];
                }
            cs = (byte) (~cs + 1);
            s += cs.ToString("X2");
            return s;
        }
    }
}