// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
using System.Windows.Forms;

namespace Ice.VisualStudio
{
    [ComVisibleAttribute(true)]
    public class FixDemoConnect : IDTExtensibility2, IDTCommandTarget
    {
        //
        // The Ice version, that is used to build the target filename for
        // plugins like helloplugin34.dll
        //
        public static string version = "34";
        public static bool removingTemplate = true;

        public void OnConnection(object application, ext_ConnectMode connectMode, object addInInst, ref Array custom)
        {
            try
            {
                _applicationObject = (DTE2)application;
                _addInInstance = (AddIn)addInInst;

                if(connectMode == ext_ConnectMode.ext_cm_Startup)
                {
                    _solutionEvents = _applicationObject.Events.SolutionEvents;
                    _solutionEvents.Opened += new _dispSolutionEvents_OpenedEventHandler(solutionOpened);
                }
            }
            catch(Exception ex)
            {
                unexpectedExceptionWarning(ex);
                throw;
            }
        }

        public void QueryStatus(string commandName, vsCommandStatusTextWanted neededText, ref vsCommandStatus status,
                                ref object commandText)
        { 
        }

        public void solutionOpened()
        {
            try
            {
                List<Project> projects = buildOrder(_applicationObject.Solution);
                foreach(Project p in projects)
                {
                    if(!isCppProject(p))
                    {
                        continue;
                    }
#if VS2010
                    _applicationObject.StatusBar.Text = "Ice Add-In fixing project: `" + p.FullName + "'";

                    if(removingTemplate)
                    {
                        //
                        // Try to remove the template configuration, the template configuration
                        // this template is a bug that occurrs with the conversion wizard
                        //
                        // http://connect.microsoft.com/VisualStudio/feedback/details/540363/
                        //
                        removeConfiguiration(p, "Template|Win32");
                        removeConfiguiration(p, "Template|x64");
                    }
                    else
                    {
                        fixProjectConfiguration(p);
                    }
#else
                    fixProjectConfiguration(p);
#endif
                }
#if VS2010
                if(removingTemplate)
                {
                    removingTemplate = false;
                    String solutionFile = _applicationObject.Solution.FullName;
                    _applicationObject.Solution.Close();
                    _applicationObject.Solution.Open(solutionFile);
                }
#endif
            }
            catch (Exception ex)
            {
                unexpectedExceptionWarning(ex);
                throw;
            }
            _applicationObject.StatusBar.Text = "Ready";
        }


        public void Exec(string commandName, vsCommandExecOption executeOption, ref object varIn, ref object varOut,
                         ref bool handled)
        {
        }

