using System;
using System.IO;
namespace TL866
{

    public class hexwriter
    {

        public void WriteHex(byte[] buffer, StreamWriter fstream)
        {
            int bl = 0;
            fstream.WriteLine(GetLine(2, 0, 4, new byte[] { 0, 0 }));
            for (int i = 0; i <= buffer.Length - 1; i += 16)
            {
                bl += 16;
                fstream.WriteLine(GetLine((buffer.Length - (i + 16) == 0 ? 8 : 16), i, 0, buffer));
                if ((bl % 0x10000) == 0)
                {
                    if (buffer.Length - (i + 16) != 0)
                        fstream.WriteLine(GetLine(2, 0, 4, new byte[] { 0, 1 }));
                }
            }
            fstream.WriteLine(GetLine(8, 0x1fff8, 0, buffer));
            fstream.WriteLine(GetLine(0, 0, 1, null));
        }


        private string GetLine(int lenght, int address, int recordtype, byte[] buffer)
        {
            string s = "";
            int cs = 0;
            s = ":" + (lenght & 0xff).ToString("X2") + (address & 0xffff).ToString("X4") + (recordtype & 0xff).ToString("X2");
            cs += lenght;
            cs += ((address & 0xffff) >> 8) & 0xff;
            cs += (address & 0xff);
            cs += (recordtype & 0xff);
            if (buffer != null)
            {
                for (int k = 0; k <= lenght - 1; k++)
                {
                    s += buffer[address + k].ToString("X2");
                    cs += buffer[address + k];
                }
            }
            cs = cs & 255;
            cs = cs ^ 255;
            cs = cs + 1;
            cs = cs % 256;
            s += cs.ToString("X2");
            return s;
        }
    }
}
