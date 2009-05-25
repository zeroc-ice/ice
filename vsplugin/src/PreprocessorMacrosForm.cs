// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using EnvDTE;
namespace Ice.VisualStudio
{
    public partial class PreprocessorMacrosForm : Form
    {
        public PreprocessorMacrosForm(Project project, String macros)
        {
            InitializeComponent();
            this.CancelButton = btnCancel;
            this.AcceptButton = btnOk;
            txtMacros.Text = macros;
            _project = project;
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void btnOk_Click(object sender, EventArgs e)
        {
            Util.setProjectProperty(_project, Util.PropertyNames.IceMacros, txtMacros.Text);
            Close();
        }

        private EnvDTE.Project _project;
    }
}
