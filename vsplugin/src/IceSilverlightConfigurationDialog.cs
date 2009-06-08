// **********************************************************************
//
// Copyright (c) 2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Text;
using System.Windows.Forms;

using EnvDTE;

namespace Ice.VisualStudio
{
    public partial class IceSilverlightConfigurationDialog : Form
    {
        public IceSilverlightConfigurationDialog(Project project)
        {
            InitializeComponent();
            _project = project;
            
            //
            // Set the toolTip messages.
            //
            toolTip.SetToolTip(txtIceHome, "Ice Installation Directory.");
            toolTip.SetToolTip(btnSelectIceHome, "Ice Installation Directory.");
            toolTip.SetToolTip(chkStreaming, "Geneate Streaming API (--stream).");
            toolTip.SetToolTip(chkIcePrefix, "Allow Ice prefix (--ice).");
            
            toolTip.SetToolTip(btnClose, "Close without save configuration changes.");

            if(_project != null)
            {
                this.Text = "Ice Configuration - Project: " + _project.Name;
                bool enabled = Util.isSliceBuilderEnabled(project);
                setEnabled(enabled);
                chkEnableBuilder.Checked = enabled;
                load();
                _initialized = true;
            }
        }
        
        private void load()
        {
            if (_project != null)
            {
                System.Windows.Forms.Cursor c = Cursor.Current;
                Cursor = Cursors.WaitCursor;
                includeDirList.Items.Clear();
                txtIceHome.Text = Util.getIceHome(_project);
                txtMacros.Text = Util.getProjectProperty(_project, Util.PropertyNames.IceMacros);

                chkIcePrefix.Checked = Util.getProjectPropertyAsBool(_project, Util.PropertyNames.IcePrefix);
                chkStreaming.Checked = Util.getProjectPropertyAsBool(_project, Util.PropertyNames.IceStreaming);
                chkConsole.Checked = Util.getProjectPropertyAsBool(_project, Util.PropertyNames.ConsoleOutput);
                
                IncludePathList list =
                    new IncludePathList(Util.getProjectProperty(_project, Util.PropertyNames.IceIncludePath));
                foreach (String s in list)
                {
                    includeDirList.Items.Add(s.Trim());
                }


                ComponentList selectedComponents = Util.getIceSilverlightComponents(_project);
                foreach (String s in Util.ComponentNames.silverlightNames)
                {
                    if(String.IsNullOrEmpty(selectedComponents.Find(delegate(string d)
                                                    {
                                                        return d.Equals(s, StringComparison.CurrentCultureIgnoreCase);
                                                    })))
                    {
                        checkComponent(s, false);
                    }
                    else
                    {
                        checkComponent(s, true);
                    }
                }
                Cursor = c;
            }
        
        }

        private void checkComponent(String component, bool check)
        {
            switch (component)
            {
            case "IceSL":
            {
                chkIceSl.Checked = check;
                break;
            }
            default:
            {
                break;
            }
            }
        }
        private void chkEnableBuilder_CheckedChanged(object sender, EventArgs e)
        {
            System.Windows.Forms.Cursor c = Cursor.Current;
            Cursor = Cursors.WaitCursor;
            if(_initialized)
            {
                _initialized = false;
                setEnabled(false);
                chkEnableBuilder.Enabled = false;
                Builder builder = Connect.getBuilder();
                if(chkEnableBuilder.Checked)
                {
                    builder.addBuilderToProject(_project);
                }
                else
                {
                    builder.removeBuilderFromProject(_project);
                }
                load();
                chkEnableBuilder.Enabled = true;
                setEnabled(chkEnableBuilder.Checked);
                _initialized = true;
            }
            Cursor = c;
        }
        
        private void setEnabled(bool enabled)
        {
            Util.setProjectProperty(_project, Util.PropertyNames.Ice, enabled.ToString());
            txtIceHome.Enabled = enabled;
            btnSelectIceHome.Enabled = enabled;

            chkIcePrefix.Enabled = enabled;
            chkStreaming.Enabled = enabled;
            chkConsole.Enabled = enabled;
            
            includeDirList.Enabled = enabled;
            btnAddInclude.Enabled = enabled;
            btnRemoveInclude.Enabled = enabled;
            btnMoveIncludeUp.Enabled = enabled;
            btnMoveIncludeDown.Enabled = enabled;

            txtMacros.Enabled = enabled;


            chkIceSl.Enabled = enabled;
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            if (_changed)
            {
                System.Windows.Forms.Cursor c = Cursor.Current;
                Cursor = Cursors.WaitCursor;
                Builder builder = Connect.getBuilder();
                builder.cleanProject(_project);
                builder.buildCSharpProject(_project, false, true);
                Cursor = c;
            }
            Close();
        }

        private void btnSelectIceHome_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog dialog = new FolderBrowserDialog();
            dialog.SelectedPath = Util.getAbsoluteIceHome(_project);
            dialog.Description = "Select Ice Home Installation Directory";
            DialogResult result = dialog.ShowDialog();
            if (result == DialogResult.OK)
            {
                Util.updateIceHome(_project, dialog.SelectedPath);
                load();
                _changed = true;
            }
        }

