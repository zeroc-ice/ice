// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice.VisualStudio
{
    partial class IceHomeView
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
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.txtIceHome = new System.Windows.Forms.TextBox();
            this.btnSelectIceHome = new System.Windows.Forms.Button();
            this.toolTip = new System.Windows.Forms.ToolTip(this.components);
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.txtIceHome);
            this.groupBox1.Controls.Add(this.btnSelectIceHome);
            this.groupBox1.Location = new System.Drawing.Point(5, 0);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(487, 45);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Ice Home";
            // 
            // txtIceHome
            // 
            this.txtIceHome.Location = new System.Drawing.Point(7, 16);
            this.txtIceHome.Name = "txtIceHome";
            this.txtIceHome.Size = new System.Drawing.Size(393, 20);
            this.txtIceHome.TabIndex = 2;
            this.txtIceHome.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.txtIceHome_KeyPress);
            this.txtIceHome.KeyUp += new System.Windows.Forms.KeyEventHandler(txtIceHome_KeyUp);
            this.txtIceHome.Enter += new System.EventHandler(this.txtIceHome_Focus);
            this.txtIceHome.LostFocus += new System.EventHandler(this.txtIceHome_LostFocus);

            // 
            // btnSelectIceHome
            // 
            this.btnSelectIceHome.Location = new System.Drawing.Point(406, 16);
            this.btnSelectIceHome.Name = "btnSelectIceHome";
            this.btnSelectIceHome.Size = new System.Drawing.Size(75, 23);
            this.btnSelectIceHome.TabIndex = 1;
            this.btnSelectIceHome.Text = "...";
            this.btnSelectIceHome.UseVisualStyleBackColor = true;
            this.btnSelectIceHome.Click += new System.EventHandler(this.btnSelectIceHome_Click);
            // 
            // IceHomeView
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.groupBox1);
            this.Name = "IceHomeView";
            this.Size = new System.Drawing.Size(498, 48);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.TextBox txtIceHome;
        private System.Windows.Forms.Button btnSelectIceHome;
        private System.Windows.Forms.ToolTip toolTip;
    }
}
