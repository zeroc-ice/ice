// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.IO;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using EnvDTE;

namespace Ice.VisualStudio
{
    public partial class ProductReferencesForm : Form
    {
        public ProductReferencesForm(Project project)
        {
            InitializeComponent();
            this.CancelButton = closeBtn;
            _project = project;
            ComponentList selectedComponents = Util.getIceCSharpComponents(_project);
            foreach(String s in selectedComponents)
            {
                if(String.IsNullOrEmpty(s))
                {
                    continue;
                }
                checkComponent(s);
            }
        }

        private void checkComponent(String component)
        {
            switch (component)
            {
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
            default:
            {
                break;
            }
            }
        }

        private void chkGlacier2_CheckedChanged(object sender, EventArgs e)
        {
            if(chkGlacier2.Checked)
            {
                Util.addCSharpReference(_project, "Glacier2");
            }
            else
            {
                Util.addCSharpReference(_project, "Glacier");
            }
        }

        private void chkIce_CheckedChanged(object sender, EventArgs e)
        {
            if(chkIce.Checked)
            {
                Util.addCSharpReference(_project, "Ice");
            }
            else
            {
                Util.removeCSharpReference(_project, "Ice");
            }
        }

        private void chkIceBox_CheckedChanged(object sender, EventArgs e)
        {
            if(chkIceBox.Checked)
            {
                Util.addCSharpReference(_project, "IceBox");
            }
            else
            {
                Util.removeCSharpReference(_project, "IceBox");
            }
        }

        private void chkIceGrid_CheckedChanged(object sender, EventArgs e)
        {
            if(chkIceGrid.Checked)
            {
                Util.addCSharpReference(_project, "IceGrid");
            }
            else
            {
                Util.removeCSharpReference(_project, "IceGrid");
            }
        }

        private void chkIcePatch2_CheckedChanged(object sender, EventArgs e)
        {
            if(chkIcePatch2.Checked)
            {
                Util.addCSharpReference(_project, "IcePatch2");
            }
            else
            {
                Util.removeCSharpReference(_project, "IcePatch2");
            }
        }

        private void chkIceSSL_CheckedChanged(object sender, EventArgs e)
        {
            if(chkIceSSL.Checked)
            {
                Util.addCSharpReference(_project, "IceSSL");
            }
            else
            {
                Util.removeCSharpReference(_project, "IceSSL");
            }
        }

        private void chkIceStorm_CheckedChanged(object sender, EventArgs e)
        {
            if(chkIceStorm.Checked)
            {
                Util.addCSharpReference(_project, "IceStorm");
            }
            else
            {
                Util.removeCSharpReference(_project, "IceStorm");
            }
        }

        private void closeBtn_Click(object sender, EventArgs e)
        {
            Close();
        }

        private Project _project;
    }
}
