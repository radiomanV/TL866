namespace TL866
{
    partial class EditDialog
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
            this.BtnRndSer = new System.Windows.Forms.Button();
            this.BtnRndDev = new System.Windows.Forms.Button();
            this.Label5 = new System.Windows.Forms.Label();
            this.Label4 = new System.Windows.Forms.Label();
            this.TxtDevcode = new System.Windows.Forms.TextBox();
            this.Cancel_Button = new System.Windows.Forms.Button();
            this.OK_Button = new System.Windows.Forms.Button();
            this.TableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.TxtSerial = new System.Windows.Forms.TextBox();
            this.TableLayoutPanel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // BtnRndSer
            // 
            this.BtnRndSer.Location = new System.Drawing.Point(235, 65);
            this.BtnRndSer.Name = "BtnRndSer";
            this.BtnRndSer.Size = new System.Drawing.Size(75, 23);
            this.BtnRndSer.TabIndex = 71;
            this.BtnRndSer.Text = "Random";
            this.BtnRndSer.UseVisualStyleBackColor = true;
            this.BtnRndSer.Click += new System.EventHandler(this.BtnRndSer_Click);
            // 
            // BtnRndDev
            // 
            this.BtnRndDev.Location = new System.Drawing.Point(32, 65);
            this.BtnRndDev.Name = "BtnRndDev";
            this.BtnRndDev.Size = new System.Drawing.Size(75, 23);
            this.BtnRndDev.TabIndex = 72;
            this.BtnRndDev.Text = "Random";
            this.BtnRndDev.UseVisualStyleBackColor = true;
            this.BtnRndDev.Click += new System.EventHandler(this.BtnRndDev_Click);
            // 
            // Label5
            // 
            this.Label5.AutoSize = true;
            this.Label5.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.Label5.Location = new System.Drawing.Point(232, 11);
            this.Label5.Name = "Label5";
            this.Label5.Size = new System.Drawing.Size(91, 16);
            this.Label5.TabIndex = 70;
            this.Label5.Text = "Serial number";
            // 
            // Label4
            // 
            this.Label4.AutoSize = true;
            this.Label4.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
            this.Label4.Location = new System.Drawing.Point(22, 11);
            this.Label4.Name = "Label4";
            this.Label4.Size = new System.Drawing.Size(85, 16);
            this.Label4.TabIndex = 69;
            this.Label4.Text = "Device code";
            // 
            // TxtDevcode
            // 
            this.TxtDevcode.BackColor = System.Drawing.SystemColors.Info;
            this.TxtDevcode.Font = new System.Drawing.Font("Lucida Sans", 14.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.TxtDevcode.Location = new System.Drawing.Point(9, 30);
            this.TxtDevcode.MaxLength = 8;
            this.TxtDevcode.Name = "TxtDevcode";
            this.TxtDevcode.Size = new System.Drawing.Size(117, 30);
            this.TxtDevcode.TabIndex = 67;
            this.TxtDevcode.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.TxtDevcode.TextChanged += new System.EventHandler(this.TxtDevcode_TextChanged);
            // 
            // Cancel_Button
            // 
            this.Cancel_Button.Anchor = System.Windows.Forms.AnchorStyles.None;
            this.Cancel_Button.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.Cancel_Button.Location = new System.Drawing.Point(76, 3);
            this.Cancel_Button.Name = "Cancel_Button";
            this.Cancel_Button.Size = new System.Drawing.Size(67, 23);
            this.Cancel_Button.TabIndex = 1;
            this.Cancel_Button.Text = "Cancel";
            this.Cancel_Button.Click += new System.EventHandler(this.Cancel_Button_Click);
            // 
            // OK_Button
            // 
            this.OK_Button.Anchor = System.Windows.Forms.AnchorStyles.None;
            this.OK_Button.Location = new System.Drawing.Point(3, 3);
            this.OK_Button.Name = "OK_Button";
            this.OK_Button.Size = new System.Drawing.Size(67, 23);
            this.OK_Button.TabIndex = 0;
            this.OK_Button.Text = "OK";
            this.OK_Button.Click += new System.EventHandler(this.OK_Button_Click);
            // 
            // TableLayoutPanel1
            // 
            this.TableLayoutPanel1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.TableLayoutPanel1.ColumnCount = 2;
            this.TableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
            this.TableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
            this.TableLayoutPanel1.Controls.Add(this.OK_Button, 0, 0);
            this.TableLayoutPanel1.Controls.Add(this.Cancel_Button, 1, 0);
            this.TableLayoutPanel1.Location = new System.Drawing.Point(304, 119);
            this.TableLayoutPanel1.Name = "TableLayoutPanel1";
            this.TableLayoutPanel1.RowCount = 1;
            this.TableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
            this.TableLayoutPanel1.Size = new System.Drawing.Size(146, 29);
            this.TableLayoutPanel1.TabIndex = 66;
            // 
            // TxtSerial
            // 
            this.TxtSerial.BackColor = System.Drawing.SystemColors.Info;
            this.TxtSerial.Font = new System.Drawing.Font("Lucida Sans", 14.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.TxtSerial.Location = new System.Drawing.Point(132, 30);
            this.TxtSerial.MaxLength = 24;
            this.TxtSerial.Name = "TxtSerial";
            this.TxtSerial.Size = new System.Drawing.Size(324, 30);
            this.TxtSerial.TabIndex = 68;
            this.TxtSerial.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.TxtSerial.WordWrap = false;
            // 
            // EditDialog
            // 
            this.AcceptButton = this.OK_Button;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.Cancel_Button;
            this.ClientSize = new System.Drawing.Size(465, 158);
            this.Controls.Add(this.BtnRndSer);
            this.Controls.Add(this.BtnRndDev);
            this.Controls.Add(this.Label5);
            this.Controls.Add(this.Label4);
            this.Controls.Add(this.TxtSerial);
            this.Controls.Add(this.TxtDevcode);
            this.Controls.Add(this.TableLayoutPanel1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "EditDialog";
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Edit";
            this.TableLayoutPanel1.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        internal System.Windows.Forms.Button BtnRndSer;
        internal System.Windows.Forms.Button BtnRndDev;
        internal System.Windows.Forms.Label Label5;
        internal System.Windows.Forms.Label Label4;
        internal System.Windows.Forms.TextBox TxtDevcode;
        internal System.Windows.Forms.Button Cancel_Button;
        internal System.Windows.Forms.Button OK_Button;
        internal System.Windows.Forms.TableLayoutPanel TableLayoutPanel1;
        internal System.Windows.Forms.TextBox TxtSerial;
    }
}