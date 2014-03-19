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
            this.comboBoxConfigurationName = new System.Windows.Forms.ComboBox();
            this.label1 = new System.Windows.Forms.Label();
            this.txtExtraOptions = new System.Windows.Forms.TextBox();
            this.toolTip = new System.Windows.Forms.ToolTip(this.components);
            this.groupExtraOptions.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupExtraOptions
            // 
            this.groupExtraOptions.Controls.Add(this.comboBoxConfigurationName);
            this.groupExtraOptions.Controls.Add(this.label1);
            this.groupExtraOptions.Controls.Add(this.txtExtraOptions);
            this.groupExtraOptions.Location = new System.Drawing.Point(0, 0);
            this.groupExtraOptions.Margin = new System.Windows.Forms.Padding(0);
            this.groupExtraOptions.Name = "groupExtraOptions";
            this.groupExtraOptions.Size = new System.Drawing.Size(487, 102);
            this.groupExtraOptions.TabIndex = 0;
            this.groupExtraOptions.TabStop = false;
            this.groupExtraOptions.Text = "Additional Slice compiler options";
            // 
            // comboBoxConfigurationName
            // 
            this.comboBoxConfigurationName.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxConfigurationName.DropDownWidth = 120;
            this.comboBoxConfigurationName.FormattingEnabled = true;
            this.comboBoxConfigurationName.Location = new System.Drawing.Point(84, 19);
            this.comboBoxConfigurationName.Name = "comboBoxConfigurationName";
            this.comboBoxConfigurationName.Size = new System.Drawing.Size(120, 21);
            this.comboBoxConfigurationName.TabIndex = 2;
            this.comboBoxConfigurationName.SelectedIndexChanged += new System.EventHandler(this.comboBoxConfigurationName_SelectedIndexChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(6, 22);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(72, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Configuration:";
            // 
            // txtExtraOptions
            // 
            this.txtExtraOptions.Location = new System.Drawing.Point(6, 47);
            this.txtExtraOptions.Multiline = true;
            this.txtExtraOptions.Name = "txtExtraOptions";
            this.txtExtraOptions.Size = new System.Drawing.Size(475, 48);
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
            this.Size = new System.Drawing.Size(487, 108);
            this.groupExtraOptions.ResumeLayout(false);
            this.groupExtraOptions.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox groupExtraOptions;
        private System.Windows.Forms.TextBox txtExtraOptions;
        private System.Windows.Forms.ToolTip toolTip;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox comboBoxConfigurationName;
    }
}
