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
    partial class OutputDirView
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
            if(disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.groupOutputDir = new System.Windows.Forms.GroupBox();
            this.txtOutputDir = new System.Windows.Forms.TextBox();
            this.btnSelectOutputDir = new System.Windows.Forms.Button();
            this.toolTip = new System.Windows.Forms.ToolTip(this.components);
            this.groupOutputDir.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupOutputDir
            // 
            this.groupOutputDir.Controls.Add(this.txtOutputDir);
            this.groupOutputDir.Controls.Add(this.btnSelectOutputDir);
            this.groupOutputDir.Location = new System.Drawing.Point(0, 0);
            this.groupOutputDir.Name = "groupOutputDir";
            this.groupOutputDir.Size = new System.Drawing.Size(487, 45);
            this.groupOutputDir.TabIndex = 0;
            this.groupOutputDir.TabStop = false;
            this.groupOutputDir.Text = "Output directory for generated files";
            // 
            // txtOutputDir
            // 
            this.txtOutputDir.Location = new System.Drawing.Point(6, 17);
            this.txtOutputDir.Name = "txtOutputDir";
            this.txtOutputDir.Size = new System.Drawing.Size(444, 20);
            this.txtOutputDir.TabIndex = 0;
            this.txtOutputDir.KeyUp += new System.Windows.Forms.KeyEventHandler(this.txtOutputDir_KeyUp);
            this.txtOutputDir.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.txtOutputDir_KeyPress);
            this.txtOutputDir.Enter += new System.EventHandler(this.txtOutputDir_Focus);
            this.txtOutputDir.LostFocus += new System.EventHandler(this.txtOutputDir_LostFocus);
            // 
            // btnSelectOutputDir
            // 
            this.btnSelectOutputDir.Location = new System.Drawing.Point(456, 16);
            this.btnSelectOutputDir.Name = "btnSelectOutputDir";
            this.btnSelectOutputDir.Size = new System.Drawing.Size(25, 23);
            this.btnSelectOutputDir.TabIndex = 1;
            this.btnSelectOutputDir.Text = "...";
            this.btnSelectOutputDir.UseVisualStyleBackColor = true;
            this.btnSelectOutputDir.Click += new System.EventHandler(this.btnSelectOutputDir_Click);
            // 
            // OutputDirView
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.groupOutputDir);
            this.Margin = new System.Windows.Forms.Padding(0);
            this.Name = "OutputDirView";
            this.Size = new System.Drawing.Size(487, 49);
            this.groupOutputDir.ResumeLayout(false);
            this.groupOutputDir.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox groupOutputDir;
        private System.Windows.Forms.TextBox txtOutputDir;
        private System.Windows.Forms.Button btnSelectOutputDir;
        private System.Windows.Forms.ToolTip toolTip;
    }
}
