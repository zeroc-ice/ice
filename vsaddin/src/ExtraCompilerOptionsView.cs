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
            _configurationOptions = new Dictionary<string, string>();

            comboBoxConfigurationName.Items.Add("All");
            _configurationOptions["All"] = Util.getProjectProperty(_project, Util.PropertyIceExtraOptions);

            object[] configurations = (object[])_project.ConfigurationManager.ConfigurationRowNames;
            foreach(object name in configurations)
            {
                comboBoxConfigurationName.Items.Add(name);
                _configurationOptions[name.ToString()] = Util.getProjectProperty(_project, Util.PropertyIceExtraOptions + "_" + name);
            }
            comboBoxConfigurationName.SelectedIndex = 0;
            txtExtraOptions.Text = Util.getProjectProperty(_project, Util.PropertyIceExtraOptions);
        }

        public bool hasUnsavedChanges()
        {
            bool unsaved = false;
            List<String> configurations = new List<String>();
            configurations.Add("All");
            foreach(object name in (object[])_project.ConfigurationManager.ConfigurationRowNames)
            {
                configurations.Add(name.ToString());
            }
            foreach(String name in configurations)
            {
                string property = Util.PropertyIceExtraOptions;
                if(!name.Equals("All"))
                {
                    property += "_" + name.ToString();
                }

                string value = "";
                if(!_configurationOptions.TryGetValue(name.ToString(), out value))
                {
                    unsaved = true;
                    break;
                }

                if(!value.Trim().Equals(Util.getProjectProperty(_project, property), 
                                        StringComparison.CurrentCulture))
                {
                    unsaved = true;
                    break;
                }
            }
            return unsaved;
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

                List<String> configurations = new List<String>();
                configurations.Add("All");
                foreach(object name in (object[])_project.ConfigurationManager.ConfigurationRowNames)
                {
                    configurations.Add(name.ToString());
                }
                foreach(String name in configurations)
                {
                    string property = Util.PropertyIceExtraOptions;
                    if(!name.Equals("All"))
                    {
                        property += "_" + name.ToString();
                    }

                    string value = "";
                    _configurationOptions.TryGetValue(name.ToString(), out value);
                    Util.setProjectProperty(_project, property, value.Trim());
                }

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
            _configurationOptions[comboBoxConfigurationName.SelectedItem.ToString()] = txtExtraOptions.Text.Trim();
            _dialog.needSave();
        }

        private void  txtExtraOptions_TextChanged(object sender, EventArgs e)
        {
            _configurationOptions[comboBoxConfigurationName.SelectedItem.ToString()] = txtExtraOptions.Text.Trim();
            _dialog.needSave();
        }

        private bool parseSlice2csOptions(String args)
        {
            Options opts = null;
            return Util.parseSlice2csOptions(args, true, ref opts);
        }

        private bool parseSlice2cppOptions(String configuration, String args)
        {
            Options opts = null;
            String headerExt = "";
            String sourceExt = "";

            bool success = Util.parseSlice2cppOptions(args, true, ref opts, ref headerExt, ref sourceExt);

            //
            // --header-ext and --source-ext can only be changed in the default configuration named "All". 
            //
            if(configuration.Equals("All"))
            {
                _headerExt = headerExt;
                _sourceExt = sourceExt;
            }
            else if(!headerExt.Equals("h") || !sourceExt.Equals("cpp"))
            {
                MessageBox.Show("Extra Options field contains some errors:\n" +
                                "--header-ext and --source-ext can only be used in the configuration named 'All'" +
                                " header-ext: " + headerExt + " source-ext: " + sourceExt,
                                "Ice Visual Studio Add-in", MessageBoxButtons.OK,
                                MessageBoxIcon.Error,
                                MessageBoxDefaultButton.Button1,
                                (MessageBoxOptions)0);
                success = false;
            }
            return success;
         }

        private bool checkExtraOptions()
        {
            bool isCppProject = Util.isCppProject(_project);
            bool isCsharpProject = isCppProject ? false : Util.isCSharpProject(_project);

            bool sucess = true;
            foreach(KeyValuePair<string, string> entry in _configurationOptions)
            {
                if(String.IsNullOrEmpty(entry.Value))
                {
                    continue;
                }
                if(isCppProject)
                {
                    if(!parseSlice2cppOptions(entry.Key, entry.Value))
                    {
                        sucess = false;
                    }
                }
                else if(isCsharpProject)
                {
                    if(!parseSlice2csOptions(entry.Value))
                    {
                        sucess = false;
                    }
                }
            }
            return sucess;
        }

        private void comboBoxConfigurationName_SelectedIndexChanged(object sender, EventArgs e)
        {
            String name = comboBoxConfigurationName.SelectedItem.ToString();
            String value = "";
            if(_configurationOptions.TryGetValue(name, out value))
            {
                txtExtraOptions.Text = _configurationOptions[name];
            }
            else
            {
                txtExtraOptions.Text = "";
            }
        }


        private Dictionary<String, String> _configurationOptions;

        private IceConfigurationDialog _dialog;
        private Project _project;
        private string _headerExt = "h";
        private string _sourceExt = "cpp";
    }
}
