using System;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;
using System.IO;
using System.Threading;
using System.Reflection;
using System.Diagnostics;

namespace TL866
{
    public partial class MainForm : Form
    {
        public Firmware firmware;
        private BackgroundWorker worker;
        public UsbDevice usb;

        private bool reset_flag;
        private string devcode;
        private string serial;
        private int devtype;

        private AdvancedDialog AdvancedWindow;

        private enum LED_ACTION
        {
            LED_ERASE_ON,
            LED_ERASE_OFF,
            LED_WRITE_ON,
            LED_WRITE_OFF
        }


        public MainForm()
        {
            InitializeComponent();

            reset_flag = false;
            firmware = new Firmware();
            usb = new UsbDevice();
            devcode = "";
            serial = "";
            usb.UsbDeviceChanged += new UsbDevice.UsbDeviceChangedEventHandler(UsbDeviceChanged);
            usb.RegisterForDeviceChange(true, this);
            worker = new BackgroundWorker();
            worker.DoWork += new DoWorkEventHandler(worker_DoWork);
            worker.ProgressChanged += new ProgressChangedEventHandler(worker_ProgressChanged);
            worker.WorkerReportsProgress = true;
            worker.WorkerSupportsCancellation = true;
            Leds_Off();
            UsbDeviceChanged();
        }



        //Events/////////////////////////////////////////////
        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            worker.CancelAsync();
            usb.CloseDevice();
            usb.RegisterForDeviceChange(false, this);
        }


        private void BtnInput_Click(object sender, EventArgs e)
        {
            OpenFileDialog dlg = new OpenFileDialog();
            dlg.Title = "Update.dat";
            dlg.Filter = "dat files (*.dat)|*.dat|All files (*.*)|*.*";
            dlg.CheckFileExists = true;
            if (dlg.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                TxtInput.Text = "";
                try
                {
                    firmware.Open(dlg.FileName);
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message, "TL866", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                    lblVersion.Text = "";
                    return;
                }
                lblVersion.Text = String.Format("[V:{0}]", firmware.GetVersion());
            }
            TxtInput.Text = dlg.FileName;
        }

