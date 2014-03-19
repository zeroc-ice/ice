// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
    [ComVisibleAttribute(true)]
    public class Connect : IDTExtensibility2, IDTCommandTarget
    {
        [ComVisibleAttribute(false)]
        public static Builder getBuilder()
        {
            return _builder;
        }

        [ComVisibleAttribute(false)]
        public static DTE getCurrentDTE()
        {
            if (_builder == null)
            {
                return null;
            }
            return _builder.getCurrentDTE();
        }

        public static DTE2 getApplicationObject()
        {
            if (_builder == null)
            {
                return null;
            }
            return _builder.getApplicationObject();
        }

        public void OnConnection(object application, ext_ConnectMode connectMode, object addInInst, ref Array custom)
        {
            try
            {
                _applicationObject = (DTE2)application;
                _addInInstance = (AddIn)addInInst;
                if(connectMode == ext_ConnectMode.ext_cm_Startup || connectMode == ext_ConnectMode.ext_cm_CommandLine)
                {
                    if(_builder == null)
                    {

                        //
                        // This property is set to false to avoid VC++ "no matching rule" dialog, the property
                        // doesn't exist in VS2010
                        //
#if VS2008
                        try
                        {
                            EnvDTE.Properties props = _applicationObject.get_Properties("Projects", "VCGeneral");
                            EnvDTE.Property prop = props.Item("ShowNoMatchingRuleDlg");
                            prop.Value = false;
                        }
                        catch(System.Runtime.InteropServices.COMException)
                        {
                            // Can happen if C++ support is not installed.
                        }
#endif
                        Builder.commandLine = connectMode == ext_ConnectMode.ext_cm_CommandLine;
                        _builder = new Builder();
                        _builder.init(_applicationObject, connectMode, _addInInstance);
                    }
                }
            }
            catch(Exception ex)
            {
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
        }

        //
        // VS calls this method to retrieve the status of the add-in commands.
        //
        public void QueryStatus(string commandName, vsCommandStatusTextWanted neededText, ref vsCommandStatus status,
                                ref object commandText)
        {
            try
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
            catch(Exception ex)
            {
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
        }

        public void Exec(string commandName, vsCommandExecOption executeOption, ref object varIn, ref object varOut,
                         ref bool handled)
        {
            try
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
                            handled = false;
                            return;
                        }

                        if(Util.isCSharpProject(project))
                        {
                                IceCsharpConfigurationDialog dialog = new IceCsharpConfigurationDialog(project);
                                dialog.ShowDialog();
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
            catch(Exception ex)
            {
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
        }

        public void OnDisconnection(ext_DisconnectMode disconnectMode, ref Array custom)
        {
            try
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
            catch(Exception ex)
            {
                Util.unexpectedExceptionWarning(ex);
                throw;
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
