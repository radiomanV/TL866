using System;
using System.Windows.Forms;

namespace TL866
{
    public partial class AdvancedDialog : Form
    {
        public AdvancedDialog()
        {
            InitializeComponent();
            txtDevcode.Text = devcode;
            txtSerial.Text = serial;
        }


        public string devcode;
        public string serial;


        private void OK_Button_Click(System.Object sender, System.EventArgs e)
        {
            this.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.Hide();
        }


        //Write bootloader
        private void btnWriteBootloader_Click(System.Object sender, System.EventArgs e)
        {
            byte[] b = new byte[9];
            if (GetMainForm().usb.Get_Devices().Count == 0)
            {
                MessageBox.Show( Utils.NO_DEVICE, "TL866", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return;
            }
            if (GetMainForm().usb.Get_Devices().Count > 1)
            {
                MessageBox.Show( Utils.MULTIPLE_DEVICES, "TL866", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return;
            }
            if (check())
            {
                if (GetMainForm().usb.OpenDevice(GetMainForm().usb.Get_Devices()[0]))
                {
                    if (MessageBox.Show(this, Utils.WARNING_BRICK, "TL866", MessageBoxButtons.YesNo, MessageBoxIcon.Exclamation) == DialogResult.Yes)
                    {
                        ushort crc = Bootloader_CRC();
                        if ((crc == Firmware.A_BOOTLOADER_CRC || crc == Firmware.CS_BOOTLOADER_CRC))
                        {
                            GetMainForm().usb.Write(new byte[] { Firmware.DUMPER_WRITE_BOOTLOADER, (byte)(radioA.Checked ? Firmware.BOOTLOADER_TYPE.A_BOOTLOADER : Firmware.BOOTLOADER_TYPE.CS_BOOTLOADER) });
                            GetMainForm().usb.Read(b);
                            GetMainForm().UsbDeviceChanged(0);
                            if (b[0] == Firmware.DUMPER_WRITE_BOOTLOADER)
                            {
                                MessageBox.Show( "Bootloader was successfully written.", "TL866", MessageBoxButtons.OK, MessageBoxIcon.Information);
                            }
                            else
                            {
                                MessageBox.Show( "Bootloader writing failed.", "TL866", MessageBoxButtons.OK, MessageBoxIcon.Error);
                            }
                        }
                        else
                        {
                            MessageBox.Show( "Bootloader CRC error!\nAs a safety measure, nothing will be written.", "TL866", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                        }
                    }
                }
            }
        }


        //Write config
        private void btnWriteConfig_Click(System.Object sender, System.EventArgs e)
        {
            byte[] b = new byte[9];
            if (GetMainForm().usb.Get_Devices().Count == 0)
            {
                MessageBox.Show( Utils.NO_DEVICE, "TL866", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return;
            }
            if (GetMainForm().usb.Get_Devices().Count > 1)
            {
                MessageBox.Show( Utils.MULTIPLE_DEVICES, "TL866", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return;
            }
            if (check())
            {
                if (GetMainForm().usb.OpenDevice(GetMainForm().usb.Get_Devices()[0]))
                {
                    if (MessageBox.Show(this, Utils.WARNING_BRICK, "TL866", MessageBoxButtons.YesNo, MessageBoxIcon.Exclamation) == DialogResult.Yes)
                    {
                        GetMainForm().usb.Write(new byte[] { Firmware.DUMPER_WRITE_CONFIG, (byte)(chkCP.Checked ? 1 : 0) });
                        GetMainForm().usb.Read(b);
                        GetMainForm().UsbDeviceChanged(0);
                        if (b[0] == Firmware.DUMPER_WRITE_CONFIG)
                        {
                            MessageBox.Show( "Code protection bit was successfully written.", "TL866", MessageBoxButtons.OK, MessageBoxIcon.Information);
                        }
                        else
                        {
                            MessageBox.Show( "Writing code protect bit failed.", "TL866", MessageBoxButtons.OK, MessageBoxIcon.Error);
                        }
                    }
                }
            }
        }

        //write info
        private void btnWriteInfo_Click(System.Object sender, System.EventArgs e)
        {
            byte[] b = new byte[34];
            b[0] = Firmware.DUMPER_WRITE_INFO;
            string s1 = txtDevcode.Text + new string(' ', 8 - txtDevcode.Text.Length);
            string s2 = txtSerial.Text + new string(' ', 24 - txtSerial.Text.Length);
            Array.Copy(System.Text.Encoding.ASCII.GetBytes(s1), 0, b, 1, 8);
            Array.Copy(System.Text.Encoding.ASCII.GetBytes(s2), 0, b, 9, 24);
            if (GetMainForm().usb.Get_Devices().Count == 0)
            {
                MessageBox.Show( Utils.NO_DEVICE, "TL866", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return;
            }
            if (GetMainForm().usb.Get_Devices().Count > 1)
            {
                MessageBox.Show( Utils.MULTIPLE_DEVICES, "TL866", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return;
            }
            if (check())
            {
                if (GetMainForm().usb.OpenDevice(GetMainForm().usb.Get_Devices()[0]))
                {
                    if (MessageBox.Show(this, Utils.WARNING_BRICK, "TL866", MessageBoxButtons.YesNo, MessageBoxIcon.Exclamation) == DialogResult.Yes)
                    {
                        GetMainForm().usb.Write(b);
                        GetMainForm().usb.Read(b);
                        GetMainForm().UsbDeviceChanged(0);
                        if (b[0] == 4)
                        {
                            MessageBox.Show( "Device info was successfully written.", "TL866", MessageBoxButtons.OK, MessageBoxIcon.Information);
                        }
                        else
                        {
                            MessageBox.Show( "Writing device info failed.", "TL866", MessageBoxButtons.OK, MessageBoxIcon.Error);
                        }
                    }
                }
            }
        }

        private ushort Bootloader_CRC()
        {
            byte[] buff = new byte[6144];
            ////6Kbytes
            byte[] rd = new byte[64];
            for (uint i = 0; i <= buff.Length - 1; i += 64)
            {
                GetMainForm().usb.Write(new byte[] {
					1,
					64,
					(byte)(i & 0xff),
					(byte)((i >> 8) & 0xff),
					(byte)((i >> 16) & 0xff)
				});
                if (GetMainForm().usb.Read(rd) != 64)
                {
                    MessageBox.Show( "Read error!", "TL866", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                    return 0;
                }
                Array.Copy(rd, 0, buff, i, rd.Length);
            }
            Crc16 crc = new Crc16();
            return crc.GetCRC16(buff, 0);
        }

        private bool check()
        {
            byte[] rd = new byte[64];
            if (GetMainForm().usb.Get_Devices().Count > 0)
            {
                if (GetMainForm().usb.OpenDevice(GetMainForm().usb.Get_Devices()[0]))
                {
                    GetMainForm().usb.Write(new byte[] { Firmware.REPORT_COMMAND, 0, 0, 0, 0 });
                    if (GetMainForm().usb.Read(rd) > 0)
                    {
                        return ((System.Text.Encoding.UTF8.GetString(rd, 7, 8).Trim().ToLower() == "codedump" && (System.Text.Encoding.UTF8.GetString(rd, 15, 24).Trim()) == "000000000000000000000000"));
                    }
                }
            }
            return false;
        }

        private void btnClone_Click(System.Object sender, System.EventArgs e)
        {
            GetMainForm().UsbDeviceChanged(0);
            if (string.IsNullOrEmpty(devcode) || string.IsNullOrEmpty(serial))
                return;
            txtDevcode.Text = devcode;
            txtSerial.Text = serial;
        }

        private void btnDefault_Click(System.Object sender, System.EventArgs e)
        {
            txtDevcode.Text = "00000000";
            txtSerial.Text = "000000000000000000000000";
        }

        private void btnEdit_Click(System.Object sender, System.EventArgs e)
        {
            GetMainForm().EditWindow.txtDevcode.Text = txtDevcode.Text;
            GetMainForm().EditWindow.txtSerial.Text = txtSerial.Text;
            if (GetMainForm().EditWindow.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                txtDevcode.Text = GetMainForm().EditWindow.txtDevcode.Text;
                txtSerial.Text = GetMainForm().EditWindow.txtSerial.Text;
            }
        }

        private void Dialog2_FormClosing(object sender, FormClosingEventArgs e)
        {
            e.Cancel = true;
            this.Hide();
        }


        private MainForm GetMainForm()
        {
            return Application.OpenForms["MainForm"] as MainForm;
        }
    }
}