        public void OnDisconnection(ext_DisconnectMode disconnectMode, ref Array custom)
        {
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

        public static bool isCppProject(Project project)
        {
            if (project == null)
            {
                return false;
            }

            if (String.IsNullOrEmpty(project.Kind))
            {
                return false;
            }
            return project.Kind == vcContextGuids.vcContextGuidVCProject;
        }

        public static void unexpectedExceptionWarning(Exception ex)
        {
            MessageBox.Show("The Ice Visual Studio Add-In has raised an unexpected exception:\n" +
                            ex.ToString(),
                            "Ice Visual Studio Add-In", MessageBoxButtons.OK,
                            MessageBoxIcon.Error,
                            MessageBoxDefaultButton.Button1,
                            (MessageBoxOptions)0);
        }

        //
        // Rerturn a list of projects with match the build order
        // of the solution.
        //
        public static List<Project> buildOrder(Solution solution)
        {
            List<Project> projects = new List<Project>();
            foreach (Project p in solution.Projects)
            {
                buildOrder(solution, p, ref projects);
            }
            return projects;
        }


        //
        // This method helps to build the list of projects with the
        // right build order.
        //
        private static void buildOrder(Solution solution, Project project, ref List<Project> projects)
        {
            if (project.Kind == EnvDTE80.ProjectKinds.vsProjectKindSolutionFolder)
            {
                foreach (ProjectItem item in project.ProjectItems)
                {
                    Project p = item.Object as Project;
                    if (p == null)
                    {
                        continue;
                    }
                    buildOrder(solution, p, ref projects);
                }
            }
            BuildDependencies dependencies = solution.SolutionBuild.BuildDependencies;
            for (int i = 0; i < dependencies.Count; ++i)
            {
                BuildDependency dp = dependencies.Item(i + 1);
                if (dp.Project.Equals(project))
                {
                    System.Array requiredProjects = dp.RequiredProjects as System.Array;
                    if (requiredProjects == null)
                    {
                        continue;
                    }
                    foreach (Project p in requiredProjects)
                    {
                        buildOrder(solution, p, ref projects);
                    }
                }
            }
            if (projects.Find(
                delegate(Project p)
                {
                    return project.UniqueName.Equals(p.UniqueName);
                }) == null)
            {
                projects.Add(project);
            }
        }

        static public bool isDebug(runtimeLibraryOption rt)
        {
            return rt == runtimeLibraryOption.rtMultiThreadedDebug || rt == runtimeLibraryOption.rtMultiThreadedDebugDLL;
        }

#if VS2010
        private static void removeConfiguiration(Project project, string name)
        {
            Microsoft.Build.Evaluation.Project p =
                Microsoft.Build.Evaluation.ProjectCollection.GlobalProjectCollection.LoadProject(project.FullName);

            List<Microsoft.Build.Evaluation.ProjectItem> removeItems = new List<Microsoft.Build.Evaluation.ProjectItem>();
            foreach(Microsoft.Build.Evaluation.ProjectItem item in p.Items)
            {
                if(!item.ItemType.Equals("ProjectConfiguration"))
                {
                    continue;
                }

                if (!item.EvaluatedInclude.Equals(name))
                {
                    continue;
                }
                removeItems.Add(item);
            }

            foreach(Microsoft.Build.Evaluation.ProjectItem item in removeItems)
            {
                p.RemoveItem(item);
            }

            List<Microsoft.Build.Construction.ProjectImportGroupElement> imports = 
                new List<Microsoft.Build.Construction.ProjectImportGroupElement>();
            foreach(Microsoft.Build.Construction.ProjectImportGroupElement import in p.Xml.ImportGroups)
            {
                if(!import.Condition.Contains(name))
                {
                    continue;
                }
                imports.Add(import);
            }

            foreach(Microsoft.Build.Construction.ProjectImportGroupElement import in imports)
            {
                import.Parent.RemoveChild(import);
            }

            List<Microsoft.Build.Construction.ProjectPropertyGroupElement> propertyGroups =
                new List<Microsoft.Build.Construction.ProjectPropertyGroupElement>();
            foreach(Microsoft.Build.Construction.ProjectPropertyGroupElement propertyGroup in p.Xml.PropertyGroups)
            {
                if(!propertyGroup.Condition.Contains(name))
                {
                    continue;
                }
                propertyGroups.Add(propertyGroup);
            }

            foreach(Microsoft.Build.Construction.ProjectPropertyGroupElement propertyGroup in propertyGroups)
            {
                propertyGroup.Parent.RemoveChild(propertyGroup);
            }

            p.Save();
        }
#endif
        public static void fixProjectConfiguration(Project project)
        {
            VCProject vcProject = (VCProject)project.Object;
            if (vcProject == null)
            {
                return;
            }

            IVCCollection configurations = (IVCCollection)vcProject.Configurations;
            foreach(VCConfiguration conf in configurations)
            {

#if VS2008
                //
                // Ice projects only use our custom property sheet.
                //
                conf.InheritedPropertySheets = "$(ALLUSERSPROFILE)\\ZeroC\\ice.vsprops";
#endif
                bool x64 = false;
                VCPlatform platform = (VCPlatform)conf.Platform;
                if (platform == null || String.IsNullOrEmpty(platform.Name))
                {
                    continue;
                }

                if (platform.Name.Equals("x64", StringComparison.CurrentCultureIgnoreCase) ||
                    platform.Name.Equals("Itanium", StringComparison.CurrentCultureIgnoreCase))
                {
                    x64 = true;
                }

                IVCCollection tools = (IVCCollection)conf.Tools;

                
                VCCLCompilerTool compilerTool =
                    (VCCLCompilerTool)tools.Item("VCCLCompilerTool");
                VCLinkerTool linkerTool = (VCLinkerTool)tools.Item("VCLinkerTool");

                VCBscMakeTool bscMakeTool = (VCBscMakeTool)tools.Item("VCBscMakeTool");
                VCCustomBuildTool customBuildTool = (VCCustomBuildTool)tools.Item("VCCustomBuildTool");

                VCMidlTool midlTool = (VCMidlTool)tools.Item("VCMidlTool");

                VCPreLinkEventTool preLinkTool = (VCPreLinkEventTool)tools.Item("VCPreLinkEventTool");

                bool debug = isDebug(compilerTool.RuntimeLibrary);

                bool dynamicLibrary = conf.ConfigurationType ==
                    Microsoft.VisualStudio.VCProjectEngine.ConfigurationTypes.typeDynamicLibrary;

                string linkerOutputFile = conf.Evaluate("$(ProjectName)");
                bool plugin = linkerOutputFile.EndsWith("plugin");

                //
                // Dynamic Libraries use a custom output file name.
                //
                string targetName = linkerOutputFile;
                if(dynamicLibrary)
                {
                    if (plugin)
                    {
                        linkerOutputFile += version;
                    }

                    if(debug)
                    {
                        linkerOutputFile += "d";
                    }
                    targetName = linkerOutputFile;
                    linkerOutputFile += ".dll";
                    linkerTool.OutputFile = linkerOutputFile;
#if VS2010
                    IVCRulePropertyStorage generalRules = (IVCRulePropertyStorage)conf.Rules.Item("ConfigurationGeneral");
                    generalRules.SetPropertyValue("TargetName", targetName);
#endif
                }


#if VS2008
                string config = "$(ConfigurationName)\\";
#endif

#if VS2010
                string config = "$(Configuration)\\";
#endif

                conf.OutputDirectory = "$(ProjectDir)";
                //
                // That properties must use the defaults
                //
#if VS2008
                conf.ClearToolProperty(compilerTool, "SuppressStartupBanner");
                conf.ClearToolProperty(compilerTool, "RuntimeTypeInfo");
                conf.ClearToolProperty(compilerTool, "ObjectFile");
                conf.ClearToolProperty(compilerTool, "AssemblerListingLocation");
                conf.ClearToolProperty(compilerTool, "ProgramDataBaseFileName");
                conf.ClearToolProperty(compilerTool, "PrecompiledHeaderFile");
                conf.ClearToolProperty(linkerTool, "ManifestFile");
                conf.ClearToolProperty(linkerTool, "ProgramDatabaseFile");
                conf.ClearToolProperty(linkerTool, "SuppressStartupBanner");

                conf.ClearToolProperty(bscMakeTool, "SuppressStartupBanner");
                conf.ClearToolProperty(bscMakeTool, "OutputFile");

                conf.ClearToolProperty(midlTool, "TypeLibraryName");
                conf.ClearToolProperty(midlTool, "HeaderFileName");
                conf.ClearToolProperty(midlTool, "TargetEnvironment");

                conf.ClearToolProperty(customBuildTool, "Description");
                conf.ClearToolProperty(customBuildTool, "CommandLine");
                conf.ClearToolProperty(customBuildTool, "Outputs");
                conf.ClearToolProperty(customBuildTool, "AdditionalDependencies");


                conf.ClearToolProperty(preLinkTool, "Description");
                conf.ClearToolProperty(preLinkTool, "CommandLine");

                //
                // Dynamic Libraries don't use default output file.
                //
                if(!dynamicLibrary)
                {
                    conf.ClearToolProperty(linkerTool, "OutputFile");
                }
#endif
                if (!x64)
                {
                    conf.IntermediateDirectory = config + "$(ProjectName)\\";
                }
                else
                {
                    conf.BuildLogFile = "";
                    conf.IntermediateDirectory = config + "$(PlatformName)\\$(ProjectName)\\";
                }
            }

        }

        private DTE2 _applicationObject;
        private AddIn _addInInstance;
        private SolutionEvents _solutionEvents;
    }
}
