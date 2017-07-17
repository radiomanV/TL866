using System;
using System.Text;
using System.Windows.Forms;

namespace TL866
{
    public partial class AdvancedDialog : Form
    {
        public AdvancedDialog()
        {
            InitializeComponent();
            GetInfo();
        }

        private void OK_Button_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.OK;
            Close();
        }

        public void GetInfo()
        {
            if (IsDumperActive())
            {
                TxtInfo.Clear();
                GetMainForm().usb.Write(new[] {Firmware.DUMPER_INFO});
                byte[] info = new byte[34];
                if (GetMainForm().usb.Read(info) > 0)
                {
                    TxtDevcode.Text = Encoding.ASCII.GetString(info, 0, 8).Trim();
                    TxtSerial.Text = Encoding.ASCII.GetString(info, 8, 24).Trim();

                    TxtInfo.AppendText(string.Format("Device code: {0}{1}\n", TxtDevcode.Text,
                        GetMainForm().firmware.Calc_CRC(TxtDevcode.Text, TxtSerial.Text) ? "(Bad device code)" : ""));
                    TxtInfo.AppendText(string.Format("Serial number: {0}{1}\n", TxtSerial.Text,
                        GetMainForm().firmware.Calc_CRC(TxtDevcode.Text, TxtSerial.Text) ? "(Bad serial code)" : ""));
                    TxtInfo.AppendText(string.Format("Bootloader version: {0}\n", info[32] == 1 ? "A" : "CS"));
                    TxtInfo.AppendText(string.Format("Code Protection bit: {0}", info[33] > 0 ? "No" : "Yes"));
                    ChkCP.CheckState = info[33] == 0 ? CheckState.Checked : CheckState.Unchecked;
                    if (info[32] == 1)
                        RadioA.Checked = true;
                    else
                        RadioCS.Checked = true;
                    return;
                }
            }
            TxtInfo.Clear();
            TxtDevcode.Clear();
            TxtSerial.Clear();
            ChkCP.CheckState = CheckState.Indeterminate;
            RadioA.Checked = false;
            RadioCS.Checked = false;
        }


