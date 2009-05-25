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
using System.Text;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Windows.Forms;
using EnvDTE;

namespace Ice.VisualStudio
{
    public partial class IncludeDirForm : Form
    {
        public IncludeDirForm(IncludePathList list, Project project)
        {
            InitializeComponent();
            this.CancelButton = okBtn;
            this.AcceptButton = addBtn;
            _project = project;
            foreach(String s in list)
            {
                includeDirList.Items.Add(s.Trim());
            }
        }

        private void addBtn_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog dialog = new FolderBrowserDialog();
            string projectDir = Path.GetDirectoryName(_project.FileName);
            dialog.SelectedPath = projectDir;
            dialog.Description = "Slice Include Directory";
            DialogResult result = dialog.ShowDialog();
            if(result == DialogResult.OK)
            {
                if(!dialog.SelectedPath.Contains(projectDir))
                {
                    includeDirList.Items.Add(dialog.SelectedPath);
                }
                else
                {
                    includeDirList.Items.Add(Util.relativePath(projectDir, dialog.SelectedPath));
                }
            }
        }

        private void okBtn_Click(object sender, EventArgs e)
        {
            IncludePathList paths = new IncludePathList();
            foreach(String s in includeDirList.Items)
            {
                paths.Add(s.Trim());
            }
            Util.setProjectProperty(_project, Util.PropertyNames.IceIncludePath, paths.ToString());
            Close();
        }

        private void cancelBtn_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void removeBtn_Click(object sender, EventArgs e)
        {
            if(includeDirList.SelectedIndex != -1)
            {
                includeDirList.Items.RemoveAt(includeDirList.SelectedIndex);
            }
        }

        private void upBtn_Click(object sender, EventArgs e)
        {
            int index = includeDirList.SelectedIndex;
            if(index > 0)
            {
                string current = includeDirList.SelectedItem.ToString();
                includeDirList.Items.RemoveAt(index);
                includeDirList.Items.Insert(index - 1, current);
                includeDirList.SelectedIndex = index - 1;
            }
        }

        private void downBtn_Click(object sender, EventArgs e)
        {
            int index = includeDirList.SelectedIndex;
            if(index < includeDirList.Items.Count - 1)
            {
                string current = includeDirList.SelectedItem.ToString();
                includeDirList.Items.RemoveAt(index);
                includeDirList.Items.Insert(index + 1, current);
                includeDirList.SelectedIndex = index + 1;
            }
        }

        private Project _project;
    }
}
