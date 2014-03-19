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
using System.Data;
using System.Drawing;
using System.IO;
using System.Text;
using System.Windows.Forms;
using System.Globalization;
using EnvDTE;

namespace Ice.VisualStudio
{
    public partial class IceCsharpConfigurationDialog : Form, IceConfigurationDialog
    {
        public IceCsharpConfigurationDialog(Project project)
        {
            InitializeComponent();
            _project = project;

            outputDirView.init(this, _project);
            extraCompilerOptions.init(this, _project);
            includePathView.init(this, _project);
            
            //
            // Set the toolTip messages.
            //
            toolTip.SetToolTip(chkStreaming, "Generate marshaling support for stream API (--stream).");
            toolTip.SetToolTip(chkChecksum, "Generate checksums for Slice definitions (--checksum).");
            toolTip.SetToolTip(chkIcePrefix, "Permit Ice prefixes (--ice).");
            toolTip.SetToolTip(chkTie, "Generate TIE classes (--tie).");

            if(_project != null)
            {
                this.Text = "Ice Configuration - Project: " + _project.Name;
                bool enabled = Util.isSliceBuilderEnabled(project);
                _compactFramework = Util.isCSharpSmartDeviceProject(_project);
                _silverlightFramework = Util.isSilverlightProject(_project);
                setEnabled(enabled);
                chkEnableBuilder.Checked = enabled;
                load();
                _initialized = true;
            }
            chkEnableBuilder.Focus();
        }
        
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
            chkTie.Checked = Util.getProjectPropertyAsBool(_project, Util.PropertyIceTie);
            chkStreaming.Checked = Util.getProjectPropertyAsBool(_project, Util.PropertyIceStreaming);
            chkChecksum.Checked = Util.getProjectPropertyAsBool(_project, Util.PropertyIceChecksum);
            comboBoxVerboseLevel.SelectedIndex = Util.getVerboseLevel(_project);

            includePathView.load();

            loadComponents();
            btnApply.Enabled = false;
            Cursor = Cursors.Default;  
        }

