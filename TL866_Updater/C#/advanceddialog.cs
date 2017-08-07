using System;
using System.Text;
using System.Windows.Forms;

namespace TL866
{
    public partial class AdvancedDialog : Form
    {
        public event EventHandler WriteBootloader;
        public event EventHandler WriteConfig;
        public event EventHandler WriteInfo;
        public event EventHandler GetInfo;

        public AdvancedDialog()
        {
            InitializeComponent();
        }

        private void OK_Button_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.OK;
            Close();
        }


        //Write bootloader
        private void BtnWriteBootloader_Click(object sender, EventArgs e)
        {
            if (WriteBootloader != null)
                WriteBootloader(this, null);
        }


        //Write config
        private void BtnWriteConfig_Click(object sender, EventArgs e)
        {
            if (WriteConfig != null)
                WriteConfig(this, null);
        }

        //write info
        private void BtnWriteInfo_Click(object sender, EventArgs e)
        {
            if (WriteInfo != null)
                WriteInfo(this, null);   
        }


        private void BtnClone_Click(object sender, EventArgs e)
        {
            if (GetInfo != null)
                GetInfo(this, null);
        }

        private void BtnDefault_Click(object sender, EventArgs e)
        {
            TxtDevcode.Text = "00000000";
            TxtSerial.Text = "000000000000000000000000";
        }

        private void BtnEdit_Click(object sender, EventArgs e)
        {
            EditDialog dlg = new EditDialog();
            dlg.TxtDevcode.Text = TxtDevcode.Text;
            dlg.TxtSerial.Text = TxtSerial.Text;
            if (dlg.ShowDialog(this) == DialogResult.OK)
            {
                TxtDevcode.Text = dlg.TxtDevcode.Text;
                TxtSerial.Text = dlg.TxtSerial.Text;
            }
        }
    }
}