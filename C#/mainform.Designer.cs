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
            this.btnDefault = new System.Windows.Forms.Button();
            this.btnClone = new System.Windows.Forms.Button();
            this.btnEdit = new System.Windows.Forms.Button();
            this.Label5 = new System.Windows.Forms.Label();
            this.Label4 = new System.Windows.Forms.Label();
            this.txtSerial = new System.Windows.Forms.TextBox();
            this.txtDevcode = new System.Windows.Forms.TextBox();
            this.GroupBox2 = new System.Windows.Forms.GroupBox();
            this.Panel1 = new System.Windows.Forms.Panel();
            this.radiofCS = new System.Windows.Forms.RadioButton();
            this.radiofA = new System.Windows.Forms.RadioButton();
            this.btnSave = new System.Windows.Forms.Button();
            this.Label6 = new System.Windows.Forms.Label();
            this.optionBoot = new System.Windows.Forms.RadioButton();
            this.optionFull = new System.Windows.Forms.RadioButton();
            this.TabPage1 = new System.Windows.Forms.TabPage();
            this.lblVersion = new System.Windows.Forms.Label();
            this.GroupBox1 = new System.Windows.Forms.GroupBox();
            this.btnAdvanced = new System.Windows.Forms.Button();
            this.btnDump = new System.Windows.Forms.Button();
            this.btnReset = new System.Windows.Forms.Button();
            this.btnReflash = new System.Windows.Forms.Button();
            this.radioDump = new System.Windows.Forms.RadioButton();
            this.radioCS = new System.Windows.Forms.RadioButton();
            this.radioA = new System.Windows.Forms.RadioButton();
            this.Label1 = new System.Windows.Forms.Label();
            this.btnInput = new System.Windows.Forms.Button();
            this.txtInput = new System.Windows.Forms.TextBox();
            this.txtInfo = new System.Windows.Forms.TextBox();
            this.Label13 = new System.Windows.Forms.Label();
            this.Label11 = new System.Windows.Forms.Label();
            this.Label10 = new System.Windows.Forms.Label();
            this.Label8 = new System.Windows.Forms.Label();
            this.ProgressBar1 = new System.Windows.Forms.ProgressBar();
            this.LedWrite = new System.Windows.Forms.Label();
            this.LedErase = new System.Windows.Forms.Label();
            this.LedNorma = new System.Windows.Forms.Label();
            this.LedBoot = new System.Windows.Forms.Label();
            this.TabControl = new System.Windows.Forms.TabControl();
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
            this.GroupBox3.Controls.Add(this.btnDefault);
            this.GroupBox3.Controls.Add(this.btnClone);
            this.GroupBox3.Controls.Add(this.btnEdit);
            this.GroupBox3.Controls.Add(this.Label5);
            this.GroupBox3.Controls.Add(this.Label4);
            this.GroupBox3.Controls.Add(this.txtSerial);
            this.GroupBox3.Controls.Add(this.txtDevcode);
            this.GroupBox3.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.GroupBox3.Location = new System.Drawing.Point(8, 16);
            this.GroupBox3.Name = "GroupBox3";
            this.GroupBox3.Size = new System.Drawing.Size(717, 108);
            this.GroupBox3.TabIndex = 58;
            this.GroupBox3.TabStop = false;
            this.GroupBox3.Text = "Device Serial number";
            // 
            // btnDefault
            // 
            this.btnDefault.Location = new System.Drawing.Point(631, 55);
            this.btnDefault.Name = "btnDefault";
            this.btnDefault.Size = new System.Drawing.Size(77, 23);
            this.btnDefault.TabIndex = 61;
            this.btnDefault.Text = "Default";
            this.btnDefault.UseVisualStyleBackColor = true;
            this.btnDefault.Click += new System.EventHandler(this.btnDefault_Click);
            // 
            // btnClone
            // 
            this.btnClone.Location = new System.Drawing.Point(548, 55);
            this.btnClone.Name = "btnClone";
            this.btnClone.Size = new System.Drawing.Size(77, 23);
            this.btnClone.TabIndex = 63;
            this.btnClone.Text = "Clone";
            this.btnClone.UseVisualStyleBackColor = true;
            this.btnClone.Click += new System.EventHandler(this.btnClone_Click);
            // 
            // btnEdit
            // 
            this.btnEdit.Location = new System.Drawing.Point(465, 55);
            this.btnEdit.Name = "btnEdit";
            this.btnEdit.Size = new System.Drawing.Size(77, 23);
            this.btnEdit.TabIndex = 64;
            this.btnEdit.Text = "Edit";
            this.btnEdit.UseVisualStyleBackColor = true;
            this.btnEdit.Click += new System.EventHandler(this.btnEdit_Click);
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
            // txtSerial
            // 
            this.txtSerial.BackColor = System.Drawing.SystemColors.Info;
            this.txtSerial.Font = new System.Drawing.Font("Microsoft Sans Serif", 14.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.txtSerial.Location = new System.Drawing.Point(135, 49);
            this.txtSerial.MaxLength = 24;
            this.txtSerial.Name = "txtSerial";
            this.txtSerial.ReadOnly = true;
            this.txtSerial.Size = new System.Drawing.Size(324, 29);
            this.txtSerial.TabIndex = 58;
            this.txtSerial.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.txtSerial.WordWrap = false;
            // 
            // txtDevcode
            // 
            this.txtDevcode.BackColor = System.Drawing.SystemColors.Info;
            this.txtDevcode.Font = new System.Drawing.Font("Microsoft Sans Serif", 14.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.txtDevcode.Location = new System.Drawing.Point(9, 49);
            this.txtDevcode.MaxLength = 8;
            this.txtDevcode.Name = "txtDevcode";
            this.txtDevcode.ReadOnly = true;
            this.txtDevcode.Size = new System.Drawing.Size(117, 29);
            this.txtDevcode.TabIndex = 57;
            this.txtDevcode.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // GroupBox2
            // 
            this.GroupBox2.Controls.Add(this.Panel1);
            this.GroupBox2.Controls.Add(this.btnSave);
            this.GroupBox2.Controls.Add(this.Label6);
            this.GroupBox2.Controls.Add(this.optionBoot);
            this.GroupBox2.Controls.Add(this.optionFull);
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
            this.Panel1.Controls.Add(this.radiofCS);
            this.Panel1.Controls.Add(this.radiofA);
            this.Panel1.Location = new System.Drawing.Point(232, 20);
            this.Panel1.Name = "Panel1";
            this.Panel1.Size = new System.Drawing.Size(479, 100);
            this.Panel1.TabIndex = 58;
            // 
            // radiofCS
            // 
            this.radiofCS.AutoSize = true;
            this.radiofCS.Location = new System.Drawing.Point(16, 35);
            this.radiofCS.Name = "radiofCS";
            this.radiofCS.Size = new System.Drawing.Size(193, 20);
            this.radiofCS.TabIndex = 1;
            this.radiofCS.Text = "Generate TL866CS firmware";
            this.radiofCS.UseVisualStyleBackColor = true;
            // 
            // radiofA
            // 
            this.radiofA.AutoSize = true;
            this.radiofA.Checked = true;
            this.radiofA.Location = new System.Drawing.Point(16, 9);
            this.radiofA.Name = "radiofA";
            this.radiofA.Size = new System.Drawing.Size(184, 20);
            this.radiofA.TabIndex = 0;
            this.radiofA.TabStop = true;
            this.radiofA.Text = "Generate TL866A firmware";
            this.radiofA.UseVisualStyleBackColor = true;
            // 
            // btnSave
            // 
            this.btnSave.Location = new System.Drawing.Point(27, 97);
            this.btnSave.Name = "btnSave";
            this.btnSave.Size = new System.Drawing.Size(75, 23);
            this.btnSave.TabIndex = 57;
            this.btnSave.Text = "Save";
            this.btnSave.UseVisualStyleBackColor = true;
            this.btnSave.Click += new System.EventHandler(this.btnSave_Click);
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
            // optionBoot
            // 
            this.optionBoot.AutoSize = true;
            this.optionBoot.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.optionBoot.Location = new System.Drawing.Point(15, 55);
            this.optionBoot.Name = "optionBoot";
            this.optionBoot.Size = new System.Drawing.Size(121, 20);
            this.optionBoot.TabIndex = 0;
            this.optionBoot.Text = "Bootloader only";
            this.optionBoot.UseVisualStyleBackColor = true;
            // 
            // optionFull
            // 
            this.optionFull.AutoSize = true;
            this.optionFull.Checked = true;
            this.optionFull.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.optionFull.Location = new System.Drawing.Point(15, 29);
            this.optionFull.Name = "optionFull";
            this.optionFull.Size = new System.Drawing.Size(100, 20);
            this.optionFull.TabIndex = 0;
            this.optionFull.TabStop = true;
            this.optionFull.Text = "Full firmware";
            this.optionFull.UseVisualStyleBackColor = true;
            // 
            // TabPage1
            // 
            this.TabPage1.BackColor = System.Drawing.Color.Transparent;
            this.TabPage1.Controls.Add(this.lblVersion);
            this.TabPage1.Controls.Add(this.GroupBox1);
            this.TabPage1.Controls.Add(this.Label1);
            this.TabPage1.Controls.Add(this.btnInput);
            this.TabPage1.Controls.Add(this.txtInput);
            this.TabPage1.Controls.Add(this.txtInfo);
            this.TabPage1.Controls.Add(this.Label13);
            this.TabPage1.Controls.Add(this.Label11);
            this.TabPage1.Controls.Add(this.Label10);
            this.TabPage1.Controls.Add(this.Label8);
            this.TabPage1.Controls.Add(this.ProgressBar1);
            this.TabPage1.Controls.Add(this.LedWrite);
            this.TabPage1.Controls.Add(this.LedErase);
            this.TabPage1.Controls.Add(this.LedNorma);
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
            this.GroupBox1.Controls.Add(this.btnAdvanced);
            this.GroupBox1.Controls.Add(this.btnDump);
            this.GroupBox1.Controls.Add(this.btnReset);
            this.GroupBox1.Controls.Add(this.btnReflash);
            this.GroupBox1.Controls.Add(this.radioDump);
            this.GroupBox1.Controls.Add(this.radioCS);
            this.GroupBox1.Controls.Add(this.radioA);
            this.GroupBox1.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.GroupBox1.Location = new System.Drawing.Point(8, 58);
            this.GroupBox1.Name = "GroupBox1";
            this.GroupBox1.Size = new System.Drawing.Size(715, 110);
            this.GroupBox1.TabIndex = 63;
            this.GroupBox1.TabStop = false;
            this.GroupBox1.Text = "Firmware to write";
            // 
            // btnAdvanced
            // 
            this.btnAdvanced.Enabled = false;
            this.btnAdvanced.Location = new System.Drawing.Point(526, 46);
            this.btnAdvanced.Name = "btnAdvanced";
            this.btnAdvanced.Size = new System.Drawing.Size(79, 23);
            this.btnAdvanced.TabIndex = 63;
            this.btnAdvanced.Text = "Advanced";
            this.btnAdvanced.UseVisualStyleBackColor = true;
            this.btnAdvanced.Click += new System.EventHandler(this.btnAdvanced_Click);
            // 
            // btnDump
            // 
            this.btnDump.Enabled = false;
            this.btnDump.Location = new System.Drawing.Point(445, 46);
            this.btnDump.Name = "btnDump";
            this.btnDump.Size = new System.Drawing.Size(75, 23);
            this.btnDump.TabIndex = 62;
            this.btnDump.Text = "Dump";
            this.btnDump.UseVisualStyleBackColor = true;
            this.btnDump.Click += new System.EventHandler(this.btnDump_Click);
            // 
            // btnReset
            // 
            this.btnReset.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.btnReset.Location = new System.Drawing.Point(364, 46);
            this.btnReset.Name = "btnReset";
            this.btnReset.Size = new System.Drawing.Size(75, 23);
            this.btnReset.TabIndex = 53;
            this.btnReset.Text = "Reset";
            this.btnReset.UseVisualStyleBackColor = true;
            this.btnReset.Click += new System.EventHandler(this.btnReset_Click);
            // 
            // btnReflash
            // 
            this.btnReflash.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.btnReflash.Location = new System.Drawing.Point(283, 46);
            this.btnReflash.Name = "btnReflash";
            this.btnReflash.Size = new System.Drawing.Size(75, 23);
            this.btnReflash.TabIndex = 43;
            this.btnReflash.Text = "Reflash";
            this.btnReflash.UseVisualStyleBackColor = true;
            this.btnReflash.Click += new System.EventHandler(this.btnReflash_Click);
            // 
            // radioDump
            // 
            this.radioDump.AutoSize = true;
            this.radioDump.Location = new System.Drawing.Point(15, 75);
            this.radioDump.Name = "radioDump";
            this.radioDump.Size = new System.Drawing.Size(130, 20);
            this.radioDump.TabIndex = 2;
            this.radioDump.Text = "Firmware dumper";
            this.radioDump.UseVisualStyleBackColor = true;
            // 
            // radioCS
            // 
            this.radioCS.AutoSize = true;
            this.radioCS.Location = new System.Drawing.Point(15, 49);
            this.radioCS.Name = "radioCS";
            this.radioCS.Size = new System.Drawing.Size(134, 20);
            this.radioCS.TabIndex = 1;
            this.radioCS.Text = "TL866CS firmware";
            this.radioCS.UseVisualStyleBackColor = true;
            // 
            // radioA
            // 
            this.radioA.AutoSize = true;
            this.radioA.Checked = true;
            this.radioA.Location = new System.Drawing.Point(15, 23);
            this.radioA.Name = "radioA";
            this.radioA.Size = new System.Drawing.Size(125, 20);
            this.radioA.TabIndex = 0;
            this.radioA.TabStop = true;
            this.radioA.Text = "TL866A firmware";
            this.radioA.UseVisualStyleBackColor = true;
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
            // btnInput
            // 
            this.btnInput.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.btnInput.Location = new System.Drawing.Point(697, 30);
            this.btnInput.Name = "btnInput";
            this.btnInput.Size = new System.Drawing.Size(26, 22);
            this.btnInput.TabIndex = 40;
            this.btnInput.Text = "...";
            this.btnInput.UseVisualStyleBackColor = true;
            this.btnInput.Click += new System.EventHandler(this.btnInput_Click);
            // 
            // txtInput
            // 
            this.txtInput.BackColor = System.Drawing.SystemColors.Info;
            this.txtInput.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.txtInput.Location = new System.Drawing.Point(6, 30);
            this.txtInput.Name = "txtInput";
            this.txtInput.ReadOnly = true;
            this.txtInput.Size = new System.Drawing.Size(680, 22);
            this.txtInput.TabIndex = 38;
            // 
            // txtInfo
            // 
            this.txtInfo.BackColor = System.Drawing.SystemColors.Info;
            this.txtInfo.Font = new System.Drawing.Font("Courier New", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.txtInfo.ForeColor = System.Drawing.SystemColors.WindowText;
            this.txtInfo.Location = new System.Drawing.Point(8, 174);
            this.txtInfo.Multiline = true;
            this.txtInfo.Name = "txtInfo";
            this.txtInfo.ReadOnly = true;
            this.txtInfo.Size = new System.Drawing.Size(717, 115);
            this.txtInfo.TabIndex = 45;
            this.txtInfo.WordWrap = false;
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
            // LedNorma
            // 
            this.LedNorma.BackColor = System.Drawing.Color.DarkGreen;
            this.LedNorma.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.LedNorma.Location = new System.Drawing.Point(14, 305);
            this.LedNorma.Name = "LedNorma";
            this.LedNorma.Size = new System.Drawing.Size(29, 12);
            this.LedNorma.TabIndex = 46;
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
            this.TabPage2.ResumeLayout(false);
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
        internal System.Windows.Forms.Button btnDefault;
        internal System.Windows.Forms.Button btnClone;
        internal System.Windows.Forms.Button btnEdit;
        internal System.Windows.Forms.Label Label5;
        internal System.Windows.Forms.Label Label4;
        internal System.Windows.Forms.TextBox txtSerial;
        internal System.Windows.Forms.TextBox txtDevcode;
        internal System.Windows.Forms.GroupBox GroupBox2;
        internal System.Windows.Forms.Panel Panel1;
        internal System.Windows.Forms.RadioButton radiofCS;
        internal System.Windows.Forms.RadioButton radiofA;
        internal System.Windows.Forms.Button btnSave;
        internal System.Windows.Forms.Label Label6;
        internal System.Windows.Forms.RadioButton optionBoot;
        internal System.Windows.Forms.RadioButton optionFull;
        internal System.Windows.Forms.TabPage TabPage1;
        internal System.Windows.Forms.Label lblVersion;
        internal System.Windows.Forms.GroupBox GroupBox1;
        internal System.Windows.Forms.Button btnAdvanced;
        internal System.Windows.Forms.Button btnDump;
        internal System.Windows.Forms.Button btnReset;
        internal System.Windows.Forms.Button btnReflash;
        internal System.Windows.Forms.RadioButton radioDump;
        internal System.Windows.Forms.RadioButton radioCS;
        internal System.Windows.Forms.RadioButton radioA;
        internal System.Windows.Forms.Label Label1;
        internal System.Windows.Forms.Button btnInput;
        internal System.Windows.Forms.TextBox txtInput;
        internal System.Windows.Forms.TextBox txtInfo;
        internal System.Windows.Forms.Label Label13;
        internal System.Windows.Forms.Label Label11;
        internal System.Windows.Forms.Label Label10;
        internal System.Windows.Forms.Label Label8;
        internal System.Windows.Forms.ProgressBar ProgressBar1;
        internal System.Windows.Forms.Label LedWrite;
        internal System.Windows.Forms.Label LedErase;
        internal System.Windows.Forms.Label LedNorma;
        internal System.Windows.Forms.Label LedBoot;
        internal System.Windows.Forms.TabControl TabControl;

    }
}