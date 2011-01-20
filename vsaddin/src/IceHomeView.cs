// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
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
using System.Windows.Forms;

using EnvDTE;

namespace Ice.VisualStudio
{
    public partial class IceHomeView : UserControl
    {
        public IceHomeView()
        {
            InitializeComponent();
            toolTip.SetToolTip(txtIceHome, "Ice installation directory.");
            toolTip.SetToolTip(btnSelectIceHome, "Ice installation directory.");
        }

        public void init(IceConfigurationDialog dialog, Project project)
        {
            this._dialog = dialog;
            this._project = project;
        }

        public void load()
        {
            txtIceHome.Text = Util.getIceHomeRaw(_project, false);
        }

        public void setEnabled(bool enabled)
        {
            txtIceHome.Enabled = enabled;
            btnSelectIceHome.Enabled = enabled;
        }

        public bool apply(ref bool changed)
        {
            changed = false;
            if(hasUnsavedChanges())
            {
                if(!Util.updateIceHome(_project, txtIceHome.Text.Trim(), false))
                {
                    txtIceHome.Focus();
                    return false;
                }
                changed = true;
            }
            return true;
        }

        public bool hasUnsavedChanges()
        {
            if(!txtIceHome.Text.Trim().Equals(Util.getIceHomeRaw(_project, false),
                                              StringComparison.CurrentCultureIgnoreCase))
            {
                return true;
            }
            return false;
        }

        private void btnSelectIceHome_Click(object sender, EventArgs e)
        {
            if(_dialog.editingIncludeDir())
            {
                _dialog.endEditIncludeDir(true);
            }
            FolderBrowserDialog dialog = new FolderBrowserDialog();
            dialog.SelectedPath = Util.absolutePath(_project, Util.getIceHome(_project));
            dialog.Description = "Select Ice Home Installation Directory";
            DialogResult result = dialog.ShowDialog();
            if(result == DialogResult.OK)
            {
                if(!Util.checkIceHomeAndShowWarning(_project, dialog.SelectedPath))
                {
                    //
                    // If IceHome isn't valid, return focus to the component.
                    //
                    txtIceHome.Focus();
                }
                else
                {
                    txtIceHome.Text = dialog.SelectedPath;
                    _dialog.needSave();
                }
            }
        }

        private void txtIceHome_KeyPress(object sender, KeyPressEventArgs e)
        {
            if(e.KeyChar == (char)Keys.Return)
            {
                checkIceHome();
                e.Handled = true;
            }
        }

        private void txtIceHome_KeyUp(object sender, KeyEventArgs e)
        {
            if(e.KeyCode != Keys.Return)
            {
                _dialog.needSave();
            }
        }

        private void txtIceHome_LostFocus(object sender, EventArgs e)
        {
            _dialog.needSave();
        }

        private void txtIceHome_Focus(object sender, EventArgs e)
        {
            if(_dialog.editingIncludeDir())
            {
                _dialog.endEditIncludeDir(true);
            }
        }

        private void checkIceHome()
        {
            if(hasUnsavedChanges())
            {
                if(!Util.checkIceHomeAndShowWarning(_project, txtIceHome.Text))
                {
                    //
                    // If IceHome isn't valid, return focus to the component.
                    //
                    txtIceHome.Focus();
                    txtIceHome.Text = Util.getProjectProperty(_project, Util.PropertyIceHome);
                }
                _dialog.needSave();
            }
        }

        private IceConfigurationDialog _dialog;
        private Project _project;
    }
}
