using Microsoft.VisualBasic;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Data;
using System.Drawing;
using System.Diagnostics;
using System.Windows.Forms;
namespace InfoIcDump
{
	public partial class Form1 : Form
	{

		//Form overrides dispose to clean up the component list.
		[System.Diagnostics.DebuggerNonUserCode()]
		protected override void Dispose(bool disposing)
		{
			try {
				if (disposing && components != null) {
					components.Dispose();
				}
			} finally {
				base.Dispose(disposing);
			}
		}

		//Required by the Windows Form Designer

		private System.ComponentModel.IContainer components = null;
		//NOTE: The following procedure is required by the Windows Form Designer
		//It can be modified using the Windows Form Designer.  
		//Do not modify it using the code editor.
		[System.Diagnostics.DebuggerStepThrough()]
		private void InitializeComponent()
		{
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            this.LogoImage = new System.Windows.Forms.PictureBox();
            this.Label1 = new System.Windows.Forms.Label();
            this.MfcList = new System.Windows.Forms.ListBox();
            this.DeviceList = new System.Windows.Forms.ListBox();
            this.GroupBox1 = new System.Windows.Forms.GroupBox();
            this.RadioLogic = new System.Windows.Forms.RadioButton();
            this.RadioRam = new System.Windows.Forms.RadioButton();
            this.RadioPld = new System.Windows.Forms.RadioButton();
            this.RadioMcu = new System.Windows.Forms.RadioButton();
            this.RadioRom = new System.Windows.Forms.RadioButton();
            this.RadioAll = new System.Windows.Forms.RadioButton();
            this.SearchBox = new System.Windows.Forms.TextBox();
            this.Label2 = new System.Windows.Forms.Label();
            this.label_total = new System.Windows.Forms.Label();
            this.Button1 = new System.Windows.Forms.Button();
            this.progressBar = new System.Windows.Forms.ProgressBar();
            this.txt_info = new System.Windows.Forms.TextBox();
            this.checkBox1 = new System.Windows.Forms.CheckBox();
            this.checkBox2 = new System.Windows.Forms.CheckBox();
            this.checkBox3 = new System.Windows.Forms.CheckBox();
            this.checkBox4 = new System.Windows.Forms.CheckBox();
            this.label_mfc = new System.Windows.Forms.Label();
            this.label_devs = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.button2 = new System.Windows.Forms.Button();
            this.checkBox5 = new System.Windows.Forms.CheckBox();
            this.checkBox6 = new System.Windows.Forms.CheckBox();
            ((System.ComponentModel.ISupportInitialize)(this.LogoImage)).BeginInit();
            this.GroupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // LogoImage
            // 
            this.LogoImage.BackColor = System.Drawing.Color.Silver;
            this.LogoImage.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.LogoImage.ErrorImage = null;
            this.LogoImage.InitialImage = null;
            this.LogoImage.Location = new System.Drawing.Point(12, 243);
            this.LogoImage.Name = "LogoImage";
            this.LogoImage.Size = new System.Drawing.Size(138, 79);
            this.LogoImage.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
            this.LogoImage.TabIndex = 2;
            this.LogoImage.TabStop = false;
            // 
            // Label1
            // 
            this.Label1.AutoSize = true;
            this.Label1.Font = new System.Drawing.Font("Arial", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.Label1.Location = new System.Drawing.Point(37, 331);
            this.Label1.Name = "Label1";
            this.Label1.Size = new System.Drawing.Size(83, 16);
            this.Label1.TabIndex = 4;
            this.Label1.Text = "manufacturer";
            // 
            // MfcList
            // 
            this.MfcList.Font = new System.Drawing.Font("Courier New", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.MfcList.FormattingEnabled = true;
            this.MfcList.ItemHeight = 16;
            this.MfcList.Location = new System.Drawing.Point(228, 29);
            this.MfcList.Name = "MfcList";
            this.MfcList.Size = new System.Drawing.Size(175, 324);
            this.MfcList.TabIndex = 5;
            this.MfcList.SelectedIndexChanged += new System.EventHandler(this.MfcList_SelectedIndexChanged);
            // 
            // DeviceList
            // 
            this.DeviceList.BackColor = System.Drawing.SystemColors.Window;
            this.DeviceList.Font = new System.Drawing.Font("Courier New", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.DeviceList.FormattingEnabled = true;
            this.DeviceList.ItemHeight = 16;
            this.DeviceList.Location = new System.Drawing.Point(420, 29);
            this.DeviceList.Name = "DeviceList";
            this.DeviceList.Size = new System.Drawing.Size(268, 324);
            this.DeviceList.TabIndex = 6;
            this.DeviceList.SelectedIndexChanged += new System.EventHandler(this.DeviceList_SelectedIndexChanged);
            // 
            // GroupBox1
            // 
            this.GroupBox1.Controls.Add(this.RadioLogic);
            this.GroupBox1.Controls.Add(this.RadioRam);
            this.GroupBox1.Controls.Add(this.RadioPld);
            this.GroupBox1.Controls.Add(this.RadioMcu);
            this.GroupBox1.Controls.Add(this.RadioRom);
            this.GroupBox1.Controls.Add(this.RadioAll);
            this.GroupBox1.Location = new System.Drawing.Point(12, 72);
            this.GroupBox1.Name = "GroupBox1";
            this.GroupBox1.Size = new System.Drawing.Size(193, 165);
            this.GroupBox1.TabIndex = 7;
            this.GroupBox1.TabStop = false;
            this.GroupBox1.Text = "Type";
            // 
            // RadioLogic
            // 
            this.RadioLogic.AutoSize = true;
            this.RadioLogic.Font = new System.Drawing.Font("Arial", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.RadioLogic.Location = new System.Drawing.Point(18, 134);
            this.RadioLogic.Name = "RadioLogic";
            this.RadioLogic.Size = new System.Drawing.Size(70, 19);
            this.RadioLogic.TabIndex = 5;
            this.RadioLogic.Tag = "5";
            this.RadioLogic.Text = "Logic IC";
            this.RadioLogic.UseVisualStyleBackColor = true;
            this.RadioLogic.CheckedChanged += new System.EventHandler(this.RadioButton_CheckedChanged);
            // 
            // RadioRam
            // 
            this.RadioRam.AutoSize = true;
            this.RadioRam.Font = new System.Drawing.Font("Arial", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.RadioRam.Location = new System.Drawing.Point(18, 111);
            this.RadioRam.Name = "RadioRam";
            this.RadioRam.Size = new System.Drawing.Size(95, 19);
            this.RadioRam.TabIndex = 4;
            this.RadioRam.Tag = "4";
            this.RadioRam.Text = "SRAM/DRAM";
            this.RadioRam.UseVisualStyleBackColor = true;
            this.RadioRam.CheckedChanged += new System.EventHandler(this.RadioButton_CheckedChanged);
            // 
            // RadioPld
            // 
            this.RadioPld.AutoSize = true;
            this.RadioPld.Font = new System.Drawing.Font("Arial", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.RadioPld.Location = new System.Drawing.Point(18, 88);
            this.RadioPld.Name = "RadioPld";
            this.RadioPld.Size = new System.Drawing.Size(111, 19);
            this.RadioPld.TabIndex = 3;
            this.RadioPld.Tag = "3";
            this.RadioPld.Text = "PLD/GAL/CPLD";
            this.RadioPld.UseVisualStyleBackColor = true;
            this.RadioPld.CheckedChanged += new System.EventHandler(this.RadioButton_CheckedChanged);
            // 
            // RadioMcu
            // 
            this.RadioMcu.AutoSize = true;
            this.RadioMcu.Font = new System.Drawing.Font("Arial", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.RadioMcu.Location = new System.Drawing.Point(18, 65);
            this.RadioMcu.Name = "RadioMcu";
            this.RadioMcu.Size = new System.Drawing.Size(81, 19);
            this.RadioMcu.TabIndex = 2;
            this.RadioMcu.Tag = "2";
            this.RadioMcu.Text = "MCU/MPU";
            this.RadioMcu.UseVisualStyleBackColor = true;
            this.RadioMcu.CheckedChanged += new System.EventHandler(this.RadioButton_CheckedChanged);
            // 
            // RadioRom
            // 
            this.RadioRom.AutoSize = true;
            this.RadioRom.Font = new System.Drawing.Font("Arial", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.RadioRom.Location = new System.Drawing.Point(18, 42);
            this.RadioRom.Name = "RadioRom";
            this.RadioRom.Size = new System.Drawing.Size(137, 19);
            this.RadioRom.TabIndex = 1;
            this.RadioRom.Tag = "1";
            this.RadioRom.Text = "ROM/FLASH/NVRAM";
            this.RadioRom.UseVisualStyleBackColor = true;
            this.RadioRom.CheckedChanged += new System.EventHandler(this.RadioButton_CheckedChanged);
            // 
            // RadioAll
            // 
            this.RadioAll.AutoSize = true;
            this.RadioAll.Checked = true;
            this.RadioAll.Font = new System.Drawing.Font("Arial", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.RadioAll.Location = new System.Drawing.Point(18, 19);
            this.RadioAll.Name = "RadioAll";
            this.RadioAll.Size = new System.Drawing.Size(46, 19);
            this.RadioAll.TabIndex = 0;
            this.RadioAll.TabStop = true;
            this.RadioAll.Tag = "0";
            this.RadioAll.Text = "ALL";
            this.RadioAll.UseVisualStyleBackColor = true;
            this.RadioAll.CheckedChanged += new System.EventHandler(this.RadioButton_CheckedChanged);
            // 
            // SearchBox
            // 
            this.SearchBox.Font = new System.Drawing.Font("Courier New", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.SearchBox.Location = new System.Drawing.Point(12, 29);
            this.SearchBox.MaxLength = 32;
            this.SearchBox.Name = "SearchBox";
            this.SearchBox.Size = new System.Drawing.Size(193, 22);
            this.SearchBox.TabIndex = 8;
            this.SearchBox.TextChanged += new System.EventHandler(this.RadioButton_CheckedChanged);
            // 
            // Label2
            // 
            this.Label2.AutoSize = true;
            this.Label2.Font = new System.Drawing.Font("Arial", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.Label2.Location = new System.Drawing.Point(12, 12);
            this.Label2.Name = "Label2";
            this.Label2.Size = new System.Drawing.Size(89, 16);
            this.Label2.TabIndex = 9;
            this.Label2.Text = "Search device";
            // 
            // label_total
            // 
            this.label_total.AutoSize = true;
            this.label_total.Font = new System.Drawing.Font("Arial", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.label_total.Location = new System.Drawing.Point(30, 405);
            this.label_total.Name = "label_total";
            this.label_total.Size = new System.Drawing.Size(86, 16);
            this.label_total.TabIndex = 10;
            this.label_total.Text = "Total devices:";
            // 
            // Button1
            // 
            this.Button1.Font = new System.Drawing.Font("Arial", 9.75F);
            this.Button1.Location = new System.Drawing.Point(227, 391);
            this.Button1.Name = "Button1";
            this.Button1.Size = new System.Drawing.Size(75, 27);
            this.Button1.TabIndex = 11;
            this.Button1.Text = "Dump";
            this.Button1.UseVisualStyleBackColor = true;
            this.Button1.Click += new System.EventHandler(this.Button1_Click);
            // 
            // progressBar
            // 
            this.progressBar.Location = new System.Drawing.Point(227, 369);
            this.progressBar.Name = "progressBar";
            this.progressBar.Size = new System.Drawing.Size(732, 10);
            this.progressBar.TabIndex = 13;
            // 
            // txt_info
            // 
            this.txt_info.BackColor = System.Drawing.SystemColors.Info;
            this.txt_info.Font = new System.Drawing.Font("Courier New", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.txt_info.Location = new System.Drawing.Point(710, 29);
            this.txt_info.Multiline = true;
            this.txt_info.Name = "txt_info";
            this.txt_info.ReadOnly = true;
            this.txt_info.Size = new System.Drawing.Size(249, 324);
            this.txt_info.TabIndex = 14;
            // 
            // checkBox1
            // 
            this.checkBox1.AutoSize = true;
            this.checkBox1.Checked = true;
            this.checkBox1.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBox1.Font = new System.Drawing.Font("Arial", 9.75F);
            this.checkBox1.Location = new System.Drawing.Point(322, 395);
            this.checkBox1.Name = "checkBox1";
            this.checkBox1.Size = new System.Drawing.Size(79, 20);
            this.checkBox1.TabIndex = 15;
            this.checkBox1.Text = "C header";
            this.checkBox1.UseVisualStyleBackColor = true;
            this.checkBox1.CheckedChanged += new System.EventHandler(this.checkBox_CheckedChanged);
            // 
            // checkBox2
            // 
            this.checkBox2.AutoSize = true;
            this.checkBox2.Checked = true;
            this.checkBox2.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBox2.Font = new System.Drawing.Font("Arial", 9.75F);
            this.checkBox2.Location = new System.Drawing.Point(407, 395);
            this.checkBox2.Name = "checkBox2";
            this.checkBox2.Size = new System.Drawing.Size(42, 20);
            this.checkBox2.TabIndex = 15;
            this.checkBox2.Text = "INI";
            this.checkBox2.UseVisualStyleBackColor = true;
            this.checkBox2.CheckedChanged += new System.EventHandler(this.checkBox_CheckedChanged);
            // 
            // checkBox3
            // 
            this.checkBox3.AutoSize = true;
            this.checkBox3.Checked = true;
            this.checkBox3.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBox3.Font = new System.Drawing.Font("Arial", 9.75F);
            this.checkBox3.Location = new System.Drawing.Point(456, 395);
            this.checkBox3.Name = "checkBox3";
            this.checkBox3.Size = new System.Drawing.Size(52, 20);
            this.checkBox3.TabIndex = 15;
            this.checkBox3.Text = "XML";
            this.checkBox3.UseVisualStyleBackColor = true;
            this.checkBox3.CheckedChanged += new System.EventHandler(this.checkBox_CheckedChanged);
            // 
            // checkBox4
            // 
            this.checkBox4.AutoSize = true;
            this.checkBox4.Checked = true;
            this.checkBox4.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBox4.Font = new System.Drawing.Font("Arial", 9.75F);
            this.checkBox4.Location = new System.Drawing.Point(520, 395);
            this.checkBox4.Name = "checkBox4";
            this.checkBox4.Size = new System.Drawing.Size(48, 20);
            this.checkBox4.TabIndex = 15;
            this.checkBox4.Text = "Log";
            this.checkBox4.UseVisualStyleBackColor = true;
            this.checkBox4.CheckedChanged += new System.EventHandler(this.checkBox_CheckedChanged);
            // 
            // label_mfc
            // 
            this.label_mfc.AutoSize = true;
            this.label_mfc.Font = new System.Drawing.Font("Arial", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.label_mfc.Location = new System.Drawing.Point(239, 9);
            this.label_mfc.Name = "label_mfc";
            this.label_mfc.Size = new System.Drawing.Size(90, 16);
            this.label_mfc.TabIndex = 16;
            this.label_mfc.Text = "Manufacurers:";
            // 
            // label_devs
            // 
            this.label_devs.AutoSize = true;
            this.label_devs.Font = new System.Drawing.Font("Arial", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.label_devs.Location = new System.Drawing.Point(426, 9);
            this.label_devs.Name = "label_devs";
            this.label_devs.Size = new System.Drawing.Size(57, 16);
            this.label_devs.TabIndex = 16;
            this.label_devs.Text = "Devices:";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Font = new System.Drawing.Font("Arial", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.label6.Location = new System.Drawing.Point(785, 9);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(70, 16);
            this.label6.TabIndex = 16;
            this.label6.Text = "Device info";
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(33, 373);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(99, 27);
            this.button2.TabIndex = 17;
            this.button2.Text = "Load InfoIc.dll";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.button2_Click);
            // 
            // checkBox5
            // 
            this.checkBox5.AutoSize = true;
            this.checkBox5.Checked = true;
            this.checkBox5.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBox5.Font = new System.Drawing.Font("Arial", 9.75F);
            this.checkBox5.Location = new System.Drawing.Point(587, 395);
            this.checkBox5.Name = "checkBox5";
            this.checkBox5.Size = new System.Drawing.Size(170, 20);
            this.checkBox5.TabIndex = 18;
            this.checkBox5.Text = "Remove duplicates(slow)";
            this.checkBox5.UseVisualStyleBackColor = true;
            // 
            // checkBox6
            // 
            this.checkBox6.AutoSize = true;
            this.checkBox6.Checked = true;
            this.checkBox6.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBox6.Font = new System.Drawing.Font("Arial", 9.75F);
            this.checkBox6.Location = new System.Drawing.Point(774, 395);
            this.checkBox6.Name = "checkBox6";
            this.checkBox6.Size = new System.Drawing.Size(98, 20);
            this.checkBox6.TabIndex = 19;
            this.checkBox6.Text = "Sort by type";
            this.checkBox6.UseVisualStyleBackColor = true;
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(977, 430);
            this.Controls.Add(this.checkBox6);
            this.Controls.Add(this.checkBox5);
            this.Controls.Add(this.button2);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.label_devs);
            this.Controls.Add(this.label_mfc);
            this.Controls.Add(this.checkBox4);
            this.Controls.Add(this.checkBox3);
            this.Controls.Add(this.checkBox2);
            this.Controls.Add(this.checkBox1);
            this.Controls.Add(this.txt_info);
            this.Controls.Add(this.progressBar);
            this.Controls.Add(this.Button1);
            this.Controls.Add(this.label_total);
            this.Controls.Add(this.Label2);
            this.Controls.Add(this.SearchBox);
            this.Controls.Add(this.GroupBox1);
            this.Controls.Add(this.DeviceList);
            this.Controls.Add(this.MfcList);
            this.Controls.Add(this.Label1);
            this.Controls.Add(this.LogoImage);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.Name = "Form1";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "InfoIC Dump";
            ((System.ComponentModel.ISupportInitialize)(this.LogoImage)).EndInit();
            this.GroupBox1.ResumeLayout(false);
            this.GroupBox1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

		}
		private System.Windows.Forms.PictureBox LogoImage;
		private System.Windows.Forms.Label Label1;
		private System.Windows.Forms.ListBox MfcList;
        private System.Windows.Forms.ListBox DeviceList;
		private System.Windows.Forms.GroupBox GroupBox1;
		private System.Windows.Forms.RadioButton RadioLogic;
        private System.Windows.Forms.RadioButton RadioRam;
        private System.Windows.Forms.RadioButton RadioPld;
        private System.Windows.Forms.RadioButton RadioMcu;
        private System.Windows.Forms.RadioButton RadioRom;
        private System.Windows.Forms.RadioButton RadioAll;
        private System.Windows.Forms.TextBox SearchBox;
		private System.Windows.Forms.Label Label2;
		private System.Windows.Forms.Label label_total;
        private System.Windows.Forms.Button Button1;
        private ProgressBar progressBar;
        public TextBox txt_info;
        private CheckBox checkBox1;
        private CheckBox checkBox2;
        private CheckBox checkBox3;
        private CheckBox checkBox4;
        private Label label_mfc;
        private Label label_devs;
        private Label label6;
        private Button button2;
        private CheckBox checkBox5;
        private CheckBox checkBox6;
	}
}
