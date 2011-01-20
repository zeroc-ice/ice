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
    public partial class ExtraCompilerOptionsView : UserControl
    {
        public ExtraCompilerOptionsView()
        {
            InitializeComponent();
            toolTip.SetToolTip(txtExtraOptions, "Extra options passed to slice compiler.");
        }

        public void init(IceConfigurationDialog dialog, Project project)
        {
            _dialog = dialog;
            _project = project;
        }

        public void load()
        {
            txtExtraOptions.Text = Util.getProjectProperty(_project, Util.PropertyIceExtraOptions);
        }

        public bool hasUnsavedChanges()
        {
            return !txtExtraOptions.Text.Trim().Equals(Util.getProjectProperty(_project, Util.PropertyIceExtraOptions),
                                                       StringComparison.CurrentCulture);
        }

        public bool apply(ref bool changed)
        {
            changed = false;
            if(hasUnsavedChanges())
            {
                if(!checkExtraOptions())
                {
                    txtExtraOptions.Focus();
                    return false;
                }
                Util.setProjectProperty(_project, Util.PropertyIceExtraOptions, txtExtraOptions.Text);
                if(Util.isCppProject(_project))
                {
                    //
                    // If header or source extension has changed, we need to delete generated items
                    // from the project. The new items will be added when the project is build, on dialog
                    // close.
                    //
                    if(!Util.getHeaderExt(_project).Equals(_headerExt) ||
                       !Util.getSourceExt(_project).Equals(_sourceExt))
                    {
                        Util.cleanProject(_project, true);
                    }
                    Util.setProjectProperty(_project, Util.PropertyIceHeaderExt, _headerExt);
                    Util.setProjectProperty(_project, Util.PropertyIceSourceExt, _sourceExt);
                }
                changed = true;
                _dialog.needSave();
            }
            return true;
        }

        public void setEnabled(bool enabled)
        {
            txtExtraOptions.Enabled = enabled;
        }

        private void txtExtraOptions_Enter(object sender, EventArgs e)
        {
            if(_dialog.editingIncludeDir())
            {
                _dialog.endEditIncludeDir(true);
            }
        }

        private void txtExtraOptions_LostFocus(object sender, EventArgs e)
        {
            _dialog.needSave();
        }

        private void  txtExtraOptions_TextChanged(object sender, EventArgs e)
        {
            _dialog.needSave();
        }

        private bool parseSlice2slOptions()
        {
            Options opts = null;
            return Util.parseSlice2slOptions(txtExtraOptions.Text, true, ref opts);
        }

        private bool parseSlice2csOptions()
        {
            Options opts = null;
            return Util.parseSlice2csOptions(txtExtraOptions.Text, true, ref opts);
        }

        private bool parseSlice2cppOptions()
        {
            Options opts = null;
            _headerExt = ".h";
            _sourceExt = ".cpp";
            return Util.parseSlice2cppOptions(txtExtraOptions.Text, true, ref opts, ref _headerExt, ref _sourceExt);
        }

        private bool checkExtraOptions()
        {
            if(Util.isCppProject(_project))
            {
                return parseSlice2cppOptions();
            }
            else if(Util.isCSharpProject(_project))
            {
                if(Util.isSilverlightProject(_project))
                {
                    return parseSlice2slOptions();
                }
                else
                {
                    return parseSlice2csOptions();
                }
            }
            return true;
        }

        private IceConfigurationDialog _dialog;
        private Project _project;
        private string _headerExt = "h";
        private string _sourceExt = "cpp";
    }
}
