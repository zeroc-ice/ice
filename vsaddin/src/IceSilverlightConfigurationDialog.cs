// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
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
using System.Globalization;
using EnvDTE;

namespace Ice.VisualStudio
{
    public partial class IceSilverlightConfigurationDialog : Form, IceConfigurationDialog
    {
        public IceSilverlightConfigurationDialog(Project project)
        {
            InitializeComponent();
            _project = project;

            outputDirView.init(this, _project);
            extraCompilerOptions.init(this, _project);
            includePathView.init(this, _project);

            //
            // Set the toolTip messages.
            //
            toolTip.SetToolTip(chkIcePrefix, "Allow Ice prefix (--ice).");

            if(_project != null)
            {
                this.Text = "Ice Configuration - Project: " + _project.Name;
                bool enabled = Util.isSliceBuilderEnabled(project);
                setEnabled(enabled);
                chkEnableBuilder.Checked = enabled;
                load();
                _initialized = true;
            }
            chkEnableBuilder.Focus();
        }

#region IceConfigurationDialog interface

        public bool editingIncludeDir()
        {
            return includePathView.editingIncludeDir();
        }

        public void endEditIncludeDir(bool saveChanges)
        {
            includePathView.endEditIncludeDir(saveChanges);
        }

        public void needSave()
        {
            if(_initialized)
            {
                btnApply.Enabled = hasUnsavedChanges();
            }
        }

        public void unsetCancelButton()
        {
            CancelButton = null;
        }

        public void setCancelButton()
        {
            CancelButton = btnCancel;
        }

#endregion IceConfigurationDialog interface
        private void load()
        {
            if(_project == null)
            {
                return;
            }
            Cursor = Cursors.WaitCursor;
            outputDirView.load();
            extraCompilerOptions.load();

            chkIcePrefix.Checked = Util.getProjectPropertyAsBool(_project, Util.PropertyIcePrefix);
            comboBoxVerboseLevel.SelectedIndex = Util.getVerboseLevel(_project);

            includePathView.load();

            btnApply.Enabled = false;
            Cursor = Cursors.Default;
        }