        private void loadComponents()
        {
            ComponentList selectedComponents = Util.getIceDotNetComponents(_project);
            string[] dotNetNames = null;
            if(_compactFramework)
            {
                dotNetNames = Util.getDotNetCompactNames();
                checkComponent("IceSSL", false);
            }
            else if(_silverlightFramework)
            {
                dotNetNames = Util.getSilverlightNames();
                checkComponent("IceSSL", false);
                checkComponent("IceBox", false);
            }
            else
            {
                dotNetNames = Util.getDotNetNames();
            }
            foreach(String s in dotNetNames)
            {
                if(selectedComponents.Contains(s))
                {
                    checkComponent(s, true);
                }
                else
                {
                    checkComponent(s, false);
                }
            }
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

        private void checkComponent(String component, bool check)
        {
            if(editingIncludeDir())
            {
                endEditIncludeDir(true);
            }
            switch(component)
            {
                case "Glacier2":
                {
                    chkGlacier2.Checked = check;
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
                            components.Add("Ice");
                        }
                    }
                    else
                    {
                        components = iceComponents();
                    }

                    //
                    // Enable / Disable the given set of components
                    //
                    for(int i = 0; i < components.Count; ++i)
                    {
                        checkComponent(components[i], chkEnableBuilder.Checked);
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
            chkTie.Enabled = enabled;
            chkStreaming.Enabled = enabled;
            chkChecksum.Enabled = enabled;
            comboBoxVerboseLevel.Enabled = enabled;
            extraCompilerOptions.setEnabled(enabled);
            includePathView.setEnabled(enabled);
            chkGlacier2.Enabled = enabled;
            chkIceBox.Enabled = enabled;
            chkIceGrid.Enabled = enabled;
            chkIcePatch2.Enabled = enabled;
            //
            // Ice .NET Compact Framework doesn't support IceSSL
            //
            if(_compactFramework)
            {
                chkIceSSL.Enabled = false;
            }
            //
            // Ice Silverlight doesn't support IceSSL and IceBox
            //
            else if(_silverlightFramework)
            {
                chkIceSSL.Enabled = false;
                chkIceBox.Enabled = false;
            }
            else
            {
                chkIceSSL.Enabled = enabled;
            }
            
            chkIceStorm.Enabled = enabled;
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
        
        private void componentChanged(string name, bool value, bool development)
        {
            if(value)
            {
                if(!Util.addDotNetReference(_project, name, development))
                {
                    checkComponent(name, false);
                }
            }
            else
            {
                Util.removeDotNetReference(_project, name);
            }
            _changed = true;
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

        private ComponentList iceComponents()
        {
            ComponentList components = new ComponentList();

            if(chkGlacier2.Checked)
            {
                components.Add("Glacier2");
            }
            if(!_silverlightFramework)
            {
                if (chkIceBox.Checked)
                {
                    components.Add("IceBox");
                }
            }
            if(chkIceGrid.Checked)
            {
                components.Add("IceGrid");
            }
            if(chkIcePatch2.Checked)
            {
                components.Add("IcePatch2");
            }
            if(!_compactFramework && !_silverlightFramework)
            {
                if(chkIceSSL.Checked)
                {
                    components.Add("IceSSL");
                }
            }
            if(chkIceStorm.Checked)
            {
                components.Add("IceStorm");
            }
            return components;
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
                    Util.addBuilderToProject(_project, iceComponents());
                    _changed = true;
                    _initialized = false;
                    loadComponents();
                    _initialized = true;
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

                if(chkTie.Checked != Util.getProjectPropertyAsBool(_project, Util.PropertyIceTie))
                {
                    Util.setProjectProperty(_project, Util.PropertyIceTie, chkTie.Checked.ToString());
                    _changed = true;
                }

                if(chkStreaming.Checked != Util.getProjectPropertyAsBool(_project, Util.PropertyIceStreaming))
                {
                    Util.setProjectProperty(_project, Util.PropertyIceStreaming, chkStreaming.Checked.ToString());
                    _changed = true;
                }

                if(chkChecksum.Checked != Util.getProjectPropertyAsBool(_project, Util.PropertyIceChecksum))
                {
                    Util.setProjectProperty(_project, Util.PropertyIceChecksum, chkChecksum.Checked.ToString());
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

                bool development = Util.developmentMode(_project);
                ComponentList components = new ComponentList();
                if(chkGlacier2.Checked != Util.hasDotNetReference(_project, "Glacier2"))
                {
                    componentChanged("Glacier2", chkGlacier2.Checked, development);
                    if(!chkGlacier2.Checked)
                    {
                        components.Add("Glacier2");
                    }
                }
                if(!_silverlightFramework)
                {
                    if (chkIceBox.Checked != Util.hasDotNetReference(_project, "IceBox"))
                    {
                        componentChanged("IceBox", chkIceBox.Checked, development);
                        if (!chkIceBox.Checked)
                        {
                            components.Add("IceBox");
                        }
                    }
                }
                if(chkIceGrid.Checked != Util.hasDotNetReference(_project, "IceGrid"))
                {
                    componentChanged("IceGrid", chkIceGrid.Checked, development);
                    if(!chkIceGrid.Checked)
                    {
                        components.Add("IceGrid");
                    }
                }
                if(chkIcePatch2.Checked != Util.hasDotNetReference(_project, "IcePatch2"))
                {
                    componentChanged("IcePatch2", chkIcePatch2.Checked, development);
                    if(!chkIcePatch2.Checked)
                    {
                        components.Add("IcePatch2");
                    }
                }
                if(!_compactFramework && !_silverlightFramework)
                {
                    if(chkIceSSL.Checked != Util.hasDotNetReference(_project, "IceSSL"))
                    {
                        componentChanged("IceSSL", chkIceSSL.Checked, development);
                        if(!chkIceSSL.Checked)
                        {
                            components.Add("IceSSL");
                        }
                    }
                }
                if(chkIceStorm.Checked != Util.hasDotNetReference(_project, "IceStorm"))
                {
                    componentChanged("IceStorm", chkIceStorm.Checked, development);
                    if(!chkIceStorm.Checked)
                    {
                        components.Add("IceStorm");
                    }
                }

                //
                // This must be the last setting to be updated, as we want to update 
                // all other settings and that isn't possible if the builder is disabled.
                //
                if(!chkEnableBuilder.Checked && Util.isSliceBuilderEnabled(_project))
                {
                    Util.removeBuilderFromProject(_project, components);
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

            if(chkTie.Checked != Util.getProjectPropertyAsBool(_project, Util.PropertyIceTie))
            {
                return true;
            }

            if(chkStreaming.Checked != Util.getProjectPropertyAsBool(_project, Util.PropertyIceStreaming))
            {
                return true;
            }

            if(chkChecksum.Checked != Util.getProjectPropertyAsBool(_project, Util.PropertyIceChecksum))
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

            // Ice libraries
            if(chkGlacier2.Checked != Util.hasDotNetReference(_project, "Glacier2"))
            {
                return true;
            }
            if(!_silverlightFramework)
            {
                if(chkIceBox.Checked != Util.hasDotNetReference(_project, "IceBox"))
                {
                    return true;
                }
            }
            if(chkIceGrid.Checked != Util.hasDotNetReference(_project, "IceGrid"))
            {
                return true;
            }
            if(chkIcePatch2.Checked != Util.hasDotNetReference(_project, "IcePatch2"))
            {
                return true;
            }
            if(!_compactFramework && !_silverlightFramework)
            {
                if(chkIceSSL.Checked != Util.hasDotNetReference(_project, "IceSSL"))
                {
                    return true;
                }
            }
            if(chkIceStorm.Checked != Util.hasDotNetReference(_project, "IceStorm"))
            {
                return true;
            }
            return false;
        }

        private bool _initialized;
        private Project _project;
        private bool _changed = false;
        private bool _compactFramework = false;
        private bool _silverlightFramework = false;
    }
}
