using System;
using System.Windows.Forms;
using System.Text;

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
            string s;
            CRC16 crc16 = new CRC16();
            ushort crc = crc16.GetCRC16(Encoding.ASCII.GetBytes(TxtDevcode.Text), 0);
            do
            {
                s = (crc >> 8).ToString("X2") + Utils.Generator.Next(0, 255).ToString("X2") + (crc & 0xFF).ToString("X2");
                for (int i = 6; i < 15; i++)
                    s += Utils.Generator.Next(0, 255).ToString("X2");
            } while (Firmware.Calc_CRC(TxtDevcode.Text, s));
            TxtSerial.Text = s;
        }


        private void TxtDevcode_TextChanged(object sender, EventArgs e)
        {
            if (Firmware.Calc_CRC(TxtDevcode.Text, TxtSerial.Text))
                MessageBox.Show("Bad Device and serial code!", "TL866", MessageBoxButtons.OK,
                    MessageBoxIcon.Exclamation);
        }
    }
}