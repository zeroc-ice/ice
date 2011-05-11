// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice.VisualStudio
{
    partial class ExtraCompilerOptionsView
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
            this.groupExtraOptions = new System.Windows.Forms.GroupBox();
            this.txtExtraOptions = new System.Windows.Forms.TextBox();
            this.toolTip = new System.Windows.Forms.ToolTip(this.components);
            this.groupExtraOptions.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupExtraOptions
            // 
            this.groupExtraOptions.Controls.Add(this.txtExtraOptions);
            this.groupExtraOptions.Location = new System.Drawing.Point(5, 3);
            this.groupExtraOptions.Name = "groupExtraOptions";
            this.groupExtraOptions.Size = new System.Drawing.Size(487, 67);
            this.groupExtraOptions.TabIndex = 0;
            this.groupExtraOptions.TabStop = false;
            this.groupExtraOptions.Text = "Extra Compiler Options";
            // 
            // txtExtraOptions
            // 
            this.txtExtraOptions.Location = new System.Drawing.Point(6, 19);
            this.txtExtraOptions.Multiline = true;
            this.txtExtraOptions.Name = "txtExtraOptions";
            this.txtExtraOptions.Size = new System.Drawing.Size(475, 42);
            this.txtExtraOptions.TabIndex = 0;
            this.txtExtraOptions.TextChanged += new System.EventHandler(this.txtExtraOptions_TextChanged);
            this.txtExtraOptions.Enter += new System.EventHandler(this.txtExtraOptions_Enter);
            this.txtExtraOptions.LostFocus += new System.EventHandler(this.txtExtraOptions_LostFocus);
            // 
            // ExtraCompilerOptionsView
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.groupExtraOptions);
            this.Name = "ExtraCompilerOptionsView";
            this.Size = new System.Drawing.Size(495, 75);
            this.groupExtraOptions.ResumeLayout(false);
            this.groupExtraOptions.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox groupExtraOptions;
        private System.Windows.Forms.TextBox txtExtraOptions;
        private System.Windows.Forms.ToolTip toolTip;
    }
}
