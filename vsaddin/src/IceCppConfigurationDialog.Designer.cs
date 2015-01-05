// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice.VisualStudio
{
    partial class IceCppConfigurationDialog
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
            this.components = new System.ComponentModel.Container();
            this.chkEnableBuilder = new System.Windows.Forms.CheckBox();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.chkChecksum = new System.Windows.Forms.CheckBox();
            this.chkIcePrefix = new System.Windows.Forms.CheckBox();
            this.chkStreaming = new System.Windows.Forms.CheckBox();
            this.label1 = new System.Windows.Forms.Label();
            this.comboBoxVerboseLevel = new System.Windows.Forms.ComboBox();
            this.btnCancel = new System.Windows.Forms.Button();
            this.toolTip = new System.Windows.Forms.ToolTip(this.components);
            this.grouDllExportSymbol = new System.Windows.Forms.GroupBox();
            this.txtDllExportSymbol = new System.Windows.Forms.TextBox();
            this.btnOk = new System.Windows.Forms.Button();
            this.btnApply = new System.Windows.Forms.Button();
            this.outputDirView = new Ice.VisualStudio.OutputDirView();
            this.includePathView = new Ice.VisualStudio.IncludePathView();
            this.extraCompilerOptions = new Ice.VisualStudio.ExtraCompilerOptionsView();
            this.groupBox4.SuspendLayout();
            this.grouDllExportSymbol.SuspendLayout();
            this.SuspendLayout();
            // 
            // chkEnableBuilder
            // 
            this.chkEnableBuilder.AutoSize = true;
            this.chkEnableBuilder.Location = new System.Drawing.Point(4, 8);
            this.chkEnableBuilder.Name = "chkEnableBuilder";
            this.chkEnableBuilder.Size = new System.Drawing.Size(112, 17);
            this.chkEnableBuilder.TabIndex = 0;
            this.chkEnableBuilder.Text = "Enable Ice Builder";
            this.chkEnableBuilder.UseVisualStyleBackColor = true;
            this.chkEnableBuilder.CheckedChanged += new System.EventHandler(this.chkEnableBuilder_CheckedChanged);
            // 
            // groupBox4
            // 
            this.groupBox4.Controls.Add(this.chkChecksum);
            this.groupBox4.Controls.Add(this.chkIcePrefix);
            this.groupBox4.Controls.Add(this.chkStreaming);
            this.groupBox4.Location = new System.Drawing.Point(4, 85);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Size = new System.Drawing.Size(487, 44);
            this.groupBox4.TabIndex = 4;
            this.groupBox4.TabStop = false;
            this.groupBox4.Text = "Slice compiler options";
            // 
            // chkChecksum
            // 
            this.chkChecksum.AutoSize = true;
            this.chkChecksum.Location = new System.Drawing.Point(137, 19);
            this.chkChecksum.Name = "chkChecksum";
            this.chkChecksum.Size = new System.Drawing.Size(76, 17);
            this.chkChecksum.TabIndex = 2;
            this.chkChecksum.Text = "Checksum";
            this.chkChecksum.UseVisualStyleBackColor = true;
            this.chkChecksum.CheckedChanged += new System.EventHandler(this.component_Changed);
            // 
            // chkIcePrefix
            // 
            this.chkIcePrefix.AutoSize = true;
            this.chkIcePrefix.Location = new System.Drawing.Point(7, 19);
            this.chkIcePrefix.Name = "chkIcePrefix";
            this.chkIcePrefix.Size = new System.Drawing.Size(41, 17);
            this.chkIcePrefix.TabIndex = 0;
            this.chkIcePrefix.Text = "Ice";
            this.chkIcePrefix.UseVisualStyleBackColor = true;
            this.chkIcePrefix.CheckedChanged += new System.EventHandler(this.component_Changed);
            // 
            // chkStreaming
            // 
            this.chkStreaming.AutoSize = true;
            this.chkStreaming.Location = new System.Drawing.Point(56, 19);
            this.chkStreaming.Name = "chkStreaming";
            this.chkStreaming.Size = new System.Drawing.Size(73, 17);
            this.chkStreaming.TabIndex = 1;
            this.chkStreaming.Text = "Streaming";
            this.chkStreaming.UseVisualStyleBackColor = true;
            this.chkStreaming.CheckedChanged += new System.EventHandler(this.component_Changed);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(297, -62);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(67, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Trace Level:";
            // 
            // comboBoxVerboseLevel
            // 
            this.comboBoxVerboseLevel.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxVerboseLevel.FormattingEnabled = true;
            this.comboBoxVerboseLevel.Items.AddRange(new object[] {
            "Errors Only",
            "Info",
            "Debug"});
            this.comboBoxVerboseLevel.Location = new System.Drawing.Point(370, 6);
            this.comboBoxVerboseLevel.Name = "comboBoxVerboseLevel";
            this.comboBoxVerboseLevel.Size = new System.Drawing.Size(121, 21);
            this.comboBoxVerboseLevel.TabIndex = 2;
            this.comboBoxVerboseLevel.SelectedIndexChanged += new System.EventHandler(this.component_Changed);
            // 
            // btnCancel
            // 
            this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnCancel.Location = new System.Drawing.Point(333, 478);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new System.Drawing.Size(75, 23);
            this.btnCancel.TabIndex = 10;
            this.btnCancel.Text = "Cancel";
            this.btnCancel.UseVisualStyleBackColor = true;
            this.btnCancel.Click += new System.EventHandler(this.btnCancel_Click);
            // 
            // grouDllExportSymbol
            // 
            this.grouDllExportSymbol.Controls.Add(this.txtDllExportSymbol);
            this.grouDllExportSymbol.Location = new System.Drawing.Point(4, 247);
            this.grouDllExportSymbol.Name = "grouDllExportSymbol";
            this.grouDllExportSymbol.Size = new System.Drawing.Size(487, 43);
            this.grouDllExportSymbol.TabIndex = 6;
            this.grouDllExportSymbol.TabStop = false;
            this.grouDllExportSymbol.Text = "Macro for exporting symbols from DLL";
            // 
            // txtDllExportSymbol
            // 
            this.txtDllExportSymbol.Location = new System.Drawing.Point(6, 16);
            this.txtDllExportSymbol.Name = "txtDllExportSymbol";
            this.txtDllExportSymbol.Size = new System.Drawing.Size(474, 20);
            this.txtDllExportSymbol.TabIndex = 0;
            this.txtDllExportSymbol.TextChanged += new System.EventHandler(this.txtDllExportSymbol_Changed);
            this.txtDllExportSymbol.Enter += new System.EventHandler(this.txtDllExportSymbol_Focus);
            this.txtDllExportSymbol.LostFocus += new System.EventHandler(this.txtDllExportSymbol_Changed);
            // 
            // btnOk
            // 
            this.btnOk.Location = new System.Drawing.Point(250, 478);
            this.btnOk.Name = "btnOk";
            this.btnOk.Size = new System.Drawing.Size(75, 23);
            this.btnOk.TabIndex = 9;
            this.btnOk.Text = "OK";
            this.btnOk.UseVisualStyleBackColor = true;
            this.btnOk.Click += new System.EventHandler(this.btnOk_Click);
            // 
            // btnApply
            // 
            this.btnApply.Location = new System.Drawing.Point(416, 478);
            this.btnApply.Name = "btnApply";
            this.btnApply.Size = new System.Drawing.Size(75, 23);
            this.btnApply.TabIndex = 11;
            this.btnApply.Text = "Apply";
            this.btnApply.UseVisualStyleBackColor = true;
            this.btnApply.Click += new System.EventHandler(this.btnApply_Click);
            // 
            // outputDirView
            // 
            this.outputDirView.Location = new System.Drawing.Point(4, 30);
            this.outputDirView.Margin = new System.Windows.Forms.Padding(0);
            this.outputDirView.Name = "outputDirView";
            this.outputDirView.Size = new System.Drawing.Size(487, 47);
            this.outputDirView.TabIndex = 3;
            // 
            // includePathView
            // 
            this.includePathView.Location = new System.Drawing.Point(4, 298);
            this.includePathView.Name = "includePathView";
            this.includePathView.Size = new System.Drawing.Size(487, 168);
            this.includePathView.TabIndex = 7;
            // 
            // extraCompilerOptions
            // 
            this.extraCompilerOptions.Location = new System.Drawing.Point(4, 137);
            this.extraCompilerOptions.Name = "extraCompilerOptions";
            this.extraCompilerOptions.Size = new System.Drawing.Size(487, 102);
            this.extraCompilerOptions.TabIndex = 5;
            // 
            // IceCppConfigurationDialog
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.btnCancel;
            this.ClientSize = new System.Drawing.Size(494, 520);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.comboBoxVerboseLevel);
            this.Controls.Add(this.outputDirView);
            this.Controls.Add(this.includePathView);
            this.Controls.Add(this.extraCompilerOptions);
            this.Controls.Add(this.btnApply);
            this.Controls.Add(this.btnOk);
            this.Controls.Add(this.grouDllExportSymbol);
            this.Controls.Add(this.btnCancel);
            this.Controls.Add(this.groupBox4);
            this.Controls.Add(this.chkEnableBuilder);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.MaximizeBox = false;
            this.Name = "IceCppConfigurationDialog";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Ice Configuration";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.formClosing);
            this.groupBox4.ResumeLayout(false);
            this.groupBox4.PerformLayout();
            this.grouDllExportSymbol.ResumeLayout(false);
            this.grouDllExportSymbol.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

       } 

        #endregion

        private System.Windows.Forms.CheckBox chkEnableBuilder;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.CheckBox chkIcePrefix;
        private System.Windows.Forms.CheckBox chkStreaming;
        private System.Windows.Forms.Button btnCancel;
        private System.Windows.Forms.ToolTip toolTip;
        private System.Windows.Forms.GroupBox grouDllExportSymbol;
        private System.Windows.Forms.TextBox txtDllExportSymbol;
        private System.Windows.Forms.CheckBox chkChecksum;
        private System.Windows.Forms.ComboBox comboBoxVerboseLevel;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button btnOk;
        private System.Windows.Forms.Button btnApply;
        private IncludePathView includePathView;
        private OutputDirView outputDirView;
        private ExtraCompilerOptionsView extraCompilerOptions;
    }
}
