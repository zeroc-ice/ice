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
using Microsoft.VisualStudio.VCProjectEngine;
using Microsoft.VisualStudio.VCProject;
using EnvDTE;

namespace Ice.VisualStudio
{
    public partial class LibraryReferencesForm : Form
    {
        public LibraryReferencesForm(EnvDTE.Project project)
        {
            _project = project;
            _initialized = false;
            InitializeComponent();
            this.CancelButton = closeBtn;
            ComponentList components = Util.getIceCppComponents(_project);
            foreach(String s in components)
            {
                if(String.IsNullOrEmpty(s))
                {
                    continue;
                }
                checkComponent(s);
            }
            _initialized = true;
        }

        private void checkComponent(String name)
        {
            switch (name)
            {
                case "Freeze":
                {
                    chkFreeze.Checked = true;
                    break;
                }
                case "Glacier2":
                {
                    chkGlacier2.Checked = true;
                    break;
                }
                case "Ice":
                {
                    chkIce.Checked = true;
                    break;
                }
                case "IceBox":
                {
                    chkIceBox.Checked = true;
                    break;
                }
                case "IceGrid":
                {
                    chkIceGrid.Checked = true;
                    break;
                }
                case "IcePatch2":
                {
                    chkIcePatch2.Checked = true;
                    break;
                }
                case "IceSSL":
                {
                    chkIceSSL.Checked = true;
                    break;
                }
                case "IceStorm":
                {
                    chkIceStorm.Checked = true;
                    break;
                }
                case "IceUtil":
                {
                    chkIceUtil.Checked = true;
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        private void chkGlacier2_CheckedChanged(object sender, EventArgs e)
        {
            componentChanged("Glacier2", chkGlacier2.Checked);
        }

        private void chkIce_CheckedChanged(object sender, EventArgs e)
        {
            componentChanged("Ice", chkIce.Checked);
        }

        private void chkIceBox_CheckedChanged(object sender, EventArgs e)
        {
            componentChanged("IceBox", chkIceBox.Checked);
        }

        private void chkIceGrid_CheckedChanged(object sender, EventArgs e)
        {
            componentChanged("IceGrid", chkIceGrid.Checked);
        }

        private void chkIcePatch2_CheckedChanged(object sender, EventArgs e)
        {
            componentChanged("IcePatch2", chkIcePatch2.Checked);
        }

        private void chkIceSSL_CheckedChanged(object sender, EventArgs e)
        {
            componentChanged("IceSSL", chkIceSSL.Checked);
        }

        private void chkIceStorm_CheckedChanged(object sender, EventArgs e)
        {
            componentChanged("IceStorm", chkIceStorm.Checked);
        }

        private void chkIceUtil_CheckedChanged(object sender, EventArgs e)
        {
            componentChanged("IceUtil", chkIceUtil.Checked);
        }

        private void chkFreeze_CheckedChanged(object sender, EventArgs e)
        {
            componentChanged("Freeze", chkFreeze.Checked);
        }

        private void componentChanged(String name, bool isChecked)
        {
            if(_initialized)
            {
                if(isChecked)
                {
                    Util.addIceCppLibs(_project, new ComponentList(name));
                }
                else
                {
                    Util.removeIceCppLibs(_project, new ComponentList(name));
                }
            }
        }

        private void closeBtn_Click(object sender, EventArgs e)
        {
            Close();
        }

        private bool _initialized = false;
        private Project _project;
    }
}
