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
using EnvDTE;
using System.Runtime.InteropServices;
using System.Drawing.Design;
using System.Windows.Forms;

namespace Ice.VisualStudio
{

    [System.Runtime.InteropServices.ClassInterface(System.Runtime.InteropServices.ClassInterfaceType.None)]
    [ComVisible(true)]
    public class ProjectExtenderI 
    {
        ~ProjectExtenderI()
        {
            // Wrap this call in a try-catch to avoid any failure code the
            // Site may return. For instance, since this object is GC'ed,
            // the Site may have already let go of its Cookie.
            try
            {
                if(_site != null)
                {
                    _site.NotifyDelete(_cookie);
                }
            }
            catch 
            {
            }
        }

        public void Init(EnvDTE.Project project, int cookie, EnvDTE.IExtenderSite site)
        {
            _project = project;
            _cookie = cookie;
            _site = site;
        }

        [Category("Ice"), Description("Slice include path"), DisplayName("Slice Include Path"),
         Editor(typeof(IncludePathEditor), typeof(UITypeEditor))]
        public IncludePathList SliceIncludePath
        {
            get
            {
                return new IncludePathList(Util.getProjectProperty(_project, Util.PropertyNames.IceIncludePath));
            }
            set
            {
                Util.setProjectProperty(_project, Util.PropertyNames.IceIncludePath, value.ToString());
            }
        }

        [Category("Ice"), Description("Macros"), DisplayName("Preprocessor Macros"),
         Editor(typeof(PreprocessorMacroEditor), typeof(UITypeEditor))]
        public string IceMacros
        {
            get
            {
                return Util.getProjectProperty(_project, Util.PropertyNames.IceMacros);
            }
            set
            {
                Util.setProjectProperty(_project, Util.PropertyNames.IceMacros, value);
            }
        }

        [Category("Ice"), Description("Ice Home"), DisplayName("Ice Home"),
         Editor(typeof(IceHomeEditor), typeof(UITypeEditor))]
        public string IceHome
        {
            get
            {
                return Util.getProjectProperty(_project, Util.PropertyNames.IceHome, Util.getIceHome(_project));
            }
            set
            {
                if(Util.isSilverlightProject(_project))
                {
                    if (!File.Exists(value + "\\bin\\slice2sl.exe") || !Directory.Exists(value + "\\slice\\Ice"))
                    {
                        System.Windows.Forms.MessageBox.Show("Could not locate Ice for Silverlight installation in '"
                                                             + value + "' directory.\n",
                                                             "Ice Visual Studio Extension", MessageBoxButtons.OK,
                                                             MessageBoxIcon.Error);
                        return;
                    }
                }
                else
                {
                    if(!File.Exists(value + "\\bin\\slice2cpp.exe") || !File.Exists(value + "\\bin\\slice2cs.exe") ||
                       !Directory.Exists(value + "\\slice\\Ice"))
                    {
                        System.Windows.Forms.MessageBox.Show("Could not locate Ice installation in '"
                                                             + value + "' directory.\n",
                                                             "Ice Visual Studio Extension", MessageBoxButtons.OK,
                                                             MessageBoxIcon.Error);
                        return;
                    }
                }
                Util.updateIceHome(_project, value);
            }
        }

        [Category("Ice"), DisplayName("Enable Streaming"), Description("Enable Streaming Support")]
        public bool Streaming
        {
            get
            {
                return Util.getProjectPropertyAsBool(_project, Util.PropertyNames.IceStreaming);
            }
            set
            {
                Util.setProjectProperty(_project, Util.PropertyNames.IceStreaming, value.ToString());
            }
        }

        [Category("Ice"), DisplayName("Enable Ice"), Description("Enable Ice Prefix")]
        public bool EnableIce
        {
            get
            {
                return Util.getProjectPropertyAsBool(_project, Util.PropertyNames.IcePrefix);
            }
            set
            {
                Util.setProjectProperty(_project, Util.PropertyNames.IcePrefix, value.ToString());
            }
        }

        protected Project getProject()
        {
            return _project;
        }

        private EnvDTE.Project _project;
        private int _cookie;
        private EnvDTE.IExtenderSite _site;
    }
}
