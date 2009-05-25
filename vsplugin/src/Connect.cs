// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
    [ComVisible(true)]
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

        public static IVsSolution getIVsSolution()
        {
            return (IVsSolution)Package.GetGlobalService(typeof(IVsSolution));
        }


        public void OnConnection(object application, ext_ConnectMode connectMode, object addInInst, ref Array custom)
        {
            _applicationObject = (DTE2)application;
            _addInInstance = (AddIn)addInInst;

            if(_builder == null)
            {
                _builder = new Builder();
                _builder.init(_applicationObject, _addInInstance);
            }
        }

        public void QueryStatus(string commandName, vsCommandStatusTextWanted neededText, ref vsCommandStatus status, ref object commandText)
        {
            if(neededText == vsCommandStatusTextWanted.vsCommandStatusTextWantedNone)
            {
                if(commandName == "Ice.VisualStudio.Connect.AddSliceBuilder")
                {
                    Builder builder = getBuilder();
                    if(builder == null)
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
                    if(Util.isSliceBuilderEnabled(project))
                    {
                        status = (vsCommandStatus)vsCommandStatus.vsCommandStatusSupported;
                    }
                    else
                    {
                        status = (vsCommandStatus)vsCommandStatus.vsCommandStatusSupported | vsCommandStatus.vsCommandStatusEnabled;
                    }
                }
                else if(commandName == "Ice.VisualStudio.Connect.RemoveSliceBuilder")
                {
                    Builder builder = getBuilder();
                    if(builder == null)
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
                    if(Util.isSliceBuilderEnabled(project))
                    {
                        status = (vsCommandStatus)vsCommandStatus.vsCommandStatusSupported | vsCommandStatus.vsCommandStatusEnabled;
                    }
                    else
                    {
                        status = (vsCommandStatus)vsCommandStatus.vsCommandStatusSupported;
                    }
                }
            }
        }

        public void Exec(string commandName, vsCommandExecOption executeOption, ref object varIn, ref object varOut, ref bool handled)
        {
            handled = false;
            Builder builder = getBuilder();
            if(builder == null)
            {
                return;
            }

            if(executeOption == vsCommandExecOption.vsCommandExecOptionDoDefault)
            {
                if(commandName == "Ice.VisualStudio.Connect.AddSliceBuilder")
                {
                    builder.addBuilderToProject(builder.getSelectedProject());
                    handled = true;
                }
                else if(commandName == "Ice.VisualStudio.Connect.RemoveSliceBuilder")
                {
                    builder.removeBuilderFromProject(builder.getSelectedProject());
                    handled = true; 
                }
            }
            if(handled)
            {
                ServiceProvider serviceProvider = new ServiceProvider(
                (Microsoft.VisualStudio.OLE.Interop.IServiceProvider)_applicationObject.DTE);
                IVsUIShell shell = (IVsUIShell) serviceProvider.GetService(typeof(IVsUIShell));
                if(shell != null)
                {
                    shell.RefreshPropertyBrowser(0);
                }
            }
        }

        public void OnDisconnection(ext_DisconnectMode disconnectMode, ref Array custom)
        {
            if(disconnectMode == ext_DisconnectMode.ext_dm_HostShutdown ||
               disconnectMode == ext_DisconnectMode.ext_dm_UserClosed)
            {
                _builder.disconnect();
                _builder = null;
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
