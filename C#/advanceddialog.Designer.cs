namespace TL866
{
    partial class AdvancedDialog
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
            this.btnDefault = new System.Windows.Forms.Button();
            this.btnClone = new System.Windows.Forms.Button();
            this.GroupBox2 = new System.Windows.Forms.GroupBox();
            this.chkCP = new System.Windows.Forms.CheckBox();
            this.btnWriteConfig = new System.Windows.Forms.Button();
            this.btnWriteInfo = new System.Windows.Forms.Button();
            this.OK_Button = new System.Windows.Forms.Button();
            this.btnEdit = new System.Windows.Forms.Button();
            this.Label5 = new System.Windows.Forms.Label();
            this.GroupBox3 = new System.Windows.Forms.GroupBox();
            this.Label4 = new System.Windows.Forms.Label();
            this.txtSerial = new System.Windows.Forms.TextBox();
            this.txtDevcode = new System.Windows.Forms.TextBox();
            this.txtInfo = new System.Windows.Forms.TextBox();
            this.radioCS = new System.Windows.Forms.RadioButton();
            this.GroupBox1 = new System.Windows.Forms.GroupBox();
            this.radioA = new System.Windows.Forms.RadioButton();
            this.btnWriteBootloader = new System.Windows.Forms.Button();
            this.Label1 = new System.Windows.Forms.Label();
            this.GroupBox2.SuspendLayout();
            this.GroupBox3.SuspendLayout();
            this.GroupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // btnDefault
            // 
            this.btnDefault.BackColor = System.Drawing.Color.Transparent;
            this.btnDefault.Location = new System.Drawing.Point(482, 45);
            this.btnDefault.Name = "btnDefault";
            this.btnDefault.Size = new System.Drawing.Size(77, 23);
            this.btnDefault.TabIndex = 61;
            this.btnDefault.Text = "Default";
            this.btnDefault.UseVisualStyleBackColor = false;
            this.btnDefault.Click += new System.EventHandler(this.btnDefault_Click);
            // 
            // btnClone
            // 
            this.btnClone.BackColor = System.Drawing.Color.Transparent;
            this.btnClone.Location = new System.Drawing.Point(399, 45);
            this.btnClone.Name = "btnClone";
            this.btnClone.Size = new System.Drawing.Size(77, 23);
            this.btnClone.TabIndex = 63;
            this.btnClone.Text = "Clone";
            this.btnClone.UseVisualStyleBackColor = false;
            this.btnClone.Click += new System.EventHandler(this.btnClone_Click);
            // 
            // GroupBox2
            // 
            this.GroupBox2.Controls.Add(this.chkCP);
            this.GroupBox2.Controls.Add(this.btnWriteConfig);
            this.GroupBox2.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.GroupBox2.Location = new System.Drawing.Point(341, 97);
            this.GroupBox2.Name = "GroupBox2";
            this.GroupBox2.Size = new System.Drawing.Size(252, 67);
            this.GroupBox2.TabIndex = 66;
            this.GroupBox2.TabStop = false;
            this.GroupBox2.Text = "Copy protection";
            // 
            // chkCP
            // 
            this.chkCP.AutoSize = true;
            this.chkCP.Location = new System.Drawing.Point(107, 26);
            this.chkCP.Name = "chkCP";
            this.chkCP.Size = new System.Drawing.Size(139, 20);
            this.chkCP.TabIndex = 6;
            this.chkCP.Text = "Code protection bit";
            this.chkCP.UseVisualStyleBackColor = true;
            // 
            // btnWriteConfig
            // 
            this.btnWriteConfig.BackColor = System.Drawing.Color.Transparent;
            this.btnWriteConfig.Location = new System.Drawing.Point(16, 23);
            this.btnWriteConfig.Name = "btnWriteConfig";
            this.btnWriteConfig.Size = new System.Drawing.Size(75, 25);
            this.btnWriteConfig.TabIndex = 5;
            this.btnWriteConfig.Text = "Write";
            this.btnWriteConfig.UseVisualStyleBackColor = false;
            this.btnWriteConfig.Click += new System.EventHandler(this.btnWriteConfig_Click);
            // 
            // btnWriteInfo
            // 
            this.btnWriteInfo.BackColor = System.Drawing.Color.Transparent;
            this.btnWriteInfo.Location = new System.Drawing.Point(16, 74);
            this.btnWriteInfo.Name = "btnWriteInfo";
            this.btnWriteInfo.Size = new System.Drawing.Size(75, 23);
            this.btnWriteInfo.TabIndex = 65;
            this.btnWriteInfo.Text = "Write";
            this.btnWriteInfo.UseVisualStyleBackColor = false;
            this.btnWriteInfo.Click += new System.EventHandler(this.btnWriteInfo_Click);
            // 
            // OK_Button
            // 
            this.OK_Button.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.OK_Button.BackColor = System.Drawing.Color.Transparent;
            this.OK_Button.Location = new System.Drawing.Point(518, 288);
            this.OK_Button.Name = "OK_Button";
            this.OK_Button.Size = new System.Drawing.Size(75, 23);
            this.OK_Button.TabIndex = 62;
            this.OK_Button.Text = "OK";
            this.OK_Button.UseVisualStyleBackColor = false;
            this.OK_Button.Click += new System.EventHandler(this.OK_Button_Click);
            // 
            // btnEdit
            // 
            this.btnEdit.BackColor = System.Drawing.Color.Transparent;
            this.btnEdit.Location = new System.Drawing.Point(316, 45);
            this.btnEdit.Name = "btnEdit";
            this.btnEdit.Size = new System.Drawing.Size(77, 23);
            this.btnEdit.TabIndex = 64;
            this.btnEdit.Text = "Edit";
            this.btnEdit.UseVisualStyleBackColor = false;
            this.btnEdit.Click += new System.EventHandler(this.btnEdit_Click);
            // 
            // Label5
            // 
            this.Label5.AutoSize = true;
            this.Label5.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.Label5.Location = new System.Drawing.Point(131, 27);
            this.Label5.Name = "Label5";
            this.Label5.Size = new System.Drawing.Size(91, 16);
            this.Label5.TabIndex = 60;
            this.Label5.Text = "Serial number";
            // 
            // GroupBox3
            // 
            this.GroupBox3.Controls.Add(this.btnWriteInfo);
            this.GroupBox3.Controls.Add(this.btnDefault);
            this.GroupBox3.Controls.Add(this.btnClone);
            this.GroupBox3.Controls.Add(this.btnEdit);
            this.GroupBox3.Controls.Add(this.Label5);
            this.GroupBox3.Controls.Add(this.Label4);
            this.GroupBox3.Controls.Add(this.txtSerial);
            this.GroupBox3.Controls.Add(this.txtDevcode);
            this.GroupBox3.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.GroupBox3.Location = new System.Drawing.Point(12, 170);
            this.GroupBox3.Name = "GroupBox3";
            this.GroupBox3.Size = new System.Drawing.Size(581, 111);
            this.GroupBox3.TabIndex = 65;
            this.GroupBox3.TabStop = false;
            this.GroupBox3.Text = "Device Serial number";
            // 
            // Label4
            // 
            this.Label4.AutoSize = true;
            this.Label4.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.Label4.Location = new System.Drawing.Point(7, 27);
            this.Label4.Name = "Label4";
            this.Label4.Size = new System.Drawing.Size(85, 16);
            this.Label4.TabIndex = 59;
            this.Label4.Text = "Device code";
            // 
            // txtSerial
            // 
            this.txtSerial.BackColor = System.Drawing.SystemColors.Info;
            this.txtSerial.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.txtSerial.Location = new System.Drawing.Point(97, 46);
            this.txtSerial.MaxLength = 24;
            this.txtSerial.Name = "txtSerial";
            this.txtSerial.ReadOnly = true;
            this.txtSerial.Size = new System.Drawing.Size(202, 22);
            this.txtSerial.TabIndex = 58;
            this.txtSerial.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.txtSerial.WordWrap = false;
            // 
            // txtDevcode
            // 
            this.txtDevcode.BackColor = System.Drawing.SystemColors.Info;
            this.txtDevcode.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.txtDevcode.Location = new System.Drawing.Point(10, 46);
            this.txtDevcode.MaxLength = 8;
            this.txtDevcode.Name = "txtDevcode";
            this.txtDevcode.ReadOnly = true;
            this.txtDevcode.Size = new System.Drawing.Size(81, 22);
            this.txtDevcode.TabIndex = 57;
            this.txtDevcode.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // txtInfo
            // 
            this.txtInfo.BackColor = System.Drawing.SystemColors.Info;
            this.txtInfo.Font = new System.Drawing.Font("Courier New", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.txtInfo.Location = new System.Drawing.Point(12, 12);
            this.txtInfo.Multiline = true;
            this.txtInfo.Name = "txtInfo";
            this.txtInfo.ReadOnly = true;
            this.txtInfo.Size = new System.Drawing.Size(581, 79);
            this.txtInfo.TabIndex = 63;
            // 
            // radioCS
            // 
            this.radioCS.AutoSize = true;
            this.radioCS.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.radioCS.Location = new System.Drawing.Point(207, 26);
            this.radioCS.Name = "radioCS";
            this.radioCS.Size = new System.Drawing.Size(114, 20);
            this.radioCS.TabIndex = 6;
            this.radioCS.Text = "CS Bootloader";
            this.radioCS.UseVisualStyleBackColor = true;
            // 
            // GroupBox1
            // 
            this.GroupBox1.Controls.Add(this.radioCS);
            this.GroupBox1.Controls.Add(this.radioA);
            this.GroupBox1.Controls.Add(this.btnWriteBootloader);
            this.GroupBox1.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.GroupBox1.Location = new System.Drawing.Point(12, 97);
            this.GroupBox1.Name = "GroupBox1";
            this.GroupBox1.Size = new System.Drawing.Size(323, 67);
            this.GroupBox1.TabIndex = 64;
            this.GroupBox1.TabStop = false;
            this.GroupBox1.Text = "Bootloader";
            // 
            // radioA
            // 
            this.radioA.AutoSize = true;
            this.radioA.Checked = true;
            this.radioA.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.radioA.Location = new System.Drawing.Point(97, 26);
            this.radioA.Name = "radioA";
            this.radioA.Size = new System.Drawing.Size(105, 20);
            this.radioA.TabIndex = 7;
            this.radioA.TabStop = true;
            this.radioA.Text = "A Bootloader";
            this.radioA.UseVisualStyleBackColor = true;
            // 
            // btnWriteBootloader
            // 
            this.btnWriteBootloader.BackColor = System.Drawing.Color.Transparent;
            this.btnWriteBootloader.Location = new System.Drawing.Point(16, 23);
            this.btnWriteBootloader.Name = "btnWriteBootloader";
            this.btnWriteBootloader.Size = new System.Drawing.Size(75, 25);
            this.btnWriteBootloader.TabIndex = 5;
            this.btnWriteBootloader.Text = "Write";
            this.btnWriteBootloader.UseVisualStyleBackColor = false;
            this.btnWriteBootloader.Click += new System.EventHandler(this.btnWriteBootloader_Click);
            // 
            // Label1
            // 
            this.Label1.AutoSize = true;
            this.Label1.Font = new System.Drawing.Font("Courier New", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.Label1.ForeColor = System.Drawing.Color.Red;
            this.Label1.Location = new System.Drawing.Point(25, 293);
            this.Label1.Name = "Label1";
            this.Label1.Size = new System.Drawing.Size(328, 16);
            this.Label1.TabIndex = 67;
            this.Label1.Text = "Warning! You can brick your device here.";
            // 
            // AdvancedDialog
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(604, 323);
            this.Controls.Add(this.GroupBox2);
            this.Controls.Add(this.OK_Button);
            this.Controls.Add(this.GroupBox3);
            this.Controls.Add(this.txtInfo);
            this.Controls.Add(this.GroupBox1);
            this.Controls.Add(this.Label1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "AdvancedDialog";
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Advanced";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Dialog2_FormClosing);
            this.GroupBox2.ResumeLayout(false);
            this.GroupBox2.PerformLayout();
            this.GroupBox3.ResumeLayout(false);
            this.GroupBox3.PerformLayout();
            this.GroupBox1.ResumeLayout(false);
            this.GroupBox1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        internal System.Windows.Forms.Button btnDefault;
        internal System.Windows.Forms.Button btnClone;
        internal System.Windows.Forms.GroupBox GroupBox2;
        internal System.Windows.Forms.CheckBox chkCP;
        internal System.Windows.Forms.Button btnWriteConfig;
        internal System.Windows.Forms.Button btnWriteInfo;
        internal System.Windows.Forms.Button OK_Button;
        internal System.Windows.Forms.Button btnEdit;
        internal System.Windows.Forms.Label Label5;
        internal System.Windows.Forms.GroupBox GroupBox3;
        internal System.Windows.Forms.Label Label4;
        internal System.Windows.Forms.TextBox txtSerial;
        internal System.Windows.Forms.TextBox txtDevcode;
        internal System.Windows.Forms.TextBox txtInfo;
        internal System.Windows.Forms.RadioButton radioCS;
        internal System.Windows.Forms.GroupBox GroupBox1;
        internal System.Windows.Forms.RadioButton radioA;
        internal System.Windows.Forms.Button btnWriteBootloader;
        internal System.Windows.Forms.Label Label1;
    }
}