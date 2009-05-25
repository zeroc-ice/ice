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
using System.Collections.Generic;
using System.ComponentModel;
using EnvDTE;
using System.Runtime.InteropServices;
using System.Drawing.Design;
using Microsoft.VisualStudio.VCProjectEngine;
using Microsoft.VisualStudio.VCProject;

namespace Ice.VisualStudio
{
    [System.Runtime.InteropServices.ClassInterface(System.Runtime.InteropServices.ClassInterfaceType.None)]
    [ComVisible(true)]
    public class CppProjectExtenderI : ProjectExtenderI
    {
        [Category("Ice"), Description("Ice Component Libraries"), DisplayName("Ice Components"),
         Editor(typeof(Ice.VisualStudio.IceLibraryEditor), typeof(UITypeEditor))]
        public ComponentList IceRefComponents
        {
            get
            {
                return Util.getIceCppComponents(getProject());
            }
        }
    }
}
