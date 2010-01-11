// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Text;
using System.IO;
using System.Diagnostics;
using System.Collections.Generic;
using Extensibility;
using EnvDTE;
using EnvDTE80;
using Microsoft.VisualStudio.CommandBars;
using Microsoft.VisualStudio.VCProjectEngine;
using Microsoft.VisualStudio.VCProject;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using System.Resources;
using System.Reflection;
using System.Globalization;
using System.Runtime.InteropServices;

namespace Ice.VisualStudio
{
    public class Connect : IDTExtensibility2, IDTCommandTarget
    {

        public static Builder getBuilder()
        {
            return _builder;
        }

        public static DTE getCurrentDTE()
        {
            return _builder.getCurrentDTE();
        }


        public void OnConnection(object application, ext_ConnectMode connectMode, object addInInst, ref Array custom)
        {
            _applicationObject = (DTE2)application;
            _addInInstance = (AddIn)addInInst;

            if(connectMode == ext_ConnectMode.ext_cm_Startup)
            {
                if(_builder == null)
                {

                    //
                    // This property is set to false to avoid VC++ "not maching rule" dialog
                    //
                    EnvDTE.Properties props = _applicationObject.get_Properties("Projects", "VCGeneral");
                    EnvDTE.Property prop = props.Item("ShowNoMatchingRuleDlg");
                    prop.Value = false;

                    _builder = new Builder();
                    _builder.init(_applicationObject, _addInInstance);
                }
            }
        }


        //
        // VS call this method to retrive the status of AddIn commands.
        //
        public void QueryStatus(string commandName, vsCommandStatusTextWanted neededText, ref vsCommandStatus status,
                                ref object commandText)
        {
            if(neededText == vsCommandStatusTextWanted.vsCommandStatusTextWantedNone)
            {
                if(commandName == "Ice.VisualStudio.Connect.IceConfiguration")
                {
                    Builder builder = getBuilder();
                    if(builder == null)
                    {
                        status = (vsCommandStatus)vsCommandStatus.vsCommandStatusSupported;
                        return;
                    }
                    if(builder.isBuilding())
                    {
                        status = (vsCommandStatus)vsCommandStatus.vsCommandStatusSupported;
                        return;
                    }
                    Project project = builder.getSelectedProject();

                    if(project == null)
                    {
                       ProjectItem item = Util.getSelectedProjectItem(_applicationObject.DTE);
                       if(item != null)
                       {
                           project = item.ContainingProject;
                       }
                    }

                    if(project == null)
                    {
                        status = (vsCommandStatus)vsCommandStatus.vsCommandStatusSupported;
                        return;
                    }

                    if(!Util.isCppProject(project) && !Util.isCSharpProject(project) && !Util.isVBProject(project))
                    {
                        status = (vsCommandStatus)vsCommandStatus.vsCommandStatusSupported;
                        return;
                    }
                    status = (vsCommandStatus)vsCommandStatus.vsCommandStatusSupported |
                                              vsCommandStatus.vsCommandStatusEnabled;
                }
            }
        }

        public void Exec(string commandName, vsCommandExecOption executeOption, ref object varIn, ref object varOut,
                         ref bool handled)
        {
            handled = false;
            Builder builder = getBuilder();
            if(builder == null)
            {
                return;
            }

            if(executeOption == vsCommandExecOption.vsCommandExecOptionDoDefault)
            {
                if(commandName == "Ice.VisualStudio.Connect.IceConfiguration")
                {
                    Project project = builder.getSelectedProject();
                    if(project == null)
                    {
                       ProjectItem item = Util.getSelectedProjectItem(_applicationObject.DTE);
                       if(item != null)
                       {
                           project = item.ContainingProject;
                       }
                    }

                    if(project == null)
                    {
                        handled = false;
                        return;
                    }

                    if(Util.isCSharpProject(project))
                    {
                        if(Util.isSilverlightProject(project))
                        {
                            IceSilverlightConfigurationDialog dialog = new IceSilverlightConfigurationDialog(project);
                            dialog.ShowDialog();
                        }
                        else
                        {
                            IceCsharpConfigurationDialog dialog = new IceCsharpConfigurationDialog(project);
                            dialog.ShowDialog();
                        }
                    }
                    else if(Util.isVBProject(project))
                    {
                        IceVBConfigurationDialog dialog = new IceVBConfigurationDialog(project);
                        dialog.ShowDialog();
                    }
                    else if(Util.isCppProject(project))
                    {
                        IceCppConfigurationDialog dialog = new IceCppConfigurationDialog(project);
                        dialog.ShowDialog();
                    }
                    handled = true;
                }
            }
        }

        public void OnDisconnection(ext_DisconnectMode disconnectMode, ref Array custom)
        {
            if(disconnectMode == ext_DisconnectMode.ext_dm_HostShutdown ||
               disconnectMode == ext_DisconnectMode.ext_dm_UserClosed)
            {
                if(_builder != null)
                {
                    _builder.disconnect();
                    _builder.Dispose();
                    _builder = null;
                }
            }
        }

        public void OnAddInsUpdate(ref Array custom)
        {
        }

        public void OnStartupComplete(ref Array custom)
        {
        }
        public void OnBeginShutdown(ref Array custom)
        {
        }

        private DTE2 _applicationObject;
        private AddIn _addInInstance;
        private static Builder _builder;
    }
}
