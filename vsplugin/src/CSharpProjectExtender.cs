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

namespace Ice.VisualStudio
{
    [System.Runtime.InteropServices.ClassInterface(System.Runtime.InteropServices.ClassInterfaceType.None)]
    [ComVisible(true)]
    public class CSharpProjectExtenderI : ProjectExtenderI
    {

        [Category("Ice"), DisplayName("Generate Tie"), Description("Generate tie classes (--tie)")]
        public bool Tie
        {
            get
            {
                Project project = getProject();
                EnvDTE.Globals glob = project.Globals;
                if(!glob.get_VariableExists(Util.PropertyNames.IceTie))
                {
                    glob[Util.PropertyNames.IceTie] = false.ToString();
                    glob.set_VariablePersists(Util.PropertyNames.IceTie, true);
                }
                return glob[Util.PropertyNames.IceTie].ToString().ToUpper() == true.ToString().ToUpper();
            }
            set
            {
                Project project = getProject();
                EnvDTE.Globals glob = project.Globals;
                glob[Util.PropertyNames.IceTie] = value.ToString();
                glob.set_VariablePersists(Util.PropertyNames.IceTie, true);
            }
        }

        [Category("Ice"), Description("Ice Components Referenced In This Project"), DisplayName("Ice Components"),
         Editor(typeof(Ice.VisualStudio.IceComponentEditor), typeof(UITypeEditor))]
        public ComponentList IceRefComponents
        {
            get
            {
                return Util.getIceCSharpComponents(getProject());
            }
        }
    }
}
