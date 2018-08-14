using System;
using System.Windows.Forms;
using System.Text;
using System.Globalization;

namespace TL866
{
    public partial class EditDialog : Form
    {
        public EditDialog()
        {
            InitializeComponent();
        }

        private void OK_Button_Click(object sender, EventArgs e)
        {
            if (TxtDevcode.Text.Trim().ToLower() == "codedump" && TxtSerial.Text.Trim() == "000000000000000000000000")
            {
                MessageBox.Show("Please enter another device and serial code!\nThese are reserved.", "TL866",
                    MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return;
            }
            if (Firmware.Calc_CRC(TxtDevcode.Text, TxtSerial.Text))
            {
                MessageBox.Show("Bad Device and serial code!", "TL866", MessageBoxButtons.OK,
                    MessageBoxIcon.Exclamation);
                return;
            }
            DialogResult = DialogResult.OK;
            Close();
        }

        private void Cancel_Button_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.Cancel;
            Close();
        }

        private void BtnRndDev_Click(object sender, EventArgs e)
        {
            string s;
            do
            {
                s = Utils.Generator.Next(0, 99999999).ToString("00000000");
            } while (Firmware.Calc_CRC(s, TxtSerial.Text));
            TxtDevcode.Text = s;
            BtnRndSer_Click(null, null);
        }


        private void BtnRndSer_Click(object sender, EventArgs e)
        {
            if (TxtDevcode.Text == string.Empty)
                BtnRndDev_Click(null, null);
            string s;
            ushort crc = get_dev_crc();
            do
            {
                s = (crc >> 8).ToString("X2") + Utils.Generator.Next(0, 255).ToString("X2") + (crc & 0xFF).ToString("X2");
                for (int i = 0; i < 9; i++)
                    s += Utils.Generator.Next(0, 255).ToString("X2");
            } while (Firmware.Calc_CRC(TxtDevcode.Text, s));
            TxtSerial.Text = s;
        }

        ushort get_dev_crc()
        {
            CRC16 crc16 = new CRC16();
            return crc16.GetCRC16(Encoding.ASCII.GetBytes(TxtDevcode.Text), TxtDevcode.Text.Length, 0);
        }

        private void TxtDevcode_TextChanged(object sender, EventArgs e)
        {
            if (TxtDevcode.Text == string.Empty)
            { 
                TxtSerial.Text = string.Empty;
                return;
            }
            if(TxtDevcode.Text.Length > 0 && TxtSerial.Text.Length < 24)
                BtnRndSer_Click(null, null);
            if (TxtSerial.Text.Length > 5)
            {
                byte msb, lsb;
                if (byte.TryParse(TxtSerial.Text.Substring(0, 2), NumberStyles.HexNumber, CultureInfo.InvariantCulture, out msb) &&
                   byte.TryParse(TxtSerial.Text.Substring(4, 2), NumberStyles.HexNumber, CultureInfo.InvariantCulture, out lsb))
                {
                   ushort crcdev = Convert.ToUInt16(msb << 8 | lsb);
                    if (get_dev_crc() != crcdev)
                        BtnRndSer_Click(null, null);
                }
            }
        }
    }
}