        private void chkIcePrefix_CheckedChanged(object sender, EventArgs e)
        {
            System.Windows.Forms.Cursor c = Cursor.Current;
            Cursor = Cursors.WaitCursor;
            Util.setProjectProperty(_project, Util.PropertyNames.IcePrefix, chkIcePrefix.Checked.ToString());
            _changed = true;
            Cursor = c;
        }

        private void chkStreaming_CheckedChanged(object sender, EventArgs e)
        {
            System.Windows.Forms.Cursor c = Cursor.Current;
            Cursor = Cursors.WaitCursor;
            Util.setProjectProperty(_project, Util.PropertyNames.IceStreaming, chkStreaming.Checked.ToString());
            _changed = true;
            Cursor = c;
        }
        
        private void saveSliceIncludes()
        {
            System.Windows.Forms.Cursor c = Cursor.Current;
            Cursor = Cursors.WaitCursor;
            IncludePathList paths = new IncludePathList();
            foreach (String s in includeDirList.Items)
            {
                paths.Add(s.Trim());
            }
            Util.setProjectProperty(_project, Util.PropertyNames.IceIncludePath, paths.ToString());
            _changed = true;
            Cursor = c;
        }

        private void btnAddInclude_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog dialog = new FolderBrowserDialog();
            string projectDir = Path.GetDirectoryName(_project.FileName);
            dialog.SelectedPath = projectDir;
            dialog.Description = "Slice Include Directory";
            DialogResult result = dialog.ShowDialog();
            if (result == DialogResult.OK)
            {
                if (!dialog.SelectedPath.Contains(projectDir))
                {
                    includeDirList.Items.Add(dialog.SelectedPath);
                }
                else
                {
                    includeDirList.Items.Add(Util.relativePath(projectDir, dialog.SelectedPath));
                }
                
                includeDirList.SelectedIndex = includeDirList.Items.Count - 1;
                saveSliceIncludes();
            }
        }

        private void btnRemoveInclude_Click(object sender, EventArgs e)
        {
            if (includeDirList.SelectedIndex != -1)
            {
                System.Windows.Forms.Cursor c = Cursor.Current;
                Cursor = Cursors.WaitCursor;
                int selected = includeDirList.SelectedIndex;
                includeDirList.Items.RemoveAt(selected);
                if(includeDirList.Items.Count > 0)
                {
                    if(selected > 0)
                    {
                        selected -= 1;
                    }
                    includeDirList.SelectedIndex = selected;
                }
                saveSliceIncludes();
                Cursor = c;
            }
        }

        private void btnMoveIncludeUp_Click(object sender, EventArgs e)
        {
            int index = includeDirList.SelectedIndex;
            if (index > 0)
            {
                System.Windows.Forms.Cursor c = Cursor.Current;
                Cursor = Cursors.WaitCursor;
                string current = includeDirList.SelectedItem.ToString();
                includeDirList.Items.RemoveAt(index);
                includeDirList.Items.Insert(index - 1, current);
                includeDirList.SelectedIndex = index - 1;
                saveSliceIncludes();
                Cursor = c;
            }
        }

        private void btnMoveIncludeDown_Click(object sender, EventArgs e)
        {
            int index = includeDirList.SelectedIndex;
            if (index < includeDirList.Items.Count - 1)
            {
                System.Windows.Forms.Cursor c = Cursor.Current;
                Cursor = Cursors.WaitCursor;
                string current = includeDirList.SelectedItem.ToString();
                includeDirList.Items.RemoveAt(index);
                includeDirList.Items.Insert(index + 1, current);
                includeDirList.SelectedIndex = index + 1;
                saveSliceIncludes();
                Cursor = c;
            }
        }

        private void txtMacros_LostFocus(object sender, EventArgs e)
        {
            if(txtMacros.Modified)
            {
                if(_initialized)
                {
                    Util.setProjectProperty(_project, Util.PropertyNames.IceMacros, txtMacros.Text);
                    _changed = true;
                }
            }
        }


        private void chkIce_CheckedChanged(object sender, EventArgs e)
        {
            System.Windows.Forms.Cursor c = Cursor.Current;
            Cursor = Cursors.WaitCursor;
            if(_initialized)
            {
                if (chkIceSl.Checked)
                {
                    Util.addCSharpReference(_project, "IceSL");
                }
                else
                {
                    Util.removeCSharpReference(_project, "IceSL");
                }
                _changed = true;
            }
            Cursor = c;
        }

        private void chkConsole_CheckedChanged(object sender, EventArgs e)
        {
            System.Windows.Forms.Cursor c = Cursor.Current;
            Cursor = Cursors.WaitCursor;
            Util.setProjectProperty(_project, Util.PropertyNames.ConsoleOutput, chkConsole.Checked.ToString());
            Cursor = c;
        }
        
        private bool _initialized = false;
        private bool _changed = false;
        private Project _project;
    }
}