        private void chkEnableBuilder_CheckedChanged(object sender, EventArgs e)
        {
            try
            {
                Cursor = Cursors.WaitCursor;
                if(editingIncludeDir())
                {
                    endEditIncludeDir(true);
                }
                if(_initialized)
                {
                    _initialized = false;
                    setEnabled(chkEnableBuilder.Checked);

                    ComponentList components;
                    if(chkEnableBuilder.Checked)
                    {
                        //
                        // Enable the components that were previously enabled, if any.
                        //
                        components = new ComponentList(Util.getProjectProperty(_project, Util.PropertyIceComponents));

                        //
                        // If there isn't a previous set of components, we enable the default components.
                        //
                        if(components.Count == 0)
                        {
                            components.Add("IceSL");
                        }
                    }
                    else
                    {
                        components = new ComponentList("IceSL");
                    }


                    chkEnableBuilder.Enabled = true;
                    _initialized = true;
                    needSave();
                }
                Cursor = Cursors.Default;
            }
            catch(Exception ex)
            {
                Cursor = Cursors.Default;
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
        }
        
        private void setEnabled(bool enabled)
        {
            outputDirView.setEnabled(enabled);
            chkIcePrefix.Enabled = enabled;
            comboBoxVerboseLevel.Enabled = enabled;
            extraCompilerOptions.setEnabled(enabled);
            includePathView.setEnabled(enabled);
        }

        private void formClosing(object sender, FormClosingEventArgs e)
        {
            try
            {
                if(editingIncludeDir())
                {
                    endEditIncludeDir(false);
                }

                if(hasUnsavedChanges())
                {
                    if(!Util.warnUnsavedChanges(this))
                    {
                        e.Cancel = true;
                        return;
                    }
                }

                Cursor = Cursors.WaitCursor;
                if(_changed && Util.isSliceBuilderEnabled(_project))
                {
                    Util.rebuildProject(_project);
                }
                Cursor = Cursors.Default;
            }
            catch(Exception ex)
            {
                Cursor = Cursors.Default;
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            try
            {
                Close();
            }
            catch(Exception ex)
            {
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
        }

        private void component_Changed(object sender, EventArgs e)
        {
            try
            {
                Cursor = Cursors.WaitCursor;
                if(editingIncludeDir())
                {
                    endEditIncludeDir(true);
                }
                needSave();
                Cursor = Cursors.Default;
            }
            catch(Exception ex)
            {
                Cursor = Cursors.Default;
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
        }

        //
        // Apply unsaved changes, returns true if new settings are all applied correctly,
        // otherwise returns false.
        //
        private bool apply()
        {
            if(!hasUnsavedChanges())
            {
                return true; // Nothing to do.
            }
    
            try
            {
                if(editingIncludeDir())
                {
                    endEditIncludeDir(true);
                }

                //
                // This must be the first setting to be updated, as other settings cannot be
                // updated if the add-in is disabled.
                //
                if(chkEnableBuilder.Checked && !Util.isSliceBuilderEnabled(_project))
                {
                    Util.addBuilderToProject(_project, new ComponentList(Util.getSilverlightNames()));
                    _changed = true;
                }

                bool changed = false;
                if(!outputDirView.apply(ref changed))
                {
                    return false;
                }
                if(changed)
                {
                    _changed = true;
                }

                if(chkIcePrefix.Checked != Util.getProjectPropertyAsBool(_project, Util.PropertyIcePrefix))
                {
                    Util.setProjectProperty(_project, Util.PropertyIcePrefix, chkIcePrefix.Checked.ToString());
                    _changed = true;
                }

                if(comboBoxVerboseLevel.SelectedIndex != Util.getVerboseLevel(_project))
                {
                    Util.setProjectProperty(_project, Util.PropertyVerboseLevel,
                                            comboBoxVerboseLevel.SelectedIndex.ToString(CultureInfo.InvariantCulture));
                    _changed = true;
                }
                
                changed = false;
                if(!extraCompilerOptions.apply(ref changed))
                {
                    return false;
                }
                if(changed)
                {
                    _changed = true;
                }

                if(includePathView.apply())
                {
                    _changed = true;
                }

                //
                // This must be the last setting to be updated, as we want to update 
                // all other settings and that isn't possible if the builder is disabled.
                //
                if(!chkEnableBuilder.Checked && Util.isSliceBuilderEnabled(_project))
                {
                    Util.removeBuilderFromProject(_project, new ComponentList());
                    _initialized = false;
                    load();
                    _initialized = true;
                    _changed = true;
                }

                return true;
            }
            finally
            {
                needSave(); // Call needSave to update apply button status
            }
        }

        private bool hasUnsavedChanges()
        {
            if(chkEnableBuilder.Checked != Util.isSliceBuilderEnabled(_project))
            {
                return true;
            }

            //
            // If the builder is disabled, we are not interested in the other settings
            // to compute changes.
            //
            if(!Util.isSliceBuilderEnabled(_project))
            {
                return false;
            }

            if(outputDirView.hasUnsavedChanges())
            {
                return true;
            }

            if(chkIcePrefix.Checked != Util.getProjectPropertyAsBool(_project, Util.PropertyIcePrefix))
            {
                return true;
            }

            if(comboBoxVerboseLevel.SelectedIndex != Util.getVerboseLevel(_project))
            {
                return true;
            }

            if(extraCompilerOptions.hasUnsavedChanges())
            {
                return true;
            }

            if(includePathView.hasUnsavedChanges())
            {
                return true;
            }
            return false;
        }

        private void btnOk_Click(object sender, EventArgs e)
        {
            try
            {
                Cursor = Cursors.WaitCursor;
                if(apply())
                {
                    Close();
                }
                Cursor = Cursors.Default;
            }
            catch(Exception ex)
            {
                Cursor = Cursors.Default;
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
        }

        private void btnApply_Click(object sender, EventArgs e)
        {
            try
            {
                Cursor = Cursors.WaitCursor;
                apply();
                Cursor = Cursors.Default;
            }
            catch(Exception ex)
            {
                Cursor = Cursors.Default;
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
        }
        
        private bool _initialized;
        private bool _changed;
        private Project _project;
    }
}
