using System;
using System.Windows.Forms;

namespace TL866
{
    public partial class EditDialog : Form
    {


        private const uint BAD_CRC = 0xc8C2F013;

        public EditDialog()
        {
            InitializeComponent();
        }

        private void OK_Button_Click(System.Object sender, System.EventArgs e)
        {
            if ((txtDevcode.Text.Trim().ToLower()) == "codedump" && (txtSerial.Text.Trim()) == "000000000000000000000000")
            {
                MessageBox.Show( "Please enter another device and serial code!\nThese are reserved.", "TL866", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return;
            }
            if (Calc_CRC(txtDevcode.Text, txtSerial.Text))
            {
                MessageBox.Show( "Bad Device and serial code!", "TL866", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return;
            }
            this.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.Hide();
        }

        private void Cancel_Button_Click(System.Object sender, System.EventArgs e)
        {
            this.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.Hide();
        }

        private void btnRndDev_Click(System.Object sender, System.EventArgs e)
        {
            string s = "";
            do
            {
                s = Utils.Generator.Next(0, 99999999).ToString("00000000");
            } while (Calc_CRC(s, txtSerial.Text));
            txtDevcode.Text = s;
        }

        private void btnRndSer_Click(System.Object sender, System.EventArgs e)
        {
            string s = null;
            do
            {
                s = "";
                for (int i = 0; i <= 23; i++)
                {
                    s += (Utils.Generator.Next(0, 15).ToString("X"));
                }
            } while (Calc_CRC(txtDevcode.Text, s));
            txtSerial.Text = s;
        }

        public bool Calc_CRC(string DevCode, string Serial)
        {
            byte[] k = new byte[32];
            Array.Copy(System.Text.Encoding.ASCII.GetBytes(DevCode + new string(' ', 8 - DevCode.Length)), 0, k, 0, 8);
            Array.Copy(System.Text.Encoding.ASCII.GetBytes(Serial + new string(' ', 24 - Serial.Length)), 0, k, 8, 24);
            crc32 crc = new crc32();
            return crc.GetCRC32(k, 0xffffffffu) == BAD_CRC;
        }

        private void txtDevcode_TextChanged(System.Object sender, System.EventArgs e)
        {
            if (Calc_CRC(txtDevcode.Text, txtSerial.Text))
            {
                MessageBox.Show( "Bad Device and serial code!", "TL866", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
            }
        }




    }
}
