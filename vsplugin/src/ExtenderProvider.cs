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
using EnvDTE;
using System.Runtime.InteropServices;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.VCProjectEngine;
using Microsoft.VisualStudio.VCProject;
using System.ComponentModel;

namespace Ice.VisualStudio
{
#if VS90
  [GuidAttribute("D1F0ACA3-749A-4aae-AC7F-5CFF2A620867"), ProgId("Ice.VisualStudio.ExtenderProvider")]
#else
  [GuidAttribute("90C07B62-C9A2-434b-8F4D-061DF63873E6"), ProgId("Ice.VisualStudio.ExtenderProvider")]
#endif
    [ComVisible(true)]
    public class ExtenderProvider : Object, EnvDTE.IExtenderProvider
    {
        public ExtenderProvider()
        {
            _dte = Util.getCurrentDTE();
            if(_dte == null)
            {
                throw new ArgumentException("Cannot access DTE automation object.");
            }
        }

        public Object GetExtender(string category, string name, Object obj, EnvDTE.IExtenderSite site, int cookie)
        {
            object extender = null;
            Project prj = getProjectFromExtender(obj);
            if(CanExtend(category, name, obj) && prj != null)
            {


                if(Util.isCppProject(prj))
                {
                    CppProjectExtenderI ext = new CppProjectExtenderI();
                    ext.Init(prj, cookie, site);
                    return ext;
                }
                else if(Util.isCSharpProject(prj))
                {
                    if(Util.isSilverlightProject(prj))
                    {
                        SilverlightProjectExtenderI ext = new SilverlightProjectExtenderI();
                        ext.Init(prj, cookie, site);
                        return ext;
                    }
                    else
                    {
                        CSharpProjectExtenderI ext = new CSharpProjectExtenderI();
                        ext.Init(prj, cookie, site);
                        return ext;
                    }
                }

            }
            return extender;
        }

        public bool CanExtend(string category, string name, Object obj)
        {
            if(String.IsNullOrEmpty(category) || String.IsNullOrEmpty(name) || obj == null)
            {
                return false;
            }

            if(!name.Equals(extensionName, StringComparison.CurrentCultureIgnoreCase) ||
               !isProjectCategory(category))
            {
                return false;
            }

            Project project = getProjectFromExtender(obj);
            if(project == null)
            {
                return false;
            }

            if(!Util.isSliceBuilderEnabled(project))
            {
                return false;
            }
            return true;
        }

        private bool isProjectCategory(string category)
        {
            if(String.IsNullOrEmpty(category))
            {
                return false;
            }
            if(category.Equals(vcprojCATIDS.vcprojCATIDProjectNode, StringComparison.CurrentCultureIgnoreCase) ||
               category.Equals(VSLangProj.PrjBrowseObjectCATID.prjCATIDCSharpProjectBrowseObject, StringComparison.CurrentCultureIgnoreCase))
            {
                return true;
            }
            return false;
        }

        private Project getProjectFromExtender(object obj)
        {
            //
            // Get project properties to search Project object in DTE.Solution.Projects
            // FullPath should containt the project file full path for c# project.
            //
            PropertyDescriptor fullPathPropertyDesc = TypeDescriptor.GetProperties(obj)["FullPath"];
            PropertyDescriptor namePropertyDesc = TypeDescriptor.GetProperties(obj)["FileName"];
            if(fullPathPropertyDesc == null || namePropertyDesc == null)
            {
                //
                // VC++ projects not define a property named FullPath they define ProjectFile
                //
                namePropertyDesc = TypeDescriptor.GetProperties(obj)["ProjectFile"];
                if(namePropertyDesc == null)
                {
                    return null;
                }
            }

            string fullPath = "";
            if(fullPathPropertyDesc != null)
            {
                fullPath = fullPathPropertyDesc.GetValue(obj) as string;
            }

            string fileName = namePropertyDesc.GetValue(obj) as string;
            if(!String.IsNullOrEmpty(fullPath))
            {
                fullPath = Path.Combine(fullPath, fileName);
            }
            else
            {
                fullPath = fileName;
            }

            // find corresponding EnvDTE.Project object by its full path
            Project project = null;
            foreach(Project p in _dte.Solution.Projects)
            {
                if(p.FileName.Equals(fullPath, StringComparison.CurrentCultureIgnoreCase))
                {
                    project = p;
                    break;
                }
            }
            return project;
        }


        EnvDTE.DTE _dte;
#if VS90
        public const string extensionName = "Ice_VS90_ExtenderProvider";
#else
        public const string extensionName = "Ice_VS80_ExtenderProvider";
#endif
    }
}