        //Write bootloader
        private void BtnWriteBootloader_Click(object sender, EventArgs e)
        {
            if (!GetMainForm().CheckDevices(this))
                return;

            if (IsDumperActive())
                if (GetMainForm().usb.OpenDevice(GetMainForm().usb.Get_Devices()[0]))
                    if (MessageBox.Show(this, Utils.WARNING_BRICK, "TL866", MessageBoxButtons.YesNo,
                            MessageBoxIcon.Exclamation) == DialogResult.Yes)
                    {
                        uint crc = Bootloader_CRC();
                        if (crc == Firmware.A_BOOTLOADER_CRC || crc == Firmware.CS_BOOTLOADER_CRC)
                        {
                            GetMainForm().usb.Write(new[]
                            {
                                Firmware.DUMPER_WRITE_BOOTLOADER,
                                (byte) (RadioA.Checked
                                    ? Firmware.BOOTLOADER_TYPE.A_BOOTLOADER
                                    : Firmware.BOOTLOADER_TYPE.CS_BOOTLOADER)
                            });
                            byte[] readbuffer = new byte[9];
                            GetMainForm().usb.Read(readbuffer);
                            GetMainForm().UsbDeviceChanged();
                            if (readbuffer[0] == Firmware.DUMPER_WRITE_BOOTLOADER)
                                MessageBox.Show("Bootloader was successfully written.", "TL866", MessageBoxButtons.OK,
                                    MessageBoxIcon.Information);
                            else
                                MessageBox.Show("Bootloader writing failed.", "TL866", MessageBoxButtons.OK,
                                    MessageBoxIcon.Error);
                        }
                        else
                        {
                            MessageBox.Show("Bootloader CRC error!\nAs a safety measure, nothing will be written.",
                                "TL866", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                        }
                    }
        }


        //Write config
        private void BtnWriteConfig_Click(object sender, EventArgs e)
        {
            byte[] b = new byte[9];

            if (!GetMainForm().CheckDevices(this))
                return;

            if (IsDumperActive())
                if (GetMainForm().usb.OpenDevice(GetMainForm().usb.Get_Devices()[0]))
                    if (MessageBox.Show(this, Utils.WARNING_BRICK, "TL866", MessageBoxButtons.YesNo,
                            MessageBoxIcon.Exclamation) == DialogResult.Yes)
                    {
                        GetMainForm().usb.Write(new[] {Firmware.DUMPER_WRITE_CONFIG, (byte) (ChkCP.Checked ? 1 : 0)});
                        GetMainForm().usb.Read(b);
                        GetMainForm().UsbDeviceChanged();
                        if (b[0] == Firmware.DUMPER_WRITE_CONFIG)
                            MessageBox.Show("Code protection bit was successfully written.", "TL866",
                                MessageBoxButtons.OK, MessageBoxIcon.Information);
                        else
                            MessageBox.Show("Writing code protect bit failed.", "TL866", MessageBoxButtons.OK,
                                MessageBoxIcon.Error);
                    }
        }

        //write info
        private void BtnWriteInfo_Click(object sender, EventArgs e)
        {
            byte[] b = new byte[34];
            b[0] = Firmware.DUMPER_WRITE_INFO;
            string s1 = TxtDevcode.Text + new string(' ', 8 - TxtDevcode.Text.Length);
            string s2 = TxtSerial.Text + new string(' ', 24 - TxtSerial.Text.Length);
            Array.Copy(Encoding.ASCII.GetBytes(s1), 0, b, 1, 8);
            Array.Copy(Encoding.ASCII.GetBytes(s2), 0, b, 9, 24);

            if (!GetMainForm().CheckDevices(this))
                return;

            if (IsDumperActive())
                if (GetMainForm().usb.OpenDevice(GetMainForm().usb.Get_Devices()[0]))
                    if (MessageBox.Show(this, Utils.WARNING_BRICK, "TL866", MessageBoxButtons.YesNo,
                            MessageBoxIcon.Exclamation) == DialogResult.Yes)
                    {
                        GetMainForm().usb.Write(b);
                        GetMainForm().usb.Read(b);
                        GetMainForm().UsbDeviceChanged();
                        if (b[0] == Firmware.DUMPER_WRITE_INFO)
                            MessageBox.Show("Device info was successfully written.", "TL866", MessageBoxButtons.OK,
                                MessageBoxIcon.Information);
                        else
                            MessageBox.Show("Writing device info failed.", "TL866", MessageBoxButtons.OK,
                                MessageBoxIcon.Error);
                    }
        }

        private uint Bootloader_CRC()
        {
            byte[] buff = new byte[Firmware.BOOTLOADER_SIZE]; //6 Kbytes
            byte[] rd = new byte[64];
            for (uint i = 0; i < buff.Length; i += 64)
            {
                GetMainForm().usb.Write(new byte[]
                {
                    Firmware.DUMPER_READ_FLASH,
                    64,
                    (byte) (i & 0xFF),
                    (byte) ((i >> 8) & 0xFF),
                    (byte) ((i >> 16) & 0xFF)
                });
                if (GetMainForm().usb.Read(rd) != 64)
                {
                    MessageBox.Show("Read error!", "TL866", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                    return 0;
                }
                Array.Copy(rd, 0, buff, i, rd.Length);
            }
            CRC32 crc = new CRC32();
            return ~crc.GetCRC32(buff, 0xFFFFFFFF);
        }

        private bool IsDumperActive()
        {
            byte[] rd = new byte[64];
            if (GetMainForm().usb.DevicesCount > 0)
                if (GetMainForm().usb.OpenDevice(GetMainForm().usb.Get_Devices()[0]))
                {
                    GetMainForm().usb.Write(new byte[] {Firmware.REPORT_COMMAND, 0, 0, 0, 0});
                    if (GetMainForm().usb.Read(rd) > 0)
                        return Encoding.UTF8.GetString(rd, 7, 8).Trim().ToLower() == "codedump" &&
                               Encoding.UTF8.GetString(rd, 15, 24).Trim() == "000000000000000000000000";
                }
            return false;
        }

        private void BtnClone_Click(object sender, EventArgs e)
        {
            GetMainForm().UsbDeviceChanged();
            GetInfo();
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


        private MainForm GetMainForm()
        {
            return Application.OpenForms["MainForm"] as MainForm;
        }
    }
}