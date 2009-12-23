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
using System.IO;
using System.Text;
using System.Windows.Forms;

using EnvDTE;

namespace Ice.VisualStudio
{
    public partial class IceCsharpConfigurationDialog : Form
    {
        public IceCsharpConfigurationDialog(Project project)
        {
            InitializeComponent();
            _project = project;
            
            //
            // Set the toolTip messages.
            //
            toolTip.SetToolTip(txtIceHome, "Ice installation directory.");
            toolTip.SetToolTip(btnSelectIceHome, "Ice installation directory.");
            toolTip.SetToolTip(chkStreaming, "Generate marshaling support for stream API (--stream).");
            toolTip.SetToolTip(chkChecksum, "Generate checksums for Slice definitions (--checksum).");
            toolTip.SetToolTip(chkIcePrefix, "Permit Ice prefixes (--ice).");
            toolTip.SetToolTip(chkTie, "Generate TIE classes (--tie).");
            toolTip.SetToolTip(chkConsole, "Enable console output.");

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
            Cursor = Cursors.WaitCursor;
            if(_project != null)
            {
                includeDirList.Items.Clear();
                txtIceHome.Text = Util.getIceHomeRaw(_project, false);
                txtExtraOptions.Text = Util.getProjectProperty(_project, Util.PropertyIceExtraOptions);

                chkIcePrefix.Checked = Util.getProjectPropertyAsBool(_project, Util.PropertyIcePrefix);
                chkTie.Checked = Util.getProjectPropertyAsBool(_project, Util.PropertyIceTie);
                chkStreaming.Checked = Util.getProjectPropertyAsBool(_project, Util.PropertyIceStreaming);
                chkChecksum.Checked = Util.getProjectPropertyAsBool(_project, Util.PropertyIceChecksum);
                chkConsole.Checked = Util.getProjectPropertyAsBool(_project, Util.PropertyConsoleOutput);
                
                IncludePathList list =
                    new IncludePathList(Util.getProjectProperty(_project, Util.PropertyIceIncludePath));
                foreach(String s in list)
                {
                    includeDirList.Items.Add(s.Trim());
                    if(Path.IsPathRooted(s.Trim()))
                    {
                        includeDirList.SetItemCheckState(includeDirList.Items.Count - 1, CheckState.Checked);
                    }
                }

                ComponentList selectedComponents = Util.getIceCSharpComponents(_project);
                foreach(String s in Util.getCSharpNames())
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
            }
            Cursor = Cursors.Default;  
        }

