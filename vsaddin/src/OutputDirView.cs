// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.IO;
using System.Windows.Forms;
using EnvDTE;

namespace Ice.VisualStudio
{
    public partial class OutputDirView : UserControl
    {
        public OutputDirView()
        {
            InitializeComponent();
            toolTip.SetToolTip(txtOutputDir, "Output directory for Slice compiler generated files.");
            toolTip.SetToolTip(btnSelectOutputDir, "Output directory for Slice compiler generated files.");
        }

        public void init(IceConfigurationDialog dialog, Project project)
        {
            this._dialog = dialog;
            this._project = project;
        }

        public void load()
        {
            txtOutputDir.Text = Util.getProjectOutputDirRaw(_project);
        }

        public void setEnabled(bool enabled)
        {
            txtOutputDir.Enabled = enabled;
            btnSelectOutputDir.Enabled = enabled;
        }

        public bool apply(ref bool changed)
        {
            changed = false;
            if(hasUnsavedChanges())
            {
                if(!Util.updateOutputDir(_project, txtOutputDir.Text))
                {
                    return false;
                }
                changed = true;
            }
            return true;
        }

        public bool hasUnsavedChanges()
        {
            if(!txtOutputDir.Text.Trim().Equals(Util.getProjectOutputDirRaw(_project)))
            {
                return true;
            }
            return false;
        }

        private void btnSelectOutputDir_Click(object sender, EventArgs e)
        {
            if(_dialog.editingIncludeDir())
            {
                _dialog.endEditIncludeDir(true);
            }

            FolderBrowserDialog dialog = new FolderBrowserDialog();
            dialog.SelectedPath = Util.getProjectAbsoluteOutputDir(_project);
            dialog.Description = "Select Base Directory For Slice Generated Files";
            DialogResult result = dialog.ShowDialog();
            if(result == DialogResult.OK)
            {
                txtOutputDir.Text = Util.relativePath(_project, dialog.SelectedPath);
                _dialog.needSave();
            }
        }

        private void txtOutputDir_KeyPress(object sender, KeyPressEventArgs e)
        {
            _editing = true;
            if(e.KeyChar == (char)Keys.Return)
            {
                checkOutputDir();
                e.Handled = true;
            }
        }

        private void txtOutputDir_KeyUp(object sender, KeyEventArgs e)
        {
            _editing = true;
            if(e.KeyCode != Keys.Return)
            {
                _dialog.needSave();
            }
        }

        private void txtOutputDir_Focus(object sender, EventArgs e)
        {
            if(_dialog.editingIncludeDir())
            {
                _dialog.endEditIncludeDir(true);
            }
            _editing = true;
        }

        private void txtOutputDir_LostFocus(object sender, EventArgs e)
        {
            if(_editing)
            {
                checkOutputDir();
            }
        }

        private bool checkOutputDir()
        {
            //
            // Set to false so the loss of focus does not cause another call
            //
            _editing = false;
            if(hasUnsavedChanges())
            {
                _dialog.needSave();
            }
            return true;
        }

        private IceConfigurationDialog _dialog;
        private Project _project;
        private bool _editing = false;
    }
}
