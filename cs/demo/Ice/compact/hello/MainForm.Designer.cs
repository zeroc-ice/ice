// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace client
{
    partial class MainForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;
        private System.Windows.Forms.MainMenu mainMenu1;

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
            this.mainMenu1 = new System.Windows.Forms.MainMenu();
            this.btnHello = new System.Windows.Forms.Button();
            this.btnShutdown = new System.Windows.Forms.Button();
            this.lblStatus = new System.Windows.Forms.Label();
            this.lblHost = new System.Windows.Forms.Label();
            this.hostname = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // btnHello
            // 
            this.btnHello.Location = new System.Drawing.Point(14, 79);
            this.btnHello.Name = "btnHello";
            this.btnHello.Size = new System.Drawing.Size(212, 20);
            this.btnHello.TabIndex = 0;
            this.btnHello.Text = "Say Hello";
            this.btnHello.Click += new System.EventHandler(this.btnHello_Click);
            // 
            // btnShutdown
            // 
            this.btnShutdown.Location = new System.Drawing.Point(14, 109);
            this.btnShutdown.Name = "btnShutdown";
            this.btnShutdown.Size = new System.Drawing.Size(212, 20);
            this.btnShutdown.TabIndex = 1;
            this.btnShutdown.Text = "Shutdown server";
            this.btnShutdown.Click += new System.EventHandler(this.btnShutdown_Click);
            // 
            // lblStatus
            // 
            this.lblStatus.Location = new System.Drawing.Point(14, 139);
            this.lblStatus.Name = "lblStatus";
            this.lblStatus.Size = new System.Drawing.Size(212, 40);
            // 
            // lblHost
            // 
            this.lblHost.Font = new System.Drawing.Font("Tahoma", 9F, System.Drawing.FontStyle.Bold);
            this.lblHost.Location = new System.Drawing.Point(14, 18);
            this.lblHost.Name = "lblHost";
            this.lblHost.Size = new System.Drawing.Size(212, 20);
            this.lblHost.Text = "Hostname:";
            // 
            // hostname
            // 
            this.hostname.Location = new System.Drawing.Point(14, 48);
            this.hostname.Name = "hostname";
            this.hostname.Size = new System.Drawing.Size(212, 21);
            this.hostname.TabIndex = 3;
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(96F, 96F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Dpi;
            this.AutoScroll = true;
            this.ClientSize = new System.Drawing.Size(240, 294);
            this.Controls.Add(this.hostname);
            this.Controls.Add(this.lblHost);
            this.Controls.Add(this.lblStatus);
            this.Controls.Add(this.btnShutdown);
            this.Controls.Add(this.btnHello);
            this.Name = "MainForm";
            this.Text = "Ice Hello Demo";
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button btnHello;
        private System.Windows.Forms.Button btnShutdown;
        private System.Windows.Forms.Label lblStatus;
        private System.Windows.Forms.Label lblHost;
        private System.Windows.Forms.TextBox hostname;
    }
}

