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
using System.Windows.Forms;

using System.IO;

using EnvDTE;

namespace Ice.VisualStudio
{
    public partial class IncludePathView : UserControl
    {
        public IncludePathView()
        {
            InitializeComponent();
        }

        public void load()
        {
            sliceIncludeList.Items.Clear();
            IncludePathList list =
                            new IncludePathList(Util.getProjectProperty(_project, Util.PropertyIceIncludePath));
            foreach(String s in list)
            {
                sliceIncludeList.Items.Add(s.Trim());
                if(Path.IsPathRooted(s.Trim()))
                {
                    sliceIncludeList.SetItemCheckState(sliceIncludeList.Items.Count - 1, CheckState.Checked);
                }
            }
        }

        public void setEnabled(bool enabled)
        {
            sliceIncludeList.Enabled = enabled;
            btnAdd.Enabled = enabled;
            btnEdit.Enabled = enabled;
            btnRemove.Enabled = enabled;
            btnUp.Enabled = enabled;
            btnDown.Enabled = enabled;
        }

        public bool hasUnsavedChanges()
        {
            if(_editingIncludes && 
                !_txtIncludeDir.Text.Trim().TrimEnd(Path.DirectorySeparatorChar).Equals(_editingIncludeDir))
            {
                return true;    
            }

            if(!sliceIncludes().Equal(new IncludePathList(
               Util.getProjectProperty(_project, Util.PropertyIceIncludePath))))
            {
                return true;
            }
            return false;
        }

        public bool apply()
        {
            bool changed = false;
            IncludePathList includes = sliceIncludes();
            if(!includes.Equal(new IncludePathList(
                               Util.getProjectProperty(_project, Util.PropertyIceIncludePath))))
            {
                String s = includes.ToString();
                Util.setProjectProperty(_project, Util.PropertyIceIncludePath, s);
                changed = true;
            }
            return changed;
        }

        public void setIceConfigurationDialog(IceConfigurationDialog dialog)
        {
            _dialog = dialog;
        }

        public bool editingIncludeDir()
        {
            return _editingIncludes;
        }

        private void sliceIncludeList_SelectedIndexChanged(object sender, EventArgs e)
        {
            if(_editingIncludes)
            {
                endEditIncludeDir(true);
            }
        }

