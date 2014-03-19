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
using Microsoft.VisualStudio.VCProjectEngine;

namespace Ice.VisualStudio
{
    public partial class IceCppConfigurationDialog : Form, IceConfigurationDialog
    {
        public IceCppConfigurationDialog(Project project)
        {
            InitializeComponent();            
            _project = project;
            _winRT = Util.isWinRTProject(_project);

            if(_winRT)
            {
                chkFreeze.Enabled = false;
                chkFreeze.Checked = false;
                chkGlacier2.Enabled = false;
                chkGlacier2.Checked = false;
                chkIceBox.Enabled = false;
                chkIceBox.Checked = false;
                chkIceGrid.Enabled = false;
                chkIceGrid.Checked = false;
                chkIcePatch2.Enabled = false;
                chkIcePatch2.Checked = false;
                chkIceSSL.Enabled = false;
                chkIceSSL.Checked = false;
                chkIceStorm.Enabled = false;
                chkIceStorm.Checked = false;
            }

            outputDirView.init(this, _project);
            includePathView.init(this, _project);
            extraCompilerOptions.init(this, project);
            
            //
            // Set the toolTip messages.
            //
            
            toolTip.SetToolTip(chkStreaming, "Generate marshaling support for stream API (--stream).");
            toolTip.SetToolTip(chkChecksum, "Generate checksums for Slice definitions (--checksum).");
            toolTip.SetToolTip(chkIcePrefix, "Permit Ice prefixes (--ice).");

            if(_project != null)
            {
                this.Text = "Ice Configuration - Project: " + _project.Name;
                bool enabled = Util.isSliceBuilderEnabled(project);
                VCConfiguration conf =  Util.getActiveVCConfiguration(project);
                _staticLib = conf.ConfigurationType == ConfigurationTypes.typeStaticLibrary;
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

            chkStreaming.Checked = Util.getProjectPropertyAsBool(_project, Util.PropertyIceStreaming);
            chkChecksum.Checked = Util.getProjectPropertyAsBool(_project, Util.PropertyIceChecksum);

            comboBoxVerboseLevel.SelectedIndex = Util.getVerboseLevel(_project);

            includePathView.load();

            loadComponents();
            txtDllExportSymbol.Text = Util.getProjectProperty(_project, Util.PropertyIceDllExport);

            btnApply.Enabled = false;
            Cursor = Cursors.Default;
        }

        private void loadComponents()
        {
            ComponentList selectedComponents = Util.getIceCppComponents(_project);
            foreach(String s in Util.getCppNames())
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

        private void checkComponent(String component, bool check)
        {
            if(editingIncludeDir())
            {
                endEditIncludeDir(true);
            }
            switch(component)
            {
                case "Freeze":
                {
                    chkFreeze.Checked = check;
                    break;
                }
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
                        // Enable the components that were previously enabled if any.
                        //
                        components = 
                            new ComponentList(Util.getProjectProperty(_project, Util.PropertyIceComponents));

                        //
                        // If there isn't a previous set of componets, we enable the default components.
                        //
                        if(components.Count == 0)
                        {
                            components.Add("Ice");
                            components.Add("IceUtil");
                        }
                        for(int i = 0; i < components.Count; ++i)
                        {
                            checkComponent(components[i], true);
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
            chkStreaming.Enabled = enabled;
            chkChecksum.Enabled = enabled;

            comboBoxVerboseLevel.Enabled = enabled;
            includePathView.setEnabled(enabled);

            extraCompilerOptions.setEnabled(enabled);
            txtDllExportSymbol.Enabled = enabled;

            if(!_winRT)
            {
                chkFreeze.Enabled = _staticLib ? false : enabled;
                chkIceBox.Enabled = _staticLib ? false : enabled;
                chkIcePatch2.Enabled = _staticLib ? false : enabled;
                chkIceSSL.Enabled = _staticLib ? false : enabled;
                chkGlacier2.Enabled = _staticLib ? false : enabled;
                chkIceGrid.Enabled = _staticLib ? false : enabled;
                chkIceStorm.Enabled = _staticLib ? false : enabled;
            }
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

        private void txtDllExportSymbol_Focus(object sender, EventArgs e)
        {
            try
            {
                Cursor = Cursors.WaitCursor;
                if(includePathView.editingIncludeDir())
                {
                    includePathView.endEditIncludeDir(true);
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
        
        private void txtDllExportSymbol_Changed(object sender, EventArgs e)
        {
            try
            {
                Cursor = Cursors.WaitCursor;
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

        private ComponentList iceComponents()
        {
            ComponentList components = new ComponentList();
            if(chkFreeze.Checked)
            {
                components.Add("Freeze");
            }
            if(chkGlacier2.Checked)
            {
                components.Add("Glacier2");
            }
            if(chkIceBox.Checked)
            {
                components.Add("IceBox");
            }
            if(chkIceGrid.Checked)
            {
                components.Add("IceGrid");
            }
            if(chkIcePatch2.Checked)
            {
                components.Add("IcePatch2");
            }
            if(chkIceSSL.Checked)
            {
                components.Add("IceSSL");
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
                bool enabling = false;
                if(chkEnableBuilder.Checked && !Util.isSliceBuilderEnabled(_project))
                {
                    Util.addBuilderToProject(_project, iceComponents());
                    if(!_winRT)
                    {
                        _changed = true;
                        _initialized = false;
                        loadComponents();
                        _initialized = true;
                        enabling = true;
                    }
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

                if(!txtDllExportSymbol.Text.Equals(Util.getProjectProperty(_project, Util.PropertyIceDllExport),
                                                   StringComparison.CurrentCulture))
                {
                    Util.setProjectProperty(_project, Util.PropertyIceDllExport, txtDllExportSymbol.Text);
                    _changed = true;
                }

                ComponentList components = new ComponentList();
                if(!enabling)
                {
                    
                    if(!_winRT && chkFreeze.Checked != Util.hasIceCppLib(_project, "Freeze"))
                    {
                        componentChanged("Freeze", chkFreeze.Checked);
                        if(!chkFreeze.Checked)
                        {
                            components.Add("Freeze");
                        }
                    }
                    if(!_winRT && chkGlacier2.Checked != Util.hasIceCppLib(_project, "Glacier2"))
                    {
                        componentChanged("Glacier2", chkGlacier2.Checked);
                        if(!chkGlacier2.Checked)
                        {
                            components.Add("Glacier2");
                        }
                    }
                    if(!_winRT && chkIceBox.Checked != Util.hasIceCppLib(_project, "IceBox"))
                    {
                        componentChanged("IceBox", chkIceBox.Checked);
                        if(!chkIceBox.Checked)
                        {
                            components.Add("IceBox");
                        }
                    }
                    if(!_winRT && chkIceGrid.Checked != Util.hasIceCppLib(_project, "IceGrid"))
                    {
                        componentChanged("IceGrid", chkIceGrid.Checked);
                        if(!chkIceGrid.Checked)
                        {
                            components.Add("IceGrid");
                        }
                    }
                    if(!_winRT && chkIcePatch2.Checked != Util.hasIceCppLib(_project, "IcePatch2"))
                    {
                        componentChanged("IcePatch2", chkIcePatch2.Checked);
                        if(!chkIcePatch2.Checked)
                        {
                            components.Add("IcePatch2");
                        }
                    }
                    if(!_winRT && chkIceSSL.Checked != Util.hasIceCppLib(_project, "IceSSL"))
                    {
                        componentChanged("IceSSL", chkIceSSL.Checked);
                        if(!chkIceSSL.Checked)
                        {
                            components.Add("IceSSL");
                        }
                    }
                    if(!_winRT && chkIceStorm.Checked != Util.hasIceCppLib(_project, "IceStorm"))
                    {
                        componentChanged("IceStorm", chkIceStorm.Checked);
                        if(!chkIceStorm.Checked)
                        {
                            components.Add("IceStorm");
                        }
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

            if(!txtDllExportSymbol.Text.Trim().Equals(Util.getProjectProperty(_project, Util.PropertyIceDllExport),
                                                      StringComparison.CurrentCulture))
            {
                return true;
            }

            if(includePathView.hasUnsavedChanges())
            {
                return true;
            }

            if(!_staticLib && !_winRT)
            {
                // Ice libraries
                if(chkFreeze.Checked != Util.hasIceCppLib(_project, "Freeze"))
                {
                    return true;
                }
                if(chkGlacier2.Checked != Util.hasIceCppLib(_project, "Glacier2"))
                {
                    return true;
                }
                if(chkIceBox.Checked != Util.hasIceCppLib(_project, "IceBox"))
                {
                    return true;
                }
                if(chkIceGrid.Checked != Util.hasIceCppLib(_project, "IceGrid"))
                {
                    return true;
                }
                if(chkIcePatch2.Checked != Util.hasIceCppLib(_project, "IcePatch2"))
                {
                    return true;
                }
                if(chkIceSSL.Checked != Util.hasIceCppLib(_project, "IceSSL"))
                {
                    return true;
                }
                if(chkIceStorm.Checked != Util.hasIceCppLib(_project, "IceStorm"))
                {
                    return true;
                }
            }
            return false;
        }

        private void componentChanged(String name, bool isChecked)
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

        private bool _initialized;
        private Project _project;
        private bool _staticLib = false;
        private bool _changed = false;
        private bool _winRT = false;
    }
}