        private void checkComponent(String component, bool check)
        {
            if(_editingIncludes)
            {
                endEditIncludeDir(false);
            }
            switch (component)
            {
                case "Glacier2":
                {
                    chkGlacier2.Checked = check;
                    break;
                }
                case "Ice":
                {
                    chkIce.Checked = check;
                    break;
                }
                case "IceBox":
                {
                    chkIceBox.Checked = check;
                    break;
                }
                case "IceGrid":
                {
                    chkIceGrid.Checked = check;
                    break;
                }
                case "IcePatch2":
                {
                    chkIcePatch2.Checked = check;
                    break;
                }
                case "IceSSL":
                {
                    chkIceSSL.Checked = check;
                    break;
                }
                case "IceStorm":
                {
                    chkIceStorm.Checked = check;
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
            Cursor = Cursors.WaitCursor;
            if(_editingIncludes)
            {
                endEditIncludeDir(false);
            }
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
                setEnabled(chkEnableBuilder.Checked);
                chkEnableBuilder.Enabled = true;
                _initialized = true;
            }
            Cursor = Cursors.Default;
        }
        
        private void setEnabled(bool enabled)
        {
            txtIceHome.Enabled = enabled;
            btnSelectIceHome.Enabled = enabled;

            chkIcePrefix.Enabled = enabled;
            chkTie.Enabled = enabled;
            chkStreaming.Enabled = enabled;
            chkChecksum.Enabled = enabled;
            chkConsole.Enabled = enabled;
            includeDirList.Enabled = enabled;
            btnAddInclude.Enabled = enabled;
            btnEditInclude.Enabled = enabled;
            btnRemoveInclude.Enabled = enabled;
            btnMoveIncludeUp.Enabled = enabled;
            btnMoveIncludeDown.Enabled = enabled;

            txtExtraOptions.Enabled = enabled;

            chkGlacier2.Enabled = enabled;
            chkIce.Enabled = enabled;
            chkIceBox.Enabled = enabled;
            chkIceGrid.Enabled = enabled;
            chkIcePatch2.Enabled = enabled;
            chkIceSSL.Enabled = enabled;
            chkIceStorm.Enabled = enabled;
        }

        private void formClosing(object sender, EventArgs e)
        {
            Cursor = Cursors.WaitCursor;
            if(_editingIncludes)
            {
                endEditIncludeDir(false);
            }
            if(!_changed)
            {
                if(txtExtraOptions.Modified)
                {
                    _changed = true;
                }
                else if(txtIceHome.Modified)
                {
                    _changed = true;
                }
            }

            if(_changed && Util.isSliceBuilderEnabled(_project))
            {
                Builder builder = Connect.getBuilder();
                builder.cleanProject(_project);
                builder.buildProject(_project, true, vsBuildScope.vsBuildScopeProject);
            }
            Cursor = Cursors.Default;
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void btnSelectIceHome_Click(object sender, EventArgs e)
        {
            if(_editingIncludes)
            {
                endEditIncludeDir(false);
            }
            FolderBrowserDialog dialog = new FolderBrowserDialog();
            dialog.SelectedPath = Util.getAbsoluteIceHome(_project);
            dialog.Description = "Select Ice Home Installation Directory";
            DialogResult result = dialog.ShowDialog();
            if(result == DialogResult.OK)
            {
                Util.updateIceHome(_project, dialog.SelectedPath, false);
                load();
                _changed = true;
            }
        }

        private void txtIceHome_KeyPress(object sender, KeyPressEventArgs e)
        {
            if(e.KeyChar == (char)Keys.Return)
            {
                updateIceHome();
                e.Handled = true;
            }
        }

        private void txtIceHome_Focus(object sender, EventArgs e)
        {
            if(_editingIncludes)
            {
                endEditIncludeDir(false);
            }
        }

        private void txtIceHome_LostFocus(object sender, EventArgs e)
        {
            updateIceHome();
        }

        private void updateIceHome()
        {
            if(!_iceHomeUpdating)
            {
                _iceHomeUpdating = true;
                if(!txtIceHome.Text.Equals(Util.getProjectProperty(_project, Util.PropertyIceHome),
                                           StringComparison.CurrentCultureIgnoreCase))
                {
                    Util.updateIceHome(_project, txtIceHome.Text, false);
                    load();
                    _changed = true;
                    txtIceHome.Modified = false;
                }
                _iceHomeUpdating = false;
            }
        }

        private void chkIcePrefix_CheckedChanged(object sender, EventArgs e)
        {
            Cursor = Cursors.WaitCursor;
            if(_editingIncludes)
            {
                endEditIncludeDir(false);
            }
            Util.setProjectProperty(_project, Util.PropertyIcePrefix, chkIcePrefix.Checked.ToString());
            _changed = true;
            Cursor = Cursors.Default;
        }

        private void chkTie_CheckedChanged(object sender, EventArgs e)
        {
            Cursor = Cursors.WaitCursor;
            if(_editingIncludes)
            {
                endEditIncludeDir(false);
            }
            Util.setProjectProperty(_project, Util.PropertyIceTie, chkTie.Checked.ToString());
            _changed = true;
            Cursor = Cursors.Default;
        }

        private void chkStreaming_CheckedChanged(object sender, EventArgs e)
        {
            Cursor = Cursors.WaitCursor;
            if(_editingIncludes)
            {
                endEditIncludeDir(false);
            }
            Util.setProjectProperty(_project, Util.PropertyIceStreaming, chkStreaming.Checked.ToString());
            _changed = true;
            Cursor = Cursors.Default;
        }
        
        private void chkChecksum_CheckedChanged(object sender, EventArgs e)
        {
            Cursor = Cursors.WaitCursor;
            if(_editingIncludes)
            {
                endEditIncludeDir(false);
            }
            Util.setProjectProperty(_project, Util.PropertyIceChecksum, chkChecksum.Checked.ToString());
            _changed = true;
            Cursor = Cursors.Default;
        }
        
        private void saveSliceIncludes()
        {
            Cursor = Cursors.WaitCursor;
            IncludePathList paths = new IncludePathList();
            foreach(String s in includeDirList.Items)
            {
                paths.Add(s.Trim());
            }
            Util.setProjectProperty(_project, Util.PropertyIceIncludePath, paths.ToString());
            _changed = true;
            Cursor = Cursors.Default;
        }

        private void btnAddInclude_Click(object sender, EventArgs e)
        {
            if(_editingIncludes)
            {
                endEditIncludeDir(false);
            }
            includeDirList.Items.Add("");
            includeDirList.SelectedIndex = includeDirList.Items.Count - 1;
            _editingIndex = includeDirList.SelectedIndex;
            beginEditIncludeDir();
        }

        private void btnRemoveInclude_Click(object sender, EventArgs e)
        {
            Cursor = Cursors.WaitCursor;
            int index = includeDirList.SelectedIndex;
            if(_editingIncludes)
            {
                index = _editingIndex;
                endEditIncludeDir(false);
            }
            
            if(index > -1 && index < includeDirList.Items.Count)
            {
                int selected = index;
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
            }
            Cursor = Cursors.Default;
        }

        private void btnMoveIncludeUp_Click(object sender, EventArgs e)
        {
            Cursor = Cursors.WaitCursor;
            if(_editingIncludes)
            {
                endEditIncludeDir(false);
            }
            int index = includeDirList.SelectedIndex;
            if(index > 0)
            {
                string current = includeDirList.SelectedItem.ToString();
                includeDirList.Items.RemoveAt(index);
                includeDirList.Items.Insert(index - 1, current);
                includeDirList.SelectedIndex = index - 1;
                saveSliceIncludes();
            }
            resetIncludeDirChecks();
            Cursor = Cursors.Default;
        }

        private void btnMoveIncludeDown_Click(object sender, EventArgs e)
        {
            Cursor = Cursors.WaitCursor;
            if(_editingIncludes)
            {
                endEditIncludeDir(false);
            }
            int index = includeDirList.SelectedIndex;
            if(index < includeDirList.Items.Count - 1 && index > -1)
            {
                string current = includeDirList.SelectedItem.ToString();
                includeDirList.Items.RemoveAt(index);
                includeDirList.Items.Insert(index + 1, current);
                includeDirList.SelectedIndex = index + 1;
                saveSliceIncludes();
                resetIncludeDirChecks();
            }
            Cursor = Cursors.Default;
        }

        private void resetIncludeDirChecks()
        {
            _initialized = false;
            for(int i = 0; i < includeDirList.Items.Count; i++)
            {
                String path = includeDirList.Items[i].ToString();
                if(String.IsNullOrEmpty(path))
                {
                    continue;
                }

                if(Path.IsPathRooted(path))
                {
                    includeDirList.SetItemCheckState(i, CheckState.Checked);
                }
                else
                {
                    includeDirList.SetItemCheckState(i, CheckState.Unchecked);
                }
            }
            _initialized = true;
        }

        private void includeDirList_ItemCheck(object sender, ItemCheckEventArgs e)
        {
            if(_editingIncludes)
            {
                return;
            }
            string path = includeDirList.Items[e.Index].ToString();
            if(!Util.containsEnvironmentVars(path))
            {
                if (e.NewValue == CheckState.Unchecked)
                {
                    path = Util.relativePath(Path.GetDirectoryName(_project.FileName), path);
                }
                else if(e.NewValue == CheckState.Checked)
                {
                    if (!Path.IsPathRooted(path))
                    {
                        path = Path.GetFullPath(Path.Combine(Path.GetDirectoryName(_project.FileName), path));
                    }
                }
            }
            includeDirList.Items[e.Index] = path;
            if(_initialized)
            {
                saveSliceIncludes();
            }
        }

        private void txtExtraOptions_Focus(object sender, EventArgs e)
        {
            if(_editingIncludes)
            {
                endEditIncludeDir(false);
            }
        }

        private void txtExtraOptions_LostFocus(object sender, EventArgs e)
        {
            if(txtExtraOptions.Modified)
            {
                Util.setProjectProperty(_project, Util.PropertyIceExtraOptions, txtExtraOptions.Text);
                _changed = true;
            }
        }
        
        private void componentChanged(string name, bool value)
        {
            Cursor = Cursors.WaitCursor;
            if(_editingIncludes)
            {
                endEditIncludeDir(false);
            }
            if(_initialized)
            {
                if(value)
                {
                    Util.addCSharpReference(_project, name);
                }
                else
                {
                    Util.removeCSharpReference(_project, name);
                }
                _changed = true;
            }
            Cursor = Cursors.Default;        
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

        private void chkConsole_CheckedChanged(object sender, EventArgs e)
        {
            Cursor = Cursors.WaitCursor;
            if(_editingIncludes)
            {
                endEditIncludeDir(false);
            }
            Util.setProjectProperty(_project, Util.PropertyConsoleOutput, chkConsole.Checked.ToString());
            Cursor = Cursors.Default;
        }

        private void btnEdit_Click(object sender, EventArgs e)
        {
            if(includeDirList.SelectedIndex != -1)
            {
                _editingIndex = includeDirList.SelectedIndex;
                beginEditIncludeDir();
            }
        }

        private void includeDirList_SelectedIndexChanged(object sender, EventArgs e)
        {
            if(_editingIncludes)
            {
                endEditIncludeDir(false);
            }
        }

        private void beginEditIncludeDir()
        {
            if(_editingIncludes)
            {
                endEditIncludeDir(false);
            }
            _editingIncludes = true;
            CancelButton = null;
            if(_editingIndex != -1)
            {
                _txtIncludeDir = new TextBox();
                _txtIncludeDir.Text = includeDirList.Items[includeDirList.SelectedIndex].ToString();

                includeDirList.SelectionMode = SelectionMode.One;

                Rectangle rect = includeDirList.GetItemRectangle(includeDirList.SelectedIndex);
                _txtIncludeDir.Location = new Point(includeDirList.Location.X + 2,
                                                    includeDirList.Location.Y + rect.Y);
                _txtIncludeDir.Width = includeDirList.Width - 50;
                _txtIncludeDir.Parent = includeDirList;
                _txtIncludeDir.KeyDown += new KeyEventHandler(includeDirKeyDown);
                groupBox1.Controls.Add(_txtIncludeDir);

                _btnSelectInclude = new Button();
                _btnSelectInclude.Text = "...";
                _btnSelectInclude.Location = new Point(includeDirList.Location.X + _txtIncludeDir.Width,
                                                       includeDirList.Location.Y + rect.Y);
                _btnSelectInclude.Width = 49;
                _btnSelectInclude.Height = _txtIncludeDir.Height;
                _btnSelectInclude.Click += new EventHandler(selectIncludeClicked);
                groupBox1.Controls.Add(_btnSelectInclude);


                _txtIncludeDir.Show();
                _txtIncludeDir.BringToFront();
                _txtIncludeDir.Focus();

                _btnSelectInclude.Show();
                _btnSelectInclude.BringToFront();
            }
        }

        private void endEditIncludeDir(bool saveChanges)
        {
            _initialized = false;
            if(!_editingIncludes)
            {
                _initialized = true;
                return;
            }
            _editingIncludes = false;
            String path = null;
            if(_editingIndex > -1 && _editingIndex < includeDirList.Items.Count)
            {
                path = includeDirList.Items[_editingIndex].ToString();
            }

            lock(this)
            {
                CancelButton = btnClose;
                if(_txtIncludeDir == null || _btnSelectInclude == null)
                {
                    _initialized = true;
                    return;
                }
                if(saveChanges)
                {
                    path = _txtIncludeDir.Text;
                    if(path != null)
                    {
                        path = path.Trim();
                    }
                }

                this.groupBox1.Controls.Remove(_txtIncludeDir);
                _txtIncludeDir = null;

                this.groupBox1.Controls.Remove(_btnSelectInclude);
                _btnSelectInclude = null;
            }

            if(String.IsNullOrEmpty(path))
            {
                if(_editingIndex != -1)
                {
                    includeDirList.Items.RemoveAt(_editingIndex);
                    includeDirList.SelectedIndex = includeDirList.Items.Count - 1;
                    _editingIndex = -1;
                    saveSliceIncludes();
                }
            }
            else if(_editingIndex != -1 && saveChanges)
            {
                if(!path.Equals(includeDirList.Items[_editingIndex].ToString(),
                                               StringComparison.CurrentCultureIgnoreCase))
                {
                    includeDirList.Items[_editingIndex] = path;
                    if(Path.IsPathRooted(path))
                    {
                        includeDirList.SetItemCheckState(_editingIndex, CheckState.Checked);
                    }
                    else
                    {
                        includeDirList.SetItemCheckState(_editingIndex, CheckState.Unchecked);
                    }
                    saveSliceIncludes();
                }
            }
            resetIncludeDirChecks();
        }

        private void includeDirKeyDown(object sender, KeyEventArgs e)
        {
            if(e.KeyCode.Equals(Keys.Escape))
            {
                endEditIncludeDir(false);
            }
            if(e.KeyCode.Equals(Keys.Enter))
            {
                endEditIncludeDir(true);
            }
        }

        private void selectIncludeClicked(object sender, EventArgs e)
        {
            FolderBrowserDialog dialog = new FolderBrowserDialog();
            string projectDir = Path.GetFullPath(Path.GetDirectoryName(_project.FileName));
            dialog.SelectedPath = projectDir;
            dialog.Description = "Slice Include Directory";
            DialogResult result = dialog.ShowDialog();
            if(result == DialogResult.OK)
            {
                string path = dialog.SelectedPath;
                if(!Util.containsEnvironmentVars(path))
                {
                    path = Util.relativePath(projectDir, Path.GetFullPath(path));
                }
                _txtIncludeDir.Text = path;
            }
            endEditIncludeDir(true);
        }

        private int _editingIndex = -1;
        private bool _editingIncludes;
        private bool _changed = false;
        private bool _initialized = false;
        private Project _project;
        private bool _iceHomeUpdating = false;
        private TextBox _txtIncludeDir = null;
        private Button _btnSelectInclude = null;
    }
}