        private void beginEditIncludeDir()
        {
            if(_editingIncludes)
            {
                endEditIncludeDir(true);
            }
            _editingIncludes = true;
            _dialog.unsetCancelButton();
            if(_editingIndex != -1)
            {
                _txtIncludeDir = new TextBox();
                _txtIncludeDir.Text = sliceIncludeList.Items[sliceIncludeList.SelectedIndex].ToString();
                _editingIncludeDir = _txtIncludeDir.Text;
                sliceIncludeList.SelectionMode = SelectionMode.One;

                Rectangle rect = sliceIncludeList.GetItemRectangle(sliceIncludeList.SelectedIndex);
                _txtIncludeDir.Location = new Point(sliceIncludeList.Location.X + 2,
                                                    sliceIncludeList.Location.Y + rect.Y);
                _txtIncludeDir.Width = sliceIncludeList.Width - 50;
                _txtIncludeDir.Parent = sliceIncludeList;
                _txtIncludeDir.KeyDown += new KeyEventHandler(includeDirKeyDown);
                _txtIncludeDir.KeyUp += new KeyEventHandler(includeDirKeyUp);
                groupBox1.Controls.Add(_txtIncludeDir);

                _btnSelectInclude = new Button();
                _btnSelectInclude.Text = "...";
                _btnSelectInclude.Location = new Point(sliceIncludeList.Location.X + _txtIncludeDir.Width,
                                                       sliceIncludeList.Location.Y + rect.Y);
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

        public void endEditIncludeDir(bool saveChanges)
        {
            if(!_editingIncludes)
            {
                _dialog.needSave();
                return;
            }
            _editingIncludes = false;
            String path = null;
            if(_editingIndex > -1 && _editingIndex < sliceIncludeList.Items.Count)
            {
                path = sliceIncludeList.Items[_editingIndex].ToString();
            }

            lock(this)
            {
                _dialog.setCancelButton();
                if(_txtIncludeDir == null || _btnSelectInclude == null)
                {
                    _dialog.needSave();
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
                    sliceIncludeList.Items.RemoveAt(_editingIndex);
                    sliceIncludeList.SelectedIndex = sliceIncludeList.Items.Count - 1;
                    _editingIndex = -1;
                }
            }
            else if(_editingIndex != -1 && saveChanges)
            {
                if(!path.Equals(sliceIncludeList.Items[_editingIndex].ToString(),
                                               StringComparison.CurrentCultureIgnoreCase))
                {
                    IncludePathList includes = sliceIncludes();
                    if(includes.Count > _editingIndex)
                    {
                        //
                        // We don't want an item to be considered a duplicate of itself.
                        //
                        includes.RemoveAt(_editingIndex);
                    }
                    if(includes.Contains(_project, path))
                    {
                        MessageBox.Show(this, "The Slice Include Path doesn't allow duplicates.\n" +
                                   "Value: `" + path + "' will be removed.\n",
                                   "Ice Visual Studio Add-in", MessageBoxButtons.OK,
                                   MessageBoxIcon.Warning, MessageBoxDefaultButton.Button1,
                                   (MessageBoxOptions)0);
                        sliceIncludeList.Items.RemoveAt(_editingIndex);
                        sliceIncludeList.SelectedIndex = sliceIncludeList.Items.Count - 1;
                        _editingIndex = -1;
                    }
                    else
                    {
                        sliceIncludeList.Items[_editingIndex] = path;
                        if(Path.IsPathRooted(path))
                        {
                            sliceIncludeList.SetItemCheckState(_editingIndex, CheckState.Checked);
                        }
                        else
                        {
                            sliceIncludeList.SetItemCheckState(_editingIndex, CheckState.Unchecked);
                        }
                    }
                }
            }
            resetIncludeDirChecks();
            _dialog.needSave();
        }

        private void includeDirKeyDown(object sender, KeyEventArgs e)
        {
            if(e.KeyCode.Equals(Keys.Escape))
            {
                endEditIncludeDir(false);
            }
            else if(e.KeyCode.Equals(Keys.Enter))
            {
                endEditIncludeDir(true);
            }
        }

        private void includeDirKeyUp(object sender, KeyEventArgs e)
        {
            if(!e.KeyCode.Equals(Keys.Enter) && !e.KeyCode.Equals(Keys.Escape) &&
               _editingIncludes)
            {
                _dialog.needSave();
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
                    path = Util.relativePath(_project, Path.GetFullPath(path));
                }
                _txtIncludeDir.Text = path;
            }
            endEditIncludeDir(true);
        }

        private void btnAdd_Click(object sender, EventArgs e)
        {
            if(_editingIncludes)
            {
                endEditIncludeDir(true);
            }
            sliceIncludeList.Items.Add("");
            sliceIncludeList.SelectedIndex = sliceIncludeList.Items.Count - 1;
            _editingIndex = sliceIncludeList.SelectedIndex;
            beginEditIncludeDir();
        }

        private void btnEdit_Click(object sender, EventArgs e)
        {
            if(sliceIncludeList.SelectedIndex != -1)
            {
                _editingIndex = sliceIncludeList.SelectedIndex;
                beginEditIncludeDir();
            }
        }

        private void btnRemove_Click(object sender, EventArgs e)
        {
            Cursor = Cursors.WaitCursor;
            int index = sliceIncludeList.SelectedIndex;
            if(_editingIncludes)
            {
                index = _editingIndex;
                endEditIncludeDir(true);
            }
            if(index > -1 && index < sliceIncludeList.Items.Count)
            {
                int selected = index;
                sliceIncludeList.Items.RemoveAt(selected);
                if(sliceIncludeList.Items.Count > 0)
                {
                    if(selected > 0)
                    {
                        selected -= 1;
                    }
                    sliceIncludeList.SelectedIndex = selected;
                }
            }
            _dialog.needSave();
            Cursor = Cursors.Default;
        }

        private IncludePathList sliceIncludes()
        {
            IncludePathList paths = new IncludePathList();
            foreach(String s in sliceIncludeList.Items)
            {
                paths.Add(s.Trim());
            }
            return paths;
        }

        private void btnUp_Click(object sender, EventArgs e)
        {
            Cursor = Cursors.WaitCursor;
            if(_editingIncludes)
            {
                endEditIncludeDir(true);
            }
            int index = sliceIncludeList.SelectedIndex;
            if(index > 0)
            {
                string current = sliceIncludeList.SelectedItem.ToString();
                sliceIncludeList.Items.RemoveAt(index);
                sliceIncludeList.Items.Insert(index - 1, current);
                sliceIncludeList.SelectedIndex = index - 1;
                resetIncludeDirChecks();
                _dialog.needSave();
            }
            Cursor = Cursors.Default;
        }

        private void btnDown_Click(object sender, EventArgs e)
        {
            Cursor = Cursors.WaitCursor;
            if(_editingIncludes)
            {
                endEditIncludeDir(true);
            }
            int index = sliceIncludeList.SelectedIndex;
            if(index < sliceIncludeList.Items.Count - 1 && index > -1)
            {
                string current = sliceIncludeList.SelectedItem.ToString();
                sliceIncludeList.Items.RemoveAt(index);
                sliceIncludeList.Items.Insert(index + 1, current);
                sliceIncludeList.SelectedIndex = index + 1;
                resetIncludeDirChecks();
                _dialog.needSave();
            }
            Cursor = Cursors.Default;
        }

        //
        // Reset the include dir checkboxes that indicate if the path
        // is absolute or relative. This should be called after the list
        // is populated or the includes list order is modified.
        //
        private void resetIncludeDirChecks()
        {
            for(int i = 0; i < sliceIncludeList.Items.Count; ++i)
            {
                String path = sliceIncludeList.Items[i].ToString();
                if(String.IsNullOrEmpty(path))
                {
                    continue;
                }

                if(Path.IsPathRooted(path))
                {
                    sliceIncludeList.SetItemCheckState(i, CheckState.Checked);
                }
                else
                {
                    sliceIncludeList.SetItemCheckState(i, CheckState.Unchecked);
                }
            }
        }

        private void sliceIncludeList_ItemCheck(object sender, ItemCheckEventArgs e)
        {
            if(_editingIncludes)
            {
                return;
            }
            string path = sliceIncludeList.Items[e.Index].ToString();
            if(Util.containsEnvironmentVars(path))
            {
                return;
            }

            if(e.NewValue == CheckState.Unchecked)
            {
                path = Util.relativePath(_project, path);
            }
            else if(e.NewValue == CheckState.Checked)
            {
                if(!Path.IsPathRooted(path))
                {
                    path = Util.absolutePath(_project, path);
                }
            }

            sliceIncludeList.Items[e.Index] = path;

            _dialog.needSave();
        }

        public void init(IceConfigurationDialog dialog, Project project)
        {
            _dialog = dialog;
            _project = project;
        }

        private Project _project;
        private IceConfigurationDialog _dialog;        
        private int _editingIndex = -1;
        private bool _editingIncludes;
        private string _editingIncludeDir = null;
        private TextBox _txtIncludeDir;
        private Button _btnSelectInclude;
    }
}
