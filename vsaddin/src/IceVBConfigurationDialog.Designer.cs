// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice.VisualStudio
{
    partial class IceVBConfigurationDialog
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
            this.chkEnableBuilder = new System.Windows.Forms.CheckBox();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.chkIceStorm = new System.Windows.Forms.CheckBox();
            this.chkIceSSL = new System.Windows.Forms.CheckBox();
            this.chkIcePatch2 = new System.Windows.Forms.CheckBox();
            this.chkIceGrid = new System.Windows.Forms.CheckBox();
            this.chkIceBox = new System.Windows.Forms.CheckBox();
            this.chkGlacier2 = new System.Windows.Forms.CheckBox();
            this.btnCancel = new System.Windows.Forms.Button();
            this.btnApply = new System.Windows.Forms.Button();
            this.btnOk = new System.Windows.Forms.Button();
            this.groupBox3.SuspendLayout();
            this.SuspendLayout();
            // 
            // chkEnableBuilder
            // 
            this.chkEnableBuilder.AutoSize = true;
            this.chkEnableBuilder.Location = new System.Drawing.Point(4, 12);
            this.chkEnableBuilder.Name = "chkEnableBuilder";
            this.chkEnableBuilder.Size = new System.Drawing.Size(112, 17);
            this.chkEnableBuilder.TabIndex = 0;
            this.chkEnableBuilder.Text = "Enable Ice Builder";
            this.chkEnableBuilder.UseVisualStyleBackColor = true;
            this.chkEnableBuilder.CheckedChanged += new System.EventHandler(this.chkEnableBuilder_CheckedChanged);
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.chkIceStorm);
            this.groupBox3.Controls.Add(this.chkIceSSL);
            this.groupBox3.Controls.Add(this.chkIcePatch2);
            this.groupBox3.Controls.Add(this.chkIceGrid);
            this.groupBox3.Controls.Add(this.chkIceBox);
            this.groupBox3.Controls.Add(this.chkGlacier2);
            this.groupBox3.Location = new System.Drawing.Point(4, 36);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(487, 47);
            this.groupBox3.TabIndex = 1;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Add references to the following assemblies:";
            // 
            // chkIceStorm
            // 
            this.chkIceStorm.AutoSize = true;
            this.chkIceStorm.Location = new System.Drawing.Point(366, 19);
            this.chkIceStorm.Name = "chkIceStorm";
            this.chkIceStorm.Size = new System.Drawing.Size(68, 17);
            this.chkIceStorm.TabIndex = 5;
            this.chkIceStorm.Text = "IceStorm";
            this.chkIceStorm.UseVisualStyleBackColor = true;
            this.chkIceStorm.CheckedChanged += new System.EventHandler(this.component_Changed);
            // 
            // chkIceSSL
            // 
            this.chkIceSSL.AutoSize = true;
            this.chkIceSSL.Location = new System.Drawing.Point(297, 19);
            this.chkIceSSL.Name = "chkIceSSL";
            this.chkIceSSL.Size = new System.Drawing.Size(61, 17);
            this.chkIceSSL.TabIndex = 4;
            this.chkIceSSL.Text = "IceSSL";
            this.chkIceSSL.UseVisualStyleBackColor = true;
            this.chkIceSSL.CheckedChanged += new System.EventHandler(this.component_Changed);
            // 
            // chkIcePatch2
            // 
            this.chkIcePatch2.AutoSize = true;
            this.chkIcePatch2.Location = new System.Drawing.Point(214, 19);
            this.chkIcePatch2.Name = "chkIcePatch2";
            this.chkIcePatch2.Size = new System.Drawing.Size(75, 17);
            this.chkIcePatch2.TabIndex = 3;
            this.chkIcePatch2.Text = "IcePatch2";
            this.chkIcePatch2.UseVisualStyleBackColor = true;
            this.chkIcePatch2.CheckedChanged += new System.EventHandler(this.component_Changed);
            // 
            // chkIceGrid
            // 
            this.chkIceGrid.AutoSize = true;
            this.chkIceGrid.Location = new System.Drawing.Point(146, 19);
            this.chkIceGrid.Name = "chkIceGrid";
            this.chkIceGrid.Size = new System.Drawing.Size(60, 17);
            this.chkIceGrid.TabIndex = 2;
            this.chkIceGrid.Text = "IceGrid";
            this.chkIceGrid.UseVisualStyleBackColor = true;
            this.chkIceGrid.CheckedChanged += new System.EventHandler(this.component_Changed);
            // 
            // chkIceBox
            // 
            this.chkIceBox.AutoSize = true;
            this.chkIceBox.Location = new System.Drawing.Point(79, 19);
            this.chkIceBox.Name = "chkIceBox";
            this.chkIceBox.Size = new System.Drawing.Size(59, 17);
            this.chkIceBox.TabIndex = 1;
            this.chkIceBox.Text = "IceBox";
            this.chkIceBox.UseVisualStyleBackColor = true;
            this.chkIceBox.CheckedChanged += new System.EventHandler(this.component_Changed);
            // 
            // chkGlacier2
            // 
            this.chkGlacier2.AutoSize = true;
            this.chkGlacier2.Location = new System.Drawing.Point(6, 19);
            this.chkGlacier2.Name = "chkGlacier2";
            this.chkGlacier2.Size = new System.Drawing.Size(65, 17);
            this.chkGlacier2.TabIndex = 0;
            this.chkGlacier2.Text = "Glacier2";
            this.chkGlacier2.UseVisualStyleBackColor = true;
            this.chkGlacier2.CheckedChanged += new System.EventHandler(this.component_Changed);
            // 
            // btnCancel
            // 
            this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnCancel.Location = new System.Drawing.Point(333, 92);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new System.Drawing.Size(75, 23);
            this.btnCancel.TabIndex = 3;
            this.btnCancel.Text = "Cancel";
            this.btnCancel.UseVisualStyleBackColor = true;
            this.btnCancel.Click += new System.EventHandler(this.btnCancel_Click);
            // 
            // btnApply
            // 
            this.btnApply.Location = new System.Drawing.Point(416, 92);
            this.btnApply.Name = "btnApply";
            this.btnApply.Size = new System.Drawing.Size(75, 23);
            this.btnApply.TabIndex = 4;
            this.btnApply.Text = "Apply";
            this.btnApply.UseVisualStyleBackColor = true;
            this.btnApply.Click += new System.EventHandler(this.btnApply_Click);
            // 
            // btnOk
            // 
            this.btnOk.Location = new System.Drawing.Point(250, 92);
            this.btnOk.Name = "btnOk";
            this.btnOk.Size = new System.Drawing.Size(75, 23);
            this.btnOk.TabIndex = 2;
            this.btnOk.Text = "OK";
            this.btnOk.UseVisualStyleBackColor = true;
            this.btnOk.Click += new System.EventHandler(this.btnOk_Click);
            // 
            // IceVBConfigurationDialog
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.btnCancel;
            this.ClientSize = new System.Drawing.Size(494, 124);
            this.Controls.Add(this.btnOk);
            this.Controls.Add(this.btnApply);
            this.Controls.Add(this.btnCancel);
            this.Controls.Add(this.groupBox3);
            this.Controls.Add(this.chkEnableBuilder);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.MaximizeBox = false;
            this.Name = "IceVBConfigurationDialog";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Ice Configuration";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.formClosing);
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.CheckBox chkEnableBuilder;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.CheckBox chkIceStorm;
        private System.Windows.Forms.CheckBox chkIceSSL;
        private System.Windows.Forms.CheckBox chkIcePatch2;
        private System.Windows.Forms.CheckBox chkIceGrid;
        private System.Windows.Forms.CheckBox chkIceBox;
        private System.Windows.Forms.CheckBox chkGlacier2;
        private System.Windows.Forms.Button btnCancel;
        private System.Windows.Forms.Button btnApply;
        private System.Windows.Forms.Button btnOk;
    }
}
