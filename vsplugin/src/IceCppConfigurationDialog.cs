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
    public partial class IceCppConfigurationDialog : Form
    {
        public IceCppConfigurationDialog(Project project)
        {
            InitializeComponent();
            _project = project;
            
            //
            // Set the toolTip messages.
            //
            toolTip.SetToolTip(txtIceHome, "Ice installation directory.");
            toolTip.SetToolTip(btnSelectIceHome, "Ice installation directory.");
            toolTip.SetToolTip(chkStreaming, "Generate marshalling support for stream API (--stream).");
            toolTip.SetToolTip(chkChecksum, "Generate checksums for Slice definitions (--checksum).");
            toolTip.SetToolTip(chkIcePrefix, "Permit Ice prefixes (--ice).");
            toolTip.SetToolTip(chkConsole, "Enable console output.");

            if(_project != null)
            {
                this.Text = "Ice Configuration - Project: " + _project.Name;
                bool enabled = Util.isSliceBuilderEnabled(project);
                setEnabled(enabled);
                chkEnableBuilder.Checked = enabled;
                load();
                _initialized = true;
                _changed = false;
            }
        }
        
        private void load()
        {
            System.Windows.Forms.Cursor c = Cursor.Current;
            Cursor = Cursors.WaitCursor;
            if(_project != null)
            {
                includeDirList.Items.Clear();
                txtIceHome.Text = Util.getIceHome(_project);
                txtIceHome.Modified = false;
                txtExtraOptions.Text = Util.getProjectProperty(_project, Util.PropertyNames.IceExtraOptions);

                chkIcePrefix.Checked = Util.getProjectPropertyAsBool(_project, Util.PropertyNames.IcePrefix);

                chkStreaming.Checked = Util.getProjectPropertyAsBool(_project, Util.PropertyNames.IceStreaming);
                chkChecksum.Checked = Util.getProjectPropertyAsBool(_project, Util.PropertyNames.IceChecksum);
                chkConsole.Checked = Util.getProjectPropertyAsBool(_project, Util.PropertyNames.ConsoleOutput);

                IncludePathList list =
                    new IncludePathList(Util.getProjectProperty(_project, Util.PropertyNames.IceIncludePath));
                foreach(String s in list)
                {
                    includeDirList.Items.Add(s.Trim());
                    if(Path.IsPathRooted(s.Trim()))
                    {
                        includeDirList.SetItemCheckState(includeDirList.Items.Count - 1, CheckState.Checked);
                    }
                }

                ComponentList selectedComponents = Util.getIceCppComponents(_project);
                foreach(String s in Util.ComponentNames.cppNames)
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
                txtDllExportSymbol.Text = Util.getProjectProperty(_project, Util.PropertyNames.IceDllExport);
            }
            Cursor = c;        
        }

        private void checkComponent(String component, bool check)
        {
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
            case "Freeze":
            {
                chkFreeze.Checked = check;
                break;
            }
            case "IceUtil":
            {
                chkIceUtil.Checked = check;
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
                setEnabled(chkEnableBuilder.Checked);
                chkEnableBuilder.Enabled = true;
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
            chkChecksum.Enabled = enabled;
            chkConsole.Enabled = enabled;
            includeDirList.Enabled = enabled;
            btnAddInclude.Enabled = enabled;
            btnRemoveInclude.Enabled = enabled;
            btnMoveIncludeUp.Enabled = enabled;
            btnMoveIncludeDown.Enabled = enabled;

            txtExtraOptions.Enabled = enabled;

            chkFreeze.Enabled = enabled;
            chkGlacier2.Enabled = enabled;
            chkIce.Enabled = enabled;
            chkIceBox.Enabled = enabled;
            chkIceGrid.Enabled = enabled;
            chkIcePatch2.Enabled = enabled;
            chkIceSSL.Enabled = enabled;
            chkIceStorm.Enabled = enabled;
            chkIceUtil.Enabled = enabled;
            txtDllExportSymbol.Enabled = enabled;
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            if(!_changed)
            {
                if(txtDllExportSymbol.Modified)
                {
                    _changed = true;
                }
                else if(txtExtraOptions.Modified)
                {
                    _changed = true;
                }
                else if(txtIceHome.Modified)
                {
                    _changed = true;
                }
            }

            if(_changed)
            {
                System.Windows.Forms.Cursor c = Cursor.Current;
                Cursor = Cursors.WaitCursor;
                Builder builder = Connect.getBuilder();
                builder.cleanProject(_project);
                builder.buildCppProject(_project, true);
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
            if(result == DialogResult.OK)
            {
                Util.updateIceHome(_project, dialog.SelectedPath);
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
        
        private void txtIceHome_LostFocus(object sender, EventArgs e)
        {
        
            updateIceHome();
        }
        
        private void updateIceHome()
        {
            if(!_iceHomeUpdating)
            {
                _iceHomeUpdating = true;
                if(!txtIceHome.Text.Equals(Util.getProjectProperty(_project, Util.PropertyNames.IceHome),
                                           StringComparison.CurrentCultureIgnoreCase))
                {
                    String path = txtIceHome.Text;
                    if(!Path.IsPathRooted(path))
                    {
                        path = Path.Combine(Path.GetDirectoryName(_project.FileName), path);
                        path = Path.GetFullPath(path);
                    }
                    Util.updateIceHome(_project, path);
                    load();
                    _changed = true;
                    txtIceHome.Modified = false;
                }
                _iceHomeUpdating = false;
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
        
        private void chkChecksum_CheckedChanged(object sender, EventArgs e)
        {
            System.Windows.Forms.Cursor c = Cursor.Current;
            Cursor = Cursors.WaitCursor;
            Util.setProjectProperty(_project, Util.PropertyNames.IceChecksum, chkChecksum.Checked.ToString());
            _changed = true;
            Cursor = c;
        }

        private void saveSliceIncludes()
        {
            IncludePathList paths = new IncludePathList();
            foreach(String s in includeDirList.Items)
            {
                paths.Add(s.Trim());
            }
            Util.setProjectProperty(_project, Util.PropertyNames.IceIncludePath, paths.ToString());
            _changed = true;
        }

        private void btnAddInclude_Click(object sender, EventArgs e)
        {
            endEditIncludeDir(false);
            includeDirList.Items.Add("");
            includeDirList.SelectedIndex = includeDirList.Items.Count - 1;
            beginEditIncludeDir();
        }

        private void btnRemoveInclude_Click(object sender, EventArgs e)
        {
            endEditIncludeDir(false);
            if(includeDirList.SelectedIndex != -1)
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
            endEditIncludeDir(false);
            int index = includeDirList.SelectedIndex;
            if(index > 0)
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
            endEditIncludeDir(false);
            int index = includeDirList.SelectedIndex;
            if(index < includeDirList.Items.Count - 1)
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

        private void includeDirList_ItemCheck(object sender, ItemCheckEventArgs e)
        {
            string path = null;
            if(e.NewValue == CheckState.Unchecked)
            {
               path = Util.relativePath(Path.GetDirectoryName(_project.FileName), 
                                        includeDirList.Items[e.Index].ToString());
            }
            else if(e.NewValue == CheckState.Checked)
            {
               path = includeDirList.Items[e.Index].ToString();
               if(!Path.IsPathRooted(path))
               {
                   path = Path.GetFullPath(Path.Combine(Path.GetDirectoryName(_project.FileName), path));
               }
            }
            includeDirList.Items[e.Index] = path;
            saveSliceIncludes();
            _changed = true;
        }

        private void txtExtraOptions_LostFocus(object sender, EventArgs e)
        {
            if(txtExtraOptions.Modified)
            {
                Util.setProjectProperty(_project, Util.PropertyNames.IceExtraOptions, txtExtraOptions.Text);
                _changed = true;
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
            componentChanged("IceUtil", chkFreeze.Checked);
        }

        private void chkFreeze_CheckedChanged(object sender, EventArgs e)
        {
            componentChanged("Freeze", chkFreeze.Checked);
        }

        private void componentChanged(String name, bool isChecked)
        {
            System.Windows.Forms.Cursor c = Cursor.Current;
            Cursor = Cursors.WaitCursor;
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
        
        private void beginEditIncludeDir()
        {
            if(_txtIncludeDir != null)
            {
                this.Controls.Remove(_txtIncludeDir);
                _txtIncludeDir = null;
            }

            if(_btnSelectInclude != null)
            {
                this.Controls.Remove(_btnSelectInclude);
                _btnSelectInclude = null;
            }

            if(includeDirList.SelectedIndex != -1)
            {
                int index = includeDirList.SelectedIndex;
                _txtIncludeDir = new TextBox();
                _txtIncludeDir.Text = includeDirList.Items[includeDirList.SelectedIndex].ToString();
                Point p = new Point(includeDirList.Left, includeDirList.Top);
                includeDirList.SelectionMode = SelectionMode.One;
                Rectangle rect = includeDirList.GetItemRectangle(includeDirList.SelectedIndex);
                p = includeDirList.PointToScreen(new Point(rect.X, rect.Y));
                _txtIncludeDir.SetBounds(p.X - includeDirList.Width + 10,
                              p.Y - includeDirList.Height + 5,
                              includeDirList.Width - 50,
                              4);


                _btnSelectInclude = new Button();
                _btnSelectInclude.Text = "...";
                _btnSelectInclude.SetBounds(p.X - includeDirList.Width + _txtIncludeDir.Width + 5,
                                 p.Y - includeDirList.Height + 4,
                                 50,
                                 22);

                _btnSelectInclude.Click += new EventHandler(selectIncludeClicked);
                this.Controls.Add(_txtIncludeDir);
                this.Controls.Add(_btnSelectInclude);
                _txtIncludeDir.Show();
                _txtIncludeDir.BringToFront();
                _txtIncludeDir.Focus();

                _txtIncludeDir.KeyDown += new KeyEventHandler(includeDirKeyDown);

                _btnSelectInclude.Show();
                _btnSelectInclude.BringToFront();
            }
        }
        
        private void endEditIncludeDir(bool saveChanges)
        {
            if(includeDirList.SelectedIndex != -1 && saveChanges)
            {
                if(!_txtIncludeDir.Text.Equals(includeDirList.Items[includeDirList.SelectedIndex].ToString(),
                                               StringComparison.CurrentCultureIgnoreCase))
                {
                    includeDirList.Items[includeDirList.SelectedIndex] = _txtIncludeDir.Text;
                    if(Path.IsPathRooted(_txtIncludeDir.Text))
                    {
                        includeDirList.SetItemCheckState(includeDirList.SelectedIndex, CheckState.Checked);
                    }
                    else
                    {
                        includeDirList.SetItemCheckState(includeDirList.SelectedIndex, CheckState.Unchecked);
                    }
                    saveSliceIncludes();
                }
            }
            if(_txtIncludeDir != null)
            {
                this.Controls.Remove(_txtIncludeDir);
                _txtIncludeDir = null;
            }

            if(_btnSelectInclude != null)
            {
                this.Controls.Remove(_btnSelectInclude);
                _btnSelectInclude = null;
            }
        }

        private void includeDirKeyDown(object sender, KeyEventArgs e)
        {
            if(!e.KeyCode.Equals(Keys.Enter))
            {
                return;
            }
            endEditIncludeDir(true);
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
                string path = Util.relativePath(projectDir, Path.GetFullPath(dialog.SelectedPath));
                _txtIncludeDir.Text = path;
            }
            endEditIncludeDir(true);
        }
        
        private void txtDllExportSymbol_LostFocus(object sender, EventArgs e)
        {
            if(txtDllExportSymbol.Modified)
            {
                Util.setProjectProperty(_project, Util.PropertyNames.IceDllExport, txtDllExportSymbol.Text);
                _changed = true;
            }
        }
        
        private bool _initialized = false;
        private bool _changed = false;
        private Project _project;
        private bool _iceHomeUpdating = false;
        private TextBox _txtIncludeDir = null;
        private Button _btnSelectInclude = null;

        private void btnEdit_Click(object sender, EventArgs e)
        {
            beginEditIncludeDir();
        }
    }
}
