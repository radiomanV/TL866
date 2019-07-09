namespace TL866
{
    partial class MainForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.TabPage2 = new System.Windows.Forms.TabPage();
            this.GroupBox3 = new System.Windows.Forms.GroupBox();
            this.BtnDefault = new System.Windows.Forms.Button();
            this.BtnClone = new System.Windows.Forms.Button();
            this.BtnEdit = new System.Windows.Forms.Button();
            this.Label5 = new System.Windows.Forms.Label();
            this.Label4 = new System.Windows.Forms.Label();
            this.TxtSerial = new System.Windows.Forms.TextBox();
            this.TxtDevcode = new System.Windows.Forms.TextBox();
            this.GroupBox2 = new System.Windows.Forms.GroupBox();
            this.Panel1 = new System.Windows.Forms.Panel();
            this.RadiofCS = new System.Windows.Forms.RadioButton();
            this.RadiofA = new System.Windows.Forms.RadioButton();
            this.BtnSave = new System.Windows.Forms.Button();
            this.Label6 = new System.Windows.Forms.Label();
            this.OptionBoot = new System.Windows.Forms.RadioButton();
            this.OptionFull = new System.Windows.Forms.RadioButton();
            this.TabPage1 = new System.Windows.Forms.TabPage();
            this.lblVersion = new System.Windows.Forms.Label();
            this.GroupBox1 = new System.Windows.Forms.GroupBox();
            this.BtnAdvanced = new System.Windows.Forms.Button();
            this.BtnDump = new System.Windows.Forms.Button();
            this.BtnReset = new System.Windows.Forms.Button();
            this.BtnReflash = new System.Windows.Forms.Button();
            this.RadioDump = new System.Windows.Forms.RadioButton();
            this.RadioCS = new System.Windows.Forms.RadioButton();
            this.RadioA = new System.Windows.Forms.RadioButton();
            this.Label1 = new System.Windows.Forms.Label();
            this.BtnInput = new System.Windows.Forms.Button();
            this.TxtInput = new System.Windows.Forms.TextBox();
            this.TxtInfo = new System.Windows.Forms.TextBox();
            this.Label13 = new System.Windows.Forms.Label();
            this.Label11 = new System.Windows.Forms.Label();
            this.Label10 = new System.Windows.Forms.Label();
            this.Label8 = new System.Windows.Forms.Label();
            this.ProgressBar1 = new System.Windows.Forms.ProgressBar();
            this.LedWrite = new System.Windows.Forms.Label();
            this.LedErase = new System.Windows.Forms.Label();
            this.LedNorm = new System.Windows.Forms.Label();
            this.LedBoot = new System.Windows.Forms.Label();
            this.TabControl = new System.Windows.Forms.TabControl();
            this.cp0 = new System.Windows.Forms.CheckBox();
            this.label2 = new System.Windows.Forms.Label();
            this.TabPage2.SuspendLayout();
            this.GroupBox3.SuspendLayout();
            this.GroupBox2.SuspendLayout();
            this.Panel1.SuspendLayout();
            this.TabPage1.SuspendLayout();
            this.GroupBox1.SuspendLayout();
            this.TabControl.SuspendLayout();
            this.SuspendLayout();
            // 
            // TabPage2
            // 
            this.TabPage2.BackColor = System.Drawing.Color.Transparent;
            this.TabPage2.Controls.Add(this.label2);
            this.TabPage2.Controls.Add(this.GroupBox3);
            this.TabPage2.Controls.Add(this.GroupBox2);
            this.TabPage2.Location = new System.Drawing.Point(4, 25);
            this.TabPage2.Name = "TabPage2";
            this.TabPage2.Padding = new System.Windows.Forms.Padding(3);
            this.TabPage2.Size = new System.Drawing.Size(733, 329);
            this.TabPage2.TabIndex = 1;
            this.TabPage2.Text = "Firmware";
            this.TabPage2.UseVisualStyleBackColor = true;
            // 
            // GroupBox3
            // 
            this.GroupBox3.Controls.Add(this.BtnDefault);
            this.GroupBox3.Controls.Add(this.BtnClone);
            this.GroupBox3.Controls.Add(this.BtnEdit);
            this.GroupBox3.Controls.Add(this.Label5);
            this.GroupBox3.Controls.Add(this.Label4);
            this.GroupBox3.Controls.Add(this.TxtSerial);
            this.GroupBox3.Controls.Add(this.TxtDevcode);
            this.GroupBox3.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.GroupBox3.Location = new System.Drawing.Point(8, 16);
            this.GroupBox3.Name = "GroupBox3";
            this.GroupBox3.Size = new System.Drawing.Size(717, 108);
            this.GroupBox3.TabIndex = 58;
            this.GroupBox3.TabStop = false;
            this.GroupBox3.Text = "Device Serial number";
            // 
            // BtnDefault
            // 
            this.BtnDefault.Location = new System.Drawing.Point(631, 55);
            this.BtnDefault.Name = "BtnDefault";
            this.BtnDefault.Size = new System.Drawing.Size(77, 23);
            this.BtnDefault.TabIndex = 61;
            this.BtnDefault.Text = "Default";
            this.BtnDefault.UseVisualStyleBackColor = true;
            this.BtnDefault.Click += new System.EventHandler(this.BtnDefault_Click);
            // 
            // BtnClone
            // 
            this.BtnClone.Location = new System.Drawing.Point(548, 55);
            this.BtnClone.Name = "BtnClone";
            this.BtnClone.Size = new System.Drawing.Size(77, 23);
            this.BtnClone.TabIndex = 63;
            this.BtnClone.Text = "Clone";
            this.BtnClone.UseVisualStyleBackColor = true;
            this.BtnClone.Click += new System.EventHandler(this.BtnClone_Click);
            // 
            // BtnEdit
            // 
            this.BtnEdit.Location = new System.Drawing.Point(465, 55);
            this.BtnEdit.Name = "BtnEdit";
            this.BtnEdit.Size = new System.Drawing.Size(77, 23);
            this.BtnEdit.TabIndex = 64;
            this.BtnEdit.Text = "Edit";
            this.BtnEdit.UseVisualStyleBackColor = true;
            this.BtnEdit.Click += new System.EventHandler(this.BtnEdit_Click);
            // 
            // Label5
            // 
            this.Label5.AutoSize = true;
            this.Label5.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.Label5.Location = new System.Drawing.Point(220, 30);
            this.Label5.Name = "Label5";
            this.Label5.Size = new System.Drawing.Size(91, 16);
            this.Label5.TabIndex = 60;
            this.Label5.Text = "Serial number";
            // 
            // Label4
            // 
            this.Label4.AutoSize = true;
            this.Label4.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.Label4.Location = new System.Drawing.Point(24, 30);
            this.Label4.Name = "Label4";
            this.Label4.Size = new System.Drawing.Size(85, 16);
            this.Label4.TabIndex = 59;
            this.Label4.Text = "Device code";
            // 
            // TxtSerial
            // 
            this.TxtSerial.BackColor = System.Drawing.SystemColors.Info;
            this.TxtSerial.Font = new System.Drawing.Font("Microsoft Sans Serif", 14.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.TxtSerial.Location = new System.Drawing.Point(135, 49);
            this.TxtSerial.MaxLength = 24;
            this.TxtSerial.Name = "TxtSerial";
            this.TxtSerial.ReadOnly = true;
            this.TxtSerial.Size = new System.Drawing.Size(324, 29);
            this.TxtSerial.TabIndex = 58;
            this.TxtSerial.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.TxtSerial.WordWrap = false;
            // 
            // TxtDevcode
            // 
            this.TxtDevcode.BackColor = System.Drawing.SystemColors.Info;
            this.TxtDevcode.Font = new System.Drawing.Font("Microsoft Sans Serif", 14.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.TxtDevcode.Location = new System.Drawing.Point(9, 49);
            this.TxtDevcode.MaxLength = 8;
            this.TxtDevcode.Name = "TxtDevcode";
            this.TxtDevcode.ReadOnly = true;
            this.TxtDevcode.Size = new System.Drawing.Size(117, 29);
            this.TxtDevcode.TabIndex = 57;
            this.TxtDevcode.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // GroupBox2
            // 
            this.GroupBox2.Controls.Add(this.Panel1);
            this.GroupBox2.Controls.Add(this.BtnSave);
            this.GroupBox2.Controls.Add(this.Label6);
            this.GroupBox2.Controls.Add(this.OptionBoot);
            this.GroupBox2.Controls.Add(this.OptionFull);
            this.GroupBox2.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.GroupBox2.Location = new System.Drawing.Point(8, 152);
            this.GroupBox2.Name = "GroupBox2";
            this.GroupBox2.Size = new System.Drawing.Size(717, 132);
            this.GroupBox2.TabIndex = 57;
            this.GroupBox2.TabStop = false;
            this.GroupBox2.Text = "Hex file generator";
            // 
            // Panel1
            // 
            this.Panel1.Controls.Add(this.cp0);
            this.Panel1.Controls.Add(this.RadiofCS);
            this.Panel1.Controls.Add(this.RadiofA);
            this.Panel1.Location = new System.Drawing.Point(232, 20);
            this.Panel1.Name = "Panel1";
            this.Panel1.Size = new System.Drawing.Size(479, 100);
            this.Panel1.TabIndex = 58;
            // 
            // RadiofCS
            // 
            this.RadiofCS.AutoSize = true;
            this.RadiofCS.Location = new System.Drawing.Point(16, 35);
            this.RadiofCS.Name = "RadiofCS";
            this.RadiofCS.Size = new System.Drawing.Size(193, 20);
            this.RadiofCS.TabIndex = 1;
            this.RadiofCS.Text = "Generate TL866CS firmware";
            this.RadiofCS.UseVisualStyleBackColor = true;
            // 
            // RadiofA
            // 
            this.RadiofA.AutoSize = true;
            this.RadiofA.Checked = true;
            this.RadiofA.Location = new System.Drawing.Point(16, 9);
            this.RadiofA.Name = "RadiofA";
            this.RadiofA.Size = new System.Drawing.Size(184, 20);
            this.RadiofA.TabIndex = 0;
            this.RadiofA.TabStop = true;
            this.RadiofA.Text = "Generate TL866A firmware";
            this.RadiofA.UseVisualStyleBackColor = true;
            // 
            // BtnSave
            // 
            this.BtnSave.Location = new System.Drawing.Point(27, 97);
            this.BtnSave.Name = "BtnSave";
            this.BtnSave.Size = new System.Drawing.Size(77, 23);
            this.BtnSave.TabIndex = 57;
            this.BtnSave.Text = "Save";
            this.BtnSave.UseVisualStyleBackColor = true;
            this.BtnSave.Click += new System.EventHandler(this.BtnSave_Click);
            // 
            // Label6
            // 
            this.Label6.AutoSize = true;
            this.Label6.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.Label6.Location = new System.Drawing.Point(518, 113);
            this.Label6.Name = "Label6";
            this.Label6.Size = new System.Drawing.Size(0, 16);
            this.Label6.TabIndex = 56;
            // 
            // OptionBoot
            // 
            this.OptionBoot.AutoSize = true;
            this.OptionBoot.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.OptionBoot.Location = new System.Drawing.Point(15, 55);
            this.OptionBoot.Name = "OptionBoot";
            this.OptionBoot.Size = new System.Drawing.Size(121, 20);
            this.OptionBoot.TabIndex = 0;
            this.OptionBoot.Text = "Bootloader only";
            this.OptionBoot.UseVisualStyleBackColor = true;
            // 
            // OptionFull
            // 
            this.OptionFull.AutoSize = true;
            this.OptionFull.Checked = true;
            this.OptionFull.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.OptionFull.Location = new System.Drawing.Point(15, 29);
            this.OptionFull.Name = "OptionFull";
            this.OptionFull.Size = new System.Drawing.Size(100, 20);
            this.OptionFull.TabIndex = 0;
            this.OptionFull.TabStop = true;
            this.OptionFull.Text = "Full firmware";
            this.OptionFull.UseVisualStyleBackColor = true;
            // 
            // TabPage1
            // 
            this.TabPage1.BackColor = System.Drawing.Color.Transparent;
            this.TabPage1.Controls.Add(this.lblVersion);
            this.TabPage1.Controls.Add(this.GroupBox1);
            this.TabPage1.Controls.Add(this.Label1);
            this.TabPage1.Controls.Add(this.BtnInput);
            this.TabPage1.Controls.Add(this.TxtInput);
            this.TabPage1.Controls.Add(this.TxtInfo);
            this.TabPage1.Controls.Add(this.Label13);
            this.TabPage1.Controls.Add(this.Label11);
            this.TabPage1.Controls.Add(this.Label10);
            this.TabPage1.Controls.Add(this.Label8);
            this.TabPage1.Controls.Add(this.ProgressBar1);
            this.TabPage1.Controls.Add(this.LedWrite);
            this.TabPage1.Controls.Add(this.LedErase);
            this.TabPage1.Controls.Add(this.LedNorm);
            this.TabPage1.Controls.Add(this.LedBoot);
            this.TabPage1.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.TabPage1.Location = new System.Drawing.Point(4, 25);
            this.TabPage1.Name = "TabPage1";
            this.TabPage1.Padding = new System.Windows.Forms.Padding(3);
            this.TabPage1.Size = new System.Drawing.Size(733, 329);
            this.TabPage1.TabIndex = 0;
            this.TabPage1.Text = "Hardware";
            this.TabPage1.UseVisualStyleBackColor = true;
            // 
            // lblVersion
            // 
            this.lblVersion.AutoSize = true;
            this.lblVersion.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.lblVersion.ForeColor = System.Drawing.Color.Green;
            this.lblVersion.Location = new System.Drawing.Point(119, 11);
            this.lblVersion.Name = "lblVersion";
            this.lblVersion.Size = new System.Drawing.Size(0, 16);
            this.lblVersion.TabIndex = 64;
            // 
            // GroupBox1
            // 
            this.GroupBox1.Controls.Add(this.BtnAdvanced);
            this.GroupBox1.Controls.Add(this.BtnDump);
            this.GroupBox1.Controls.Add(this.BtnReset);
            this.GroupBox1.Controls.Add(this.BtnReflash);
            this.GroupBox1.Controls.Add(this.RadioDump);
            this.GroupBox1.Controls.Add(this.RadioCS);
            this.GroupBox1.Controls.Add(this.RadioA);
            this.GroupBox1.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.GroupBox1.Location = new System.Drawing.Point(8, 58);
            this.GroupBox1.Name = "GroupBox1";
            this.GroupBox1.Size = new System.Drawing.Size(715, 110);
            this.GroupBox1.TabIndex = 63;
            this.GroupBox1.TabStop = false;
            this.GroupBox1.Text = "Firmware to write";
            // 
            // BtnAdvanced
            // 
            this.BtnAdvanced.Enabled = false;
            this.BtnAdvanced.Location = new System.Drawing.Point(526, 46);
            this.BtnAdvanced.Name = "BtnAdvanced";
            this.BtnAdvanced.Size = new System.Drawing.Size(79, 23);
            this.BtnAdvanced.TabIndex = 63;
            this.BtnAdvanced.Text = "Advanced";
            this.BtnAdvanced.UseVisualStyleBackColor = true;
            this.BtnAdvanced.Click += new System.EventHandler(this.BtnAdvanced_Click);
            // 
            // BtnDump
            // 
            this.BtnDump.Enabled = false;
            this.BtnDump.Location = new System.Drawing.Point(445, 46);
            this.BtnDump.Name = "BtnDump";
            this.BtnDump.Size = new System.Drawing.Size(75, 23);
            this.BtnDump.TabIndex = 62;
            this.BtnDump.Text = "Dump";
            this.BtnDump.UseVisualStyleBackColor = true;
            this.BtnDump.Click += new System.EventHandler(this.BtnDump_Click);
            // 
            // BtnReset
            // 
            this.BtnReset.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.BtnReset.Location = new System.Drawing.Point(364, 46);
            this.BtnReset.Name = "BtnReset";
            this.BtnReset.Size = new System.Drawing.Size(75, 23);
            this.BtnReset.TabIndex = 53;
            this.BtnReset.Text = "Reset";
            this.BtnReset.UseVisualStyleBackColor = true;
            this.BtnReset.Click += new System.EventHandler(this.BtnReset_Click);
            // 
            // BtnReflash
            // 
            this.BtnReflash.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.BtnReflash.Location = new System.Drawing.Point(283, 46);
            this.BtnReflash.Name = "BtnReflash";
            this.BtnReflash.Size = new System.Drawing.Size(75, 23);
            this.BtnReflash.TabIndex = 43;
            this.BtnReflash.Text = "Reflash";
            this.BtnReflash.UseVisualStyleBackColor = true;
            this.BtnReflash.Click += new System.EventHandler(this.BtnReflash_Click);
            // 
            // RadioDump
            // 
            this.RadioDump.AutoSize = true;
            this.RadioDump.Location = new System.Drawing.Point(15, 75);
            this.RadioDump.Name = "RadioDump";
            this.RadioDump.Size = new System.Drawing.Size(130, 20);
            this.RadioDump.TabIndex = 2;
            this.RadioDump.Text = "Firmware dumper";
            this.RadioDump.UseVisualStyleBackColor = true;
            // 
            // RadioCS
            // 
            this.RadioCS.AutoSize = true;
            this.RadioCS.Location = new System.Drawing.Point(15, 49);
            this.RadioCS.Name = "RadioCS";
            this.RadioCS.Size = new System.Drawing.Size(134, 20);
            this.RadioCS.TabIndex = 1;
            this.RadioCS.Text = "TL866CS firmware";
            this.RadioCS.UseVisualStyleBackColor = true;
            // 
            // RadioA
            // 
            this.RadioA.AutoSize = true;
            this.RadioA.Checked = true;
            this.RadioA.Location = new System.Drawing.Point(15, 23);
            this.RadioA.Name = "RadioA";
            this.RadioA.Size = new System.Drawing.Size(125, 20);
            this.RadioA.TabIndex = 0;
            this.RadioA.TabStop = true;
            this.RadioA.Text = "TL866A firmware";
            this.RadioA.UseVisualStyleBackColor = true;
            // 
            // Label1
            // 
            this.Label1.AutoSize = true;
            this.Label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.Label1.Location = new System.Drawing.Point(30, 11);
            this.Label1.Name = "Label1";
            this.Label1.Size = new System.Drawing.Size(95, 16);
            this.Label1.TabIndex = 53;
            this.Label1.Text = "Update.dat file";
            // 
            // BtnInput
            // 
            this.BtnInput.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.BtnInput.Location = new System.Drawing.Point(697, 30);
            this.BtnInput.Name = "BtnInput";
            this.BtnInput.Size = new System.Drawing.Size(26, 22);
            this.BtnInput.TabIndex = 40;
            this.BtnInput.Text = "...";
            this.BtnInput.UseVisualStyleBackColor = true;
            this.BtnInput.Click += new System.EventHandler(this.BtnInput_Click);
            // 
            // TxtInput
            // 
            this.TxtInput.BackColor = System.Drawing.SystemColors.Info;
            this.TxtInput.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.TxtInput.Location = new System.Drawing.Point(6, 30);
            this.TxtInput.Name = "TxtInput";
            this.TxtInput.ReadOnly = true;
            this.TxtInput.Size = new System.Drawing.Size(680, 22);
            this.TxtInput.TabIndex = 38;
            // 
            // TxtInfo
            // 
            this.TxtInfo.BackColor = System.Drawing.SystemColors.Info;
            this.TxtInfo.Font = new System.Drawing.Font("Courier New", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.TxtInfo.ForeColor = System.Drawing.SystemColors.WindowText;
            this.TxtInfo.Location = new System.Drawing.Point(8, 174);
            this.TxtInfo.Multiline = true;
            this.TxtInfo.Name = "TxtInfo";
            this.TxtInfo.ReadOnly = true;
            this.TxtInfo.Size = new System.Drawing.Size(717, 115);
            this.TxtInfo.TabIndex = 45;
            this.TxtInfo.WordWrap = false;
            // 
            // Label13
            // 
            this.Label13.AutoSize = true;
            this.Label13.Location = new System.Drawing.Point(119, 292);
            this.Label13.Name = "Label13";
            this.Label13.Size = new System.Drawing.Size(32, 13);
            this.Label13.TabIndex = 57;
            this.Label13.Text = "Write";
            // 
            // Label11
            // 
            this.Label11.AutoSize = true;
            this.Label11.Location = new System.Drawing.Point(84, 292);
            this.Label11.Name = "Label11";
            this.Label11.Size = new System.Drawing.Size(34, 13);
            this.Label11.TabIndex = 56;
            this.Label11.Text = "Erase";
            // 
            // Label10
            // 
            this.Label10.AutoSize = true;
            this.Label10.Location = new System.Drawing.Point(11, 292);
            this.Label10.Name = "Label10";
            this.Label10.Size = new System.Drawing.Size(32, 13);
            this.Label10.TabIndex = 60;
            this.Label10.Text = "Norm";
            // 
            // Label8
            // 
            this.Label8.AutoSize = true;
            this.Label8.Location = new System.Drawing.Point(49, 292);
            this.Label8.Name = "Label8";
            this.Label8.Size = new System.Drawing.Size(29, 13);
            this.Label8.TabIndex = 59;
            this.Label8.Text = "Boot";
            // 
            // ProgressBar1
            // 
            this.ProgressBar1.Location = new System.Drawing.Point(157, 305);
            this.ProgressBar1.Name = "ProgressBar1";
            this.ProgressBar1.Size = new System.Drawing.Size(568, 12);
            this.ProgressBar1.TabIndex = 55;
            // 
            // LedWrite
            // 
            this.LedWrite.BackColor = System.Drawing.Color.Green;
            this.LedWrite.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.LedWrite.Location = new System.Drawing.Point(119, 305);
            this.LedWrite.Name = "LedWrite";
            this.LedWrite.Size = new System.Drawing.Size(29, 12);
            this.LedWrite.TabIndex = 50;
            // 
            // LedErase
            // 
            this.LedErase.BackColor = System.Drawing.Color.Green;
            this.LedErase.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.LedErase.Location = new System.Drawing.Point(84, 305);
            this.LedErase.Name = "LedErase";
            this.LedErase.Size = new System.Drawing.Size(29, 12);
            this.LedErase.TabIndex = 47;
            // 
            // LedNorm
            // 
            this.LedNorm.BackColor = System.Drawing.Color.DarkGreen;
            this.LedNorm.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.LedNorm.Location = new System.Drawing.Point(14, 305);
            this.LedNorm.Name = "LedNorm";
            this.LedNorm.Size = new System.Drawing.Size(29, 12);
            this.LedNorm.TabIndex = 46;
            // 
            // LedBoot
            // 
            this.LedBoot.BackColor = System.Drawing.Color.DarkGreen;
            this.LedBoot.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.LedBoot.Location = new System.Drawing.Point(49, 305);
            this.LedBoot.Name = "LedBoot";
            this.LedBoot.Size = new System.Drawing.Size(29, 12);
            this.LedBoot.TabIndex = 49;
            // 
            // TabControl
            // 
            this.TabControl.Controls.Add(this.TabPage1);
            this.TabControl.Controls.Add(this.TabPage2);
            this.TabControl.Dock = System.Windows.Forms.DockStyle.Fill;
            this.TabControl.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.TabControl.Location = new System.Drawing.Point(0, 0);
            this.TabControl.Name = "TabControl";
            this.TabControl.SelectedIndex = 0;
            this.TabControl.Size = new System.Drawing.Size(741, 358);
            this.TabControl.TabIndex = 1;
            // 
            // cp0
            // 
            this.cp0.AutoSize = true;
            this.cp0.Checked = true;
            this.cp0.CheckState = System.Windows.Forms.CheckState.Checked;
            this.cp0.Location = new System.Drawing.Point(16, 61);
            this.cp0.Name = "cp0";
            this.cp0.Size = new System.Drawing.Size(172, 20);
            this.cp0.TabIndex = 2;
            this.cp0.Text = "Code protection bit(CP0)";
            this.cp0.UseVisualStyleBackColor = true;
            this.cp0.CheckedChanged += new System.EventHandler(this.cp0_CheckedChanged);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.ForeColor = System.Drawing.Color.Blue;
            this.label2.Location = new System.Drawing.Point(193, 133);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(291, 16);
            this.label2.TabIndex = 59;
            this.label2.Text = "Generated firmware version 3.2.82 (Minipro 6.71)";
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(741, 358);
            this.Controls.Add(this.TabControl);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.Name = "MainForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "TL866 firmware updater";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            this.SizeChanged += new System.EventHandler(this.MainForm_SizeChanged);
            this.Resize += new System.EventHandler(this.MainForm_SizeChanged);
            this.StyleChanged += new System.EventHandler(this.MainForm_SizeChanged);
            this.SystemColorsChanged += new System.EventHandler(this.MainForm_SizeChanged);
            this.TabPage2.ResumeLayout(false);
            this.TabPage2.PerformLayout();
            this.GroupBox3.ResumeLayout(false);
            this.GroupBox3.PerformLayout();
            this.GroupBox2.ResumeLayout(false);
            this.GroupBox2.PerformLayout();
            this.Panel1.ResumeLayout(false);
            this.Panel1.PerformLayout();
            this.TabPage1.ResumeLayout(false);
            this.TabPage1.PerformLayout();
            this.GroupBox1.ResumeLayout(false);
            this.GroupBox1.PerformLayout();
            this.TabControl.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        internal System.Windows.Forms.TabPage TabPage2;
        internal System.Windows.Forms.GroupBox GroupBox3;
        internal System.Windows.Forms.Button BtnDefault;
        internal System.Windows.Forms.Button BtnClone;
        internal System.Windows.Forms.Button BtnEdit;
        internal System.Windows.Forms.Label Label5;
        internal System.Windows.Forms.Label Label4;
        internal System.Windows.Forms.TextBox TxtSerial;
        internal System.Windows.Forms.TextBox TxtDevcode;
        internal System.Windows.Forms.GroupBox GroupBox2;
        internal System.Windows.Forms.Panel Panel1;
        internal System.Windows.Forms.RadioButton RadiofCS;
        internal System.Windows.Forms.RadioButton RadiofA;
        internal System.Windows.Forms.Button BtnSave;
        internal System.Windows.Forms.Label Label6;
        internal System.Windows.Forms.RadioButton OptionBoot;
        internal System.Windows.Forms.RadioButton OptionFull;
        internal System.Windows.Forms.TabPage TabPage1;
        internal System.Windows.Forms.Label lblVersion;
        internal System.Windows.Forms.GroupBox GroupBox1;
        internal System.Windows.Forms.Button BtnAdvanced;
        internal System.Windows.Forms.Button BtnDump;
        internal System.Windows.Forms.Button BtnReset;
        internal System.Windows.Forms.Button BtnReflash;
        internal System.Windows.Forms.RadioButton RadioDump;
        internal System.Windows.Forms.RadioButton RadioCS;
        internal System.Windows.Forms.RadioButton RadioA;
        internal System.Windows.Forms.Label Label1;
        internal System.Windows.Forms.Button BtnInput;
        internal System.Windows.Forms.TextBox TxtInput;
        internal System.Windows.Forms.TextBox TxtInfo;
        internal System.Windows.Forms.Label Label13;
        internal System.Windows.Forms.Label Label11;
        internal System.Windows.Forms.Label Label10;
        internal System.Windows.Forms.Label Label8;
        internal System.Windows.Forms.ProgressBar ProgressBar1;
        internal System.Windows.Forms.Label LedWrite;
        internal System.Windows.Forms.Label LedErase;
        internal System.Windows.Forms.Label LedNorm;
        internal System.Windows.Forms.Label LedBoot;
        internal System.Windows.Forms.TabControl TabControl;
        private System.Windows.Forms.CheckBox cp0;
        private System.Windows.Forms.Label label2;

    }
}