        private void BtnDefault_Click(object sender, EventArgs e)
        {
            string[] s = firmware.GetSerialFromBin(RadiofA.Checked ? Properties.Resources.firmwareA : Properties.Resources.firmwareCS);
            TxtDevcode.Text = s[0];
            TxtSerial.Text = s[1];
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


        private void BtnClone_Click(System.Object sender, System.EventArgs e)
        {
            UsbDeviceChanged();
            if (!String.IsNullOrEmpty(devcode) || !String.IsNullOrEmpty(serial))
            {
                TxtDevcode.Text = devcode;
                TxtSerial.Text = serial;
            }
        }


        private void BtnDump_Click(System.Object sender, System.EventArgs e)
        {
            if (!CheckDevices(this))
                return;
            if (!firmware.IsValid())
            {
                MessageBox.Show(Utils.NO_FIRMWARE, "TL866", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return;
            }
            SaveFileDialog sdialog = new SaveFileDialog();
            sdialog.Title = "Firmware dump output file";
            sdialog.Filter = "Hex files (*.hex)|*.hex|All files (*.*)|*.*";
            sdialog.CheckPathExists = true;
            sdialog.OverwritePrompt = true;
            if (sdialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                UsbDeviceChanged();
                ProgressBar1.Maximum = (int)Firmware.FLASH_SIZE;//128Kbytes
                worker.RunWorkerAsync(new object[] { 2, sdialog.FileName });
            }
        }


        private void BtnAdvanced_Click(object sender, EventArgs e)
        {
            AdvancedWindow = new AdvancedDialog();
            AdvancedWindow.Show(this);
        }

        private void BtnReflash_Click(object sender, EventArgs e)
        {
            if (worker.IsBusy) return;
            if (!CheckDevices(this))
                return;
            if (!firmware.IsValid())
            {
                MessageBox.Show(Utils.NO_FIRMWARE, "TL866", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return;
            }


            if (MessageBox.Show(Utils.WARNING_REFLASH, "TL866", MessageBoxButtons.YesNo, MessageBoxIcon.Exclamation) == DialogResult.Yes)
            {
                int ftype = (int)Firmware.FIRMWARE_TYPE.FIRMWARE_A;
                if (RadioA.Checked)
                    ftype = (int)Firmware.FIRMWARE_TYPE.FIRMWARE_A;
                if (RadioCS.Checked)
                    ftype = (int)Firmware.FIRMWARE_TYPE.FIRMWARE_CS;
                if (RadioDump.Checked)
                    ftype = (int)Firmware.FIRMWARE_TYPE.FIRMWARE_CUSTOM;
                ProgressBar1.Maximum = (int)Firmware.ENCRYPTED_FIRMWARE_SIZE;
                worker.RunWorkerAsync(new object[] { 1, ftype });
            }
        }


        private void BtnSave_Click(object sender, EventArgs e)
        {
            Save(RadiofA.Checked ? Properties.Resources.firmwareA : Properties.Resources.firmwareCS);
        }


        private void BtnReset_Click(object sender, EventArgs e)
        {
            if (worker.IsBusy)
                return;
            if (!CheckDevices(this))
                return;

            if (usb.OpenDevice(usb.Get_Devices()[0]))
            {
                worker.RunWorkerAsync(new object[] { 0 });
            }
        }


        private void worker_ProgressChanged(object sender, System.ComponentModel.ProgressChangedEventArgs e)
        {
            switch ((LED_ACTION)e.ProgressPercentage)
            {
                case LED_ACTION.LED_ERASE_ON:
                    LedErase.BackColor = Color.FromArgb(255, 255, 0);
                    break;
                case LED_ACTION.LED_ERASE_OFF:
                    LedErase.BackColor = Color.FromArgb(64, 64, 0);
                    break;
                case LED_ACTION.LED_WRITE_ON:
                    LedWrite.BackColor = Color.FromArgb(255, 0, 0);
                    break;
                case LED_ACTION.LED_WRITE_OFF:
                    LedWrite.BackColor = Color.FromArgb(64, 0, 0);
                    break;
            }
        }



        private void worker_DoWork(object sender, System.ComponentModel.DoWorkEventArgs e)
        {
            object[] parameters = e.Argument as object[];
            switch ((int)parameters[0])
            {
                case 0:
                    Reset_Device();
                    break;
                case 1:
                    Reflash((int)(parameters[1]));
                    break;
                case 2:
                    Dump(parameters[1].ToString());
                    break;
            }
        }


        public void UsbDeviceChanged()
        {
            if (usb.Get_Devices().Count == 0 && reset_flag)
                return;
           
            reset_flag = false;
            int count = usb.Get_Devices().Count;
            Assembly assembly = Assembly.GetExecutingAssembly();
            FileVersionInfo version = FileVersionInfo.GetVersionInfo(assembly.Location);
            this.Text = String.Format("TL866 firmware updater  V{0}   ({1} {2} connected)",version.FileVersion, count, count == 1 ? "device" : "devices");
            if (usb.Get_Devices().Count > 0 && usb.OpenDevice(usb.Get_Devices()[0]))
            {

                usb.Write(new byte[] { Firmware.REPORT_COMMAND, 0, 0, 0, 0 });
                byte[] readbuffer = new byte[64];
                usb.Read(readbuffer);
                TxtInfo.Clear();
                switch (readbuffer[6])
                {
                    case 1:
                        TxtInfo.AppendText("Device version: TL866A\n");
                        devtype = (int)Firmware.PROGRAMMER_TYPE.TL866A;
                        break;
                    case 2:
                        TxtInfo.AppendText("Device version: TL866CS\n");
                        devtype = (int)Firmware.PROGRAMMER_TYPE.TL866CS;
                        break;
                    default:
                        TxtInfo.AppendText("Device version: Unknown\n");
                        break;
                }
                switch (readbuffer[1])
                {
                    case (int)Firmware.DEVICE_STATUS.NORMAL_MODE:
                        TxtInfo.AppendText("Device status: Normal working mode.\n");
                        LedNorm.BackColor = Color.FromArgb(0, 255, 0);
                        LedBoot.BackColor = Color.FromArgb(0, 64, 0);
                        break;
                    case (int)Firmware.DEVICE_STATUS.BOOTLOADER_MODE:
                        TxtInfo.AppendText("Device status: Bootloader mode <waiting for update.>\n");
                        LedNorm.BackColor = Color.FromArgb(0, 64, 0);
                        LedBoot.BackColor = Color.FromArgb(0, 255, 0);
                        break;
                    default:
                        TxtInfo.AppendText("Device status: Unknown\n");
                        LedNorm.BackColor = Color.FromArgb(0, 64, 0);
                        LedBoot.BackColor = Color.FromArgb(0, 64, 0);
                        break;
                }



                string s_dev = System.Text.Encoding.UTF8.GetString(readbuffer, 7, 8).Trim();
                string s_ser = System.Text.Encoding.UTF8.GetString(readbuffer, 15, 24).Trim();
                bool isDumperActive = (s_dev.ToLower() == "codedump" && s_ser == "000000000000000000000000");

                if (isDumperActive)
                {
                    usb.Write(new byte[] { Firmware.DUMPER_INFO });
                    byte[] info = new byte[34];
                    if (usb.Read(info) > 0)
                    {
                        devcode = System.Text.Encoding.ASCII.GetString(info, 0, 8).Trim();
                        serial = System.Text.Encoding.ASCII.GetString(info, 8, 24).Trim();
                    }
                    else
                    {
                        devcode = "";
                        serial = "";
                    }
                }
                else
                {
                    devcode = s_dev;
                    serial = s_ser;
                }


                if (AdvancedWindow != null && !AdvancedWindow.IsDisposed)
                    AdvancedWindow.GetInfo();
                TxtInfo.AppendText(String.Format("Device code: {0}{1}\n", devcode, firmware.Calc_CRC(devcode, serial) ? "(Bad device code)" : ""));
                TxtInfo.AppendText(String.Format("Serial number: {0}{1}\n", serial, firmware.Calc_CRC(devcode, serial) ? "(Bad serial code)" : ""));
                TxtInfo.AppendText(String.Format("Firmware version: {0}", isDumperActive ? "Firmware dumper" : readbuffer[1] == 1 ? String.Format("{0}.{1}.{2}", readbuffer[39], readbuffer[5], readbuffer[4]) : "Bootloader"));
                BtnDump.Enabled = isDumperActive;
                BtnAdvanced.Enabled = isDumperActive;
            }
            else
            {
                BtnDump.Enabled = false;
                BtnAdvanced.Enabled = false;
                Leds_Off();
                TxtInfo.Clear();
                devcode = "";
                serial = "";
                if (AdvancedWindow != null && !AdvancedWindow.IsDisposed)
                    AdvancedWindow.GetInfo();
            }
        }


        protected override void WndProc(ref System.Windows.Forms.Message m)
        {
            usb.ProcessWindowsMessage(ref m);
            base.WndProc(ref m);
        }


        //Functions//////////////////////////////
        private void Reflash(int version)
        {

            usb.Write(new byte[] { Firmware.REPORT_COMMAND, 0, 0, 0, 0 });
            byte[] readbuffer = new byte[64];
            usb.Read(readbuffer);

            if (readbuffer[1] != (int)Firmware.DEVICE_STATUS.BOOTLOADER_MODE)
            {
                Reset_Device();
                if (!wait_for_device())
                {
                    MessageBox.Show("Reset Error!", "TL866", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                    return;
                }
            }


            byte[] buffer = new byte[Firmware.ENCRYPTED_FIRMWARE_SIZE];
            switch (version)
            {
                case (int)Firmware.FIRMWARE_TYPE.FIRMWARE_A:
                    Array.Copy(firmware.GetEncryptedFirmware((int)Firmware.ENCRYPTION_KEY.A_KEY, devtype), buffer, buffer.Length);
                    break;
                case (int)Firmware.FIRMWARE_TYPE.FIRMWARE_CS:
                    Array.Copy(firmware.GetEncryptedFirmware((int)Firmware.ENCRYPTION_KEY.CS_KEY, devtype), buffer, buffer.Length);
                    break;
                case (int)Firmware.FIRMWARE_TYPE.FIRMWARE_CUSTOM:
                    Array.Copy(firmware.Encrypt_Firmware(Properties.Resources.Dumper, devtype), buffer, buffer.Length);
                    break;
            }


            Thread.Sleep(1000);
            Message("<erasing...>");
            worker.ReportProgress((int)LED_ACTION.LED_ERASE_ON);
            if (!Erase_Device(firmware.GetEraseParametter(devtype)))
            {
                MessageBox.Show("Erase failed", "TL866", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }
            if (worker.CancellationPending)
                return;
            worker.ReportProgress((int)LED_ACTION.LED_ERASE_OFF);
            Thread.Sleep(1000);
            worker.ReportProgress((int)LED_ACTION.LED_WRITE_ON);
            Message("<writing...>");
            if (!Write_Device(buffer))
            {

                worker.ReportProgress((int)LED_ACTION.LED_WRITE_OFF);
                worker.ReportProgress((int)LED_ACTION.LED_ERASE_OFF);
                MessageBox.Show("Reflash Failed!", "TL866", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }
            Thread.Sleep(500);
            Message("<resetting...>");
            worker.ReportProgress((int)LED_ACTION.LED_WRITE_OFF);
            worker.ReportProgress((int)LED_ACTION.LED_ERASE_OFF);
            Reset_Device();
            if (!wait_for_device())
            {
                SetProgressBar(0);
                MessageBox.Show("Reset failed", "TL866", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }


            usb.Write(new byte[] { Firmware.REPORT_COMMAND, 0, 0, 0, 0 });
            usb.Read(readbuffer);
            if (readbuffer[1] == (int)Firmware.DEVICE_STATUS.NORMAL_MODE)
            {
                MessageBox.Show("Reflash O.K.!", "TL866", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            else
            {
                MessageBox.Show("Reflash Failed!", "TL866", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            SetProgressBar(0);
        }

        private bool Write_Device(byte[] buffer)
        {
            if (!usb.OpenDevice(usb.Get_Devices()[0]))
                return false;
            byte[] b = new byte[Firmware.BLOCK_SIZE + 7];
            uint address = 0x1800;
            SetProgressBar(0);
            for (int i = 0; i <= buffer.Length - 1; i += (int)Firmware.BLOCK_SIZE)
            {
                if (worker.CancellationPending)
                    return false;
                b[0] = Firmware.WRITE_COMMAND;
                b[1] = 0;
                b[2] = (byte)Firmware.BLOCK_SIZE;
                b[3] = 0;
                Array.Copy(BitConverter.GetBytes(address), 0, b, 4, 4);
                Array.Copy(buffer, i, b, 7, (int)Firmware.BLOCK_SIZE);
                address += 64;
                if (!usb.Write(b))
                    return false;
                SetProgressBar(i);
            }
            return true;
        }


        private bool Erase_Device(byte magic_number)
        {
            if (usb.Get_Devices().Count > 0)
            {
                if (usb.OpenDevice(usb.Get_Devices()[0]))
                {
                    byte[] buffer = new byte[20];
                    Array.Clear(buffer, 0, buffer.Length);
                    buffer[0] = Firmware.ERASE_COMMAND;
                    buffer[7] = magic_number;
                    usb.Write(buffer);
                    byte[] readbuffer = new byte[32];
                    usb.Read(readbuffer);
                    if (readbuffer[0] == 0xcc)
                        return true;
                }
            }
            return false;
        }


        private void Leds_Off()
        {
            LedBoot.BackColor = Color.FromArgb(0, 64, 0);
            LedNorm.BackColor = Color.FromArgb(0, 64, 0);
            LedErase.BackColor = Color.FromArgb(64, 64, 0);
            LedWrite.BackColor = Color.FromArgb(64, 0, 0);
        }


        public bool CheckDevices(Form parent)
        {
            if (usb.Get_Devices().Count == 0)
            {
                MessageBox.Show(parent, Utils.NO_DEVICE, "TL866", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return false;
            }
            if (usb.Get_Devices().Count > 1)
            {
                MessageBox.Show(parent, Utils.MULTIPLE_DEVICES, "TL866", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return false;
            }
            return true;
        }

        //Delegates
        delegate void SetProgressBarCallBack(int percentage);
        delegate void SetMessageCallBack(string message);


        private void SetProgressBar(int percentage)
        {
            try
            {
                if (ProgressBar1.InvokeRequired)
                {
                    SetProgressBarCallBack c = new SetProgressBarCallBack(SetProgressBar);
                    this.Invoke(c, new Object[] { percentage });
                }
                else
                {
                    ProgressBar1.Value = percentage;
                }
            }
            catch { }
        }

        private void Message(string message)
        {
            try
            {
                if (ProgressBar1.InvokeRequired)
                {
                    SetMessageCallBack c = new SetMessageCallBack(Message);
                    this.Invoke(c, new Object[] { message });
                }
                else
                {

                    string[] lines = TxtInfo.Lines;
                    if (lines.Length > 0)
                    {
                        lines[1] = "Device status: Bootloader mode " + message;
                        TxtInfo.Lines = lines;
                    }
                }
            }
            catch { }
        }



        private void Dump(string filepath)
        {
            byte[] buffer = new byte[Firmware.FLASH_SIZE];//128Kbytes buffer   
            byte[] readbuffer = new byte[64];
            for (int i = 0; i <= buffer.Length - 1; i += 64)
            {
                if (worker.CancellationPending)
                    return;
                usb.Write(new byte[] { Firmware.DUMPER_READ_FLASH, 64, (byte)(i & 0xFF), (byte)((i >> 8) & 0xFF), (byte)((i >> 16) & 0xFF) });
                if (usb.Read(readbuffer) != 64)
                {
                    MessageBox.Show("Read error!", "TL866", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                    return;
                }
                Array.Copy(readbuffer, 0, buffer, i, readbuffer.Length);
                SetProgressBar(i);
                Application.DoEvents();
            }
            byte[] temp = new byte[Firmware.UNENCRYPTED_FIRMWARE_SIZE];
            firmware.Decrypt_Firmware(temp, devtype);
            Array.Copy(temp, 0, buffer, Firmware.BOOTLOADER_SIZE, Firmware.UNENCRYPTED_FIRMWARE_SIZE);
            StreamWriter streamwriter = File.CreateText(filepath);
            if (streamwriter != null)
            {
                hexwriter hexwriter = new hexwriter();
                hexwriter.WriteHex(buffer, streamwriter);
                streamwriter.Close();
                MessageBox.Show("Firmware dump complete!", "TL866", MessageBoxButtons.OK, MessageBoxIcon.Information);
                SetProgressBar(0);
            }
            else
            {
                MessageBox.Show("Error creating dump.hex file", "TL866", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
            }
        }


        private void Save(byte[] data)
        {
            SaveFileDialog dlg = new SaveFileDialog();
            StreamWriter streamwriter = null;
            dlg.Title = "Firmware output hex file";
            dlg.Filter = "hex files (*.hex)|*.hex|All files (*.*)|*.*";
            dlg.CheckPathExists = true;
            dlg.OverwritePrompt = true;
            if (dlg.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                try
                {
                    if (File.Exists(dlg.FileName))
                        File.Delete(dlg.FileName);
                    streamwriter = File.CreateText(dlg.FileName);
                }
                catch
                {
                    MessageBox.Show(String.Format("Error creating file {0}", dlg.FileName), "TL866", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }
                byte[] temp = new byte[data.Length];
                MemoryStream memorystream = new MemoryStream(temp);
                ////Writing bootloader 
                memorystream.Write(data, 0, (int)Firmware.BOOTLOADER_SIZE);
                ////Writing main firmware or empty on choice
                if (OptionFull.Checked)
                {
                    memorystream.Write(data, (int)Firmware.BOOTLOADER_SIZE, (int)Firmware.UNENCRYPTED_FIRMWARE_SIZE);
                }
                else
                {
                    byte[] b = new byte[Firmware.UNENCRYPTED_FIRMWARE_SIZE];
                    for (int i = 0; i <= b.Length - 1; i++)
                    {
                        b[i] = 0xFF;
                    }
                    memorystream.Write(b, 0, b.Length);
                }
                ////Writing Decryption tables and serials
                byte[] info = new byte[80];
                Array.Copy(data, 0x1FD00, info, 0, info.Length);
                firmware.DecryptSerial(info, data);
                string s1 = TxtDevcode.Text + new string(' ', 8 - TxtDevcode.Text.Length);
                string s2 = TxtSerial.Text + new string(' ', 24 - TxtSerial.Text.Length);
                Array.Copy(System.Text.Encoding.ASCII.GetBytes(s1), 0, info, 0, 8);
                Array.Copy(System.Text.Encoding.ASCII.GetBytes(s2), 0, info, 8, 24);
                firmware.EncryptSerial(info, data);
                memorystream.Write(data, 0x1FC00, 0x100);//write 256 bytes xor table
                memorystream.Write(info, 0, info.Length);//write 80 bytes encrypted serial
                memorystream.Write(data, 0x1FD50, 0x2B0);// write the remaining bytes up to 0x1FFFF
                memorystream.Close();
                ////writing to file
                hexwriter hexwriter = new hexwriter();
                hexwriter.WriteHex(temp, streamwriter);
                streamwriter.Close();
                System.Media.SystemSounds.Asterisk.Play();
            }
        }


        //wait for device to reset
        private bool wait_for_device()
        {
            int cnt = 50;//5 seconds
            while (usb.Get_Devices().Count > 0)//wait for device to leave
            {
                Thread.Sleep(100);
                if (!(--cnt > 0))
                    return false;//reset error
            }

            cnt = 50;//5 seconds
            while (!(usb.Get_Devices().Count > 0))//wait for device to arrive
            {
                Thread.Sleep(100);
                if (!(--cnt > 0))
                    return false;//reset error
            }
            return true;//device reset ok
        }


        private void Reset_Device()
        {
            reset_flag = true;
            usb.Write(new byte[] { Firmware.RESET_COMMAND, 0, 0, 0 });
            usb.CloseDevice();
        }

    }
}
