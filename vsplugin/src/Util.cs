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
using System.Collections.Generic;
using System.ComponentModel;
using EnvDTE;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.IO;
using System.Diagnostics;
using Extensibility;
using EnvDTE80;
using Microsoft.VisualStudio.CommandBars;
using Microsoft.VisualStudio.VCProjectEngine;
using Microsoft.VisualStudio.VCProject;
using Microsoft.VisualStudio.Shell;
using System.Resources;
using System.Reflection;
using VSLangProj;
using System.Globalization;

using System.Collections;
using System.Runtime.InteropServices.ComTypes;
using Microsoft.CSharp;

namespace Ice.VisualStudio
{
    public class ComponentList : List<string>
    {
        public ComponentList()
        {
        }

        public ComponentList(string[] values)
        {
            foreach(string s in values)
            {
                Add(s);
            }
        }

        public new void Add(string value)
        {
            value = value.Trim();
            if(!base.Contains(value))
            {
                base.Add(value);
            }
        }

        public new bool Contains(string value)
        {
            string found = base.Find(delegate(string s)
                                    {
                                        return s.Equals(value, StringComparison.CurrentCultureIgnoreCase);
                                    });
            return !String.IsNullOrEmpty(found);
        }

        public new void Remove(string value)
        {
            string found = base.Find(delegate(string s)
            {
                return s.Equals(value, StringComparison.CurrentCultureIgnoreCase);
            });

            if(!String.IsNullOrEmpty(found))
            {
                base.Remove(found);
            }
        }

        public ComponentList(string value)
        {
            init(value, ';');
        }

        public ComponentList(string value, char separator)
        {
            init(value, separator);
        }

        private void init(string value, char separator)
        {
            Array items = value.Split(separator);
            foreach(string s in items)
            {
                string trimmed = s.Trim();
                if(trimmed.Length > 0)
                {
                    Add(trimmed);
                }
            }
        }

        public override string ToString()
        {
            return ToString(';');
        }

        public string ToString(char separator)
        {
            StringBuilder sb = new StringBuilder();
            for(int cont = 0; cont < this.Count; ++cont)
            {
                sb.Append(this[cont]);
                if(cont < this.Count - 1)
                {
                    if(!separator.Equals(' '))
                    {
                        sb.Append(' ');
                    }
                    sb.Append(separator);
                    if(!separator.Equals(' '))
                    {
                        sb.Append(' ');
                    }
                }
            }
            return sb.ToString();
        }
    }

    public class IncludePathList : ComponentList
    {
        public IncludePathList()
            : base()
        {
        }

        public IncludePathList(string[] values)
            : base(values)
        {
        }

        public IncludePathList(string value)
            : base(value, '|')
        {
        }

        public override string ToString()
        {
            return base.ToString('|');
        }
    }

    public static class Util
    {
        public const string slice2cs = "slice2cs.exe";
        public const string slice2cpp = "slice2cpp.exe";
        public const string slice2sl = "slice2sl.exe";

        //
        // Property names used to persist project configuration.
        //
        public const string PropertyIce = "ZerocIce_Enabled";
        public const string PropertyIceHome = "ZerocIce_Home";
        public const string PropertyIceComponents = "ZerocIce_Components";
        public const string PropertyIceExtraOptions = "ZerocIce_ExtraOptions";
        public const string PropertyIceIncludePath = "ZerocIce_IncludePath";
        public const string PropertyIceStreaming = "ZerocIce_Streaming";
        public const string PropertyIceChecksum = "ZerocIce_Checksum";
        public const string PropertyIceTie = "ZerocIce_Tie";
        public const string PropertyIcePrefix = "ZerocIce_Prefix";
        public const string PropertyIceDllExport = "ZerocIce_DllExport";
        public const string PropertyConsoleOutput = "ZerocIce_ConsoleOutput";
        public const string EnvIceHome = "Zeroc_VS_IceHome";

        private static readonly string[] silverlightNames =
        {
            "IceSL"
        };

        public static string[] getSilverlightNames()
        {
            return (string[])silverlightNames.Clone();
        }

        private static readonly string[] cppNames =
        {
            "Freeze", "Glacier2", "Ice", "IceBox", "IceGrid", "IcePatch2", 
            "IceSSL", "IceStorm", "IceUtil" 
        };

        public static string[] getCppNames()
        {
            return (string[])cppNames.Clone();
        }

        private static readonly string[] dotNetNames =
        {
            "Glacier2", "Ice", "IceBox", "IceGrid", "IcePatch2", 
            "IceSSL", "IceStorm"
        };

        public static string[] getDotNetNames()
        {
            return (string[])dotNetNames.Clone();
        }

        // Silverlight handled separately because it is not part of the normal product install. It is a separate add-in.
        const string iceSilverlightHome = "C:\\IceSL-0.3.3";

        private static string getIceHomeDefault()
        {
            string defaultIceHome = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
            if (defaultIceHome.EndsWith("\\bin", StringComparison.CurrentCultureIgnoreCase))
            {
                defaultIceHome = defaultIceHome.Substring(0, defaultIceHome.Length - "\\bin".Length);
            }
            return defaultIceHome;
        }

        public static string getIceHomeRaw(Project project, bool update)
        {
            if (Util.isSilverlightProject(project))
            {
                return Util.getProjectProperty(project, Util.PropertyIceHome, iceSilverlightHome, update);
            }

            string iceHome = Util.getProjectProperty(project, Util.PropertyIceHome, "", update);
            if (String.IsNullOrEmpty(iceHome))
            {
                iceHome = getIceHomeDefault();
            }
            return iceHome;
        }

        public static string getIceHome(Project project)
        {
            string iceHome = subEnvironmentVars(getIceHomeRaw(project, true));
            Environment.SetEnvironmentVariable(EnvIceHome, iceHome, EnvironmentVariableTarget.User);
            return iceHome;
        }

        public static string getPathRelativeToProject(ProjectItem item)
        {
            StringBuilder path = new StringBuilder();
            if (item != null)
            {
                path.Append(Util.getPathRelativeToProject(item, item.ContainingProject.ProjectItems));
            }
            return Util.normalizePath(path.ToString());
        }

        public static string getPathRelativeToProject(ProjectItem item, ProjectItems items)
        {
            StringBuilder path = new StringBuilder();
            foreach (ProjectItem i in items)
            {
                if (i == item)
                {
                    if (path.Length > 0)
                    {
                        path.Append("\\");
                    }
                    path.Append(i.Name);
                    break;
                }
                else if (Util.isProjectItemFilter(i) || Util.isProjectItemFolder(i))
                {
                    string token = Util.getPathRelativeToProject(item, i.ProjectItems);
                    if (!String.IsNullOrEmpty(token))
                    {
                        path.Append(i.Name);
                        path.Append("\\");
                        path.Append(token);
                        break;
                    }
                }
            }
            return path.ToString();
        }

        public static void addCppIncludes(VCCLCompilerTool tool, Project project, string iceHomeRaw)
        {
            if (tool == null || project == null)
            {
                return;
            }

            string iceIncludeDir = iceHomeRaw;     
            if (Directory.Exists(Path.Combine(Util.absolutePath(project, iceIncludeDir), "cpp\\include")))
            {
                iceIncludeDir = Path.Combine(iceIncludeDir, "cpp");
            }
            iceIncludeDir = quote(Path.Combine(iceIncludeDir, "include"));

            string additionalIncludeDirectories = tool.AdditionalIncludeDirectories;
            if (String.IsNullOrEmpty(additionalIncludeDirectories))
            {
                tool.AdditionalIncludeDirectories = iceIncludeDir + Path.PathSeparator + ".";
                return;
            }

            ComponentList includes = new ComponentList(additionalIncludeDirectories);
            bool changed = false;
            if (!includes.Contains(iceIncludeDir))
            {
                changed = true;
                includes.Add(iceIncludeDir);
            }

            if (!includes.Contains("."))
            {
                changed = true;
                includes.Add(".");
            }

            if (changed)
            {
                tool.AdditionalIncludeDirectories = includes.ToString();
            }
        }

        public static void removeCppIncludes(VCCLCompilerTool tool, string iceHomeRaw)
        {
            if (tool == null || String.IsNullOrEmpty(tool.AdditionalIncludeDirectories))
            {
                return;
            }

            string[] _cppIncludeDirs =
            {
                "include",
                "cpp\\include",
            };

            ComponentList includes = new ComponentList(tool.AdditionalIncludeDirectories);
            foreach (string dir in _cppIncludeDirs)
            {
                includes.Remove(quote(Path.Combine(iceHomeRaw, dir)));
                includes.Remove(Path.Combine(iceHomeRaw, dir));
            }
            tool.AdditionalIncludeDirectories = includes.ToString();
        }

        private static readonly string[] _csBinDirs =
        {
            "\\bin\\",
            "\\cs\\bin\\",
            "\\sl\\bin\\",
        };

        public static void addDotNetReference(Project project, string component, string iceHomeRel)
        {
            if (project == null || String.IsNullOrEmpty(component))
            {
                return;
            }

            string iceHome = Util.absolutePath(project, iceHomeRel);
            foreach (string dir in _csBinDirs)
            {
                if (Directory.Exists(iceHome + dir))
                {
                    string reference = iceHome + dir + component + ".dll";
                    if (File.Exists(reference))
                    {
                        VSLangProj.VSProject vsProject = (VSLangProj.VSProject)project.Object;
                        try
                        {
                            vsProject.References.Add(reference);
                            return;
                        }
                        catch (COMException ex)
                        {
                            Console.WriteLine(ex);
                        }
                    }
                }
            }

            System.Windows.Forms.MessageBox.Show("Could not locate '" + component +
                                                 ".dll'. Review you 'Ice Home' setting.",
                                                 "Ice Visual Studio Extension", MessageBoxButtons.OK,
                                                 MessageBoxIcon.Error,
                                                 System.Windows.Forms.MessageBoxDefaultButton.Button1,
                                                 System.Windows.Forms.MessageBoxOptions.RightAlign);
        }

        public static bool removeDotNetReference(Project project, string component)
        {
            if (project == null || String.IsNullOrEmpty(component))
            {
                return false;
            }

            foreach (Reference r in ((VSProject)project.Object).References)
            {
                if (r.Identity.Equals(component, StringComparison.OrdinalIgnoreCase))
                {
                    r.Remove();
                    return true;
                }
            }
            return false;
        }

        public static void addCppLib(VCLinkerTool tool, string component, bool debug)
        {
            if (tool == null || String.IsNullOrEmpty(component))
            {
                return;
            }

            if (Array.BinarySearch(Util.getCppNames(), component) < 0)
            {
                return;
            }

            string libName = component;
            if (debug)
            {
                libName += "d";
            }
            libName += ".lib";

            string additionalDependencies = tool.AdditionalDependencies;
            if (String.IsNullOrEmpty(additionalDependencies))
            {
                additionalDependencies = "";
            }

            ComponentList components = new ComponentList(additionalDependencies.Split(' '));
            if (!components.Contains(libName))
            {
                components.Add(libName);
                additionalDependencies = components.ToString(' ');
                tool.AdditionalDependencies = additionalDependencies;
            }
        }

        public static bool removeCppLib(VCLinkerTool tool, string component, bool debug)
        {
            if (tool == null || String.IsNullOrEmpty(tool.AdditionalDependencies))
            {
                return false;
            }

            string libName = component;
            if (debug)
            {
                libName += "d";
            }
            libName += ".lib";

            ComponentList components = new ComponentList(tool.AdditionalDependencies.Split(' '));
            if (components.Contains(libName))
            {
                components.Remove(libName);
                tool.AdditionalDependencies = components.ToString(' ');
                return true;
            }
            return false;
        }

        public static string buildPathEnvCmd(string dir)
        {
            return "PATH=" + dir + Path.PathSeparator + "$(PATH)";
        }

        public static void addIceCppEnviroment(VCDebugSettings debugSettings, Project project, string iceHomeRaw, bool x64)
        {
            if (debugSettings == null || project == null)
            {
                return;
            }

            string iceBinDir = iceHomeRaw;
            if (Directory.Exists(Path.Combine(Util.absolutePath(project, iceBinDir), "cpp\\bin")))
            {
                iceBinDir = Path.Combine(iceBinDir, "cpp\\bin");
            }
            else
            {
                iceBinDir = Path.Combine(iceBinDir, "bin");
#if VS2010
                iceBinDir = Path.Combine(iceBinDir, "\\vc100");
#endif
                if (x64)
                {
                    iceBinDir = Path.Combine(iceBinDir, "\\x64");
                }
            }

            string icePathEnvCmd = buildPathEnvCmd(iceBinDir);
            
            string environment = debugSettings.Environment;
            if (String.IsNullOrEmpty(environment))
            {
                debugSettings.Environment = icePathEnvCmd;
                return;
            }

            ComponentList envs = new ComponentList(environment, '\n');
            // TODO: case-sensitive path comparison?
            if (!envs.Contains(icePathEnvCmd))
            {
                envs.Add(icePathEnvCmd);
                debugSettings.Environment = envs.ToString('\n');
                return;
            }

        }

        public static void removeIceCppEnviroment(VCDebugSettings debugSettings, string iceHomeRaw)
        {
            if (debugSettings == null || String.IsNullOrEmpty(debugSettings.Environment))
            {
                return;
            }

            string[] _cppBinDirs =
            {
                "bin",
                "bin\\x64",
                "bin\\vc100",
                "bin\\vc100\\x64",
                "cpp\\bin",
            };

            ComponentList envs = new ComponentList(debugSettings.Environment, '\n');
            foreach (string dir in _cppBinDirs)
            {
                envs.Remove(buildPathEnvCmd(Path.Combine(iceHomeRaw, dir)));
            }
            debugSettings.Environment = envs.ToString('\n');
        }

        public static void addIceCppLibraryDir(VCLinkerTool tool, Project project, string iceHomeRaw, bool x64)
        {
            if (tool == null || project == null)
            {
                return;
            }

            string iceLibDir = iceHomeRaw; 

            if (Directory.Exists(Path.Combine(Util.absolutePath(project, iceLibDir), "cpp\\lib")))
            {
                // Source distribution can only build one target at a time.
                iceLibDir = Path.Combine(iceLibDir, "cpp\\lib");
            }
            else
            {
                iceLibDir += "\\lib";
#if VS2010
                iceLibDir += "\\vc100";
#endif
                if (x64)
                {
                    iceLibDir += "\\x64";
                }
            }
            iceLibDir = quote(iceLibDir);

            string additionalLibraryDirectories = tool.AdditionalLibraryDirectories;
            if (String.IsNullOrEmpty(additionalLibraryDirectories))
            {
                tool.AdditionalLibraryDirectories = iceLibDir;
                return;
            }

            ComponentList libs = new ComponentList(additionalLibraryDirectories);
            if (!libs.Contains(iceLibDir))
            {
                libs.Add(iceLibDir);
                tool.AdditionalLibraryDirectories = libs.ToString();
                return;
            }
        }

        public static void removeIceCppLibraryDir(VCLinkerTool tool, string iceHomeRaw)
        {
            if (tool == null || String.IsNullOrEmpty(tool.AdditionalLibraryDirectories))
            {
                return;
            }

            string[] _cppLibDirs =
            {
                "lib",
                "lib\\x64",
                "lib\\vc100",
                "lib\\vc100\\x64",
                "cpp\\lib",
            };

            ComponentList libs = new ComponentList(tool.AdditionalLibraryDirectories);
            foreach (string dir in _cppLibDirs)
            {
                libs.Remove(quote(Path.Combine(iceHomeRaw, dir)));
                libs.Remove(Path.Combine(iceHomeRaw, dir));
            }
            tool.AdditionalLibraryDirectories = libs.ToString();
        }

        public static bool isSliceFilename(string s)
        {
            return s != null && s.EndsWith(".ice", StringComparison.CurrentCultureIgnoreCase);
        }

        public static bool equalPath(string p1, string p2)
        {
            return 0 == String.Compare(
                Path.GetFullPath(p1).TrimEnd(Path.DirectorySeparatorChar),
                Path.GetFullPath(p2).TrimEnd(Path.DirectorySeparatorChar),
                StringComparison.CurrentCultureIgnoreCase);
        }

        public static bool isSliceBuilderEnabled(Project project)
        {
            return Util.getProjectPropertyAsBool(project, Util.PropertyIce);
        }

        public static bool isCSharpProject(Project project)
        {
            if (project == null)
            {
                return false;
            }

            if (String.IsNullOrEmpty(project.Kind))
            {
                return false;
            }

            return project.Kind == VSLangProj.PrjKind.prjKindCSharpProject;
        }

        public static bool isVBProject(Project project)
        {
            if (project == null)
            {
                return false;
            }

            if (String.IsNullOrEmpty(project.Kind))
            {
                return false;
            }

            return project.Kind == VSLangProj.PrjKind.prjKindVBProject;
        }

        public static bool isSilverlightProject(Project project)
        {
            if (!Util.isCSharpProject(project))
            {
                return false;
            }

            Array extenders = (Array)project.ExtenderNames;
            foreach (string s in extenders)
            {
                if (String.IsNullOrEmpty(s))
                {
                    continue;
                }
                if (s.Equals("SilverlightProject"))
                {
                    return true;
                }
            }
            return false;
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

        public static bool isProjectItemFolder(ProjectItem item)
        {
            if (item == null)
            {
                return false;
            }

            if (String.IsNullOrEmpty(item.Kind))
            {
                return false;
            }
            return item.Kind == "{6BB5F8EF-4483-11D3-8BCF-00C04F8EC28C}";
        }

        public static bool isProjectItemFilter(ProjectItem item)
        {
            if (item == null)
            {
                return false;
            }

            if (String.IsNullOrEmpty(item.Kind))
            {
                return false;
            }
            return item.Kind == "{6BB5F8F0-4483-11D3-8BCF-00C04F8EC28C}";
        }

        public static bool isProjectItemFile(ProjectItem item)
        {
            if (item == null)
            {
                return false;
            }

            if (String.IsNullOrEmpty(item.Kind))
            {
                return false;
            }
            return item.Kind == "{6BB5F8EE-4483-11D3-8BCF-00C04F8EC28C}";
        }

        public static bool hasItemNamed(ProjectItems items, string name)
        {
            bool found = false;
            foreach (ProjectItem item in items)
            {
                if (item == null)
                {
                    continue;
                }

                if (item.Name == null)
                {
                    continue;
                }

                if (item.Name.Equals(name, StringComparison.OrdinalIgnoreCase))
                {
                    found = true;
                    break;
                }
            }
            return found;
        }

        public static ProjectItem findItem(string path, ProjectItems items)
        {
            if (String.IsNullOrEmpty(path))
            {
                return null;
            }
            ProjectItem item = null;
            foreach (ProjectItem i in items)
            {
                if (i == null)
                {
                    continue;
                }
                else if (Util.isProjectItemFile(i))
                {
                    string fullPath = i.Properties.Item("FullPath").Value.ToString();
                    if (Util.equalPath(fullPath, path))
                    {
                        item = i;
                        break;
                    }
                }
                else if (Util.isProjectItemFolder(i))
                {
                    string p = Path.GetDirectoryName(i.Properties.Item("FullPath").Value.ToString());
                    if (Util.equalPath(p, path))
                    {
                        item = i;
                        break;
                    }

                    item = findItem(path, i.ProjectItems);
                    if (item != null)
                    {
                        break;
                    }
                }
                else if (Util.isProjectItemFilter(i))
                {
                    string p = Path.GetFullPath(Path.Combine(Path.GetDirectoryName(i.ContainingProject.FileName),
                                            Util.getPathRelativeToProject(i)));

                    if (Util.equalPath(p, path))
                    {
                        item = i;
                        break;
                    }

                    item = findItem(path, i.ProjectItems);
                    if (item != null)
                    {
                        break;
                    }
                }
            }
            return item;
        }

        public static VCFile findVCFile(IVCCollection files, string name, string fullPath)
        {
            VCFile vcFile = null;
            foreach (VCFile file in files)
            {
                if (file.ItemName == name)
                {
                    if (!Util.equalPath(file.FullPath, fullPath))
                    {
                        file.Remove();
                        break;
                    }
                    vcFile = file;
                    break;
                }
            }
            return vcFile;
        }

        // TODO: what is the point of this?
        public static string normalizePath(string path)
        {
            path = path.Replace('/', '\\');
            path = path.Replace(".\\", "");
            if (path.IndexOf("\\", StringComparison.Ordinal) == 0)
            {
                path = path.Substring("\\".Length, path.Length - "\\".Length);
            }
            if (path.EndsWith("\\.", StringComparison.Ordinal))
            {
                path = path.Substring(0, path.Length - "\\.".Length);
            }
            return path;
        }

        public static string relativePath(Project project, string absoluteFilePath)
        {
            if (project == null || absoluteFilePath == null)
            {
                return "";
            }

            string mainDirPath = Path.GetFullPath(Path.GetDirectoryName(project.FileName)).Trim(Path.DirectorySeparatorChar);
            absoluteFilePath = Path.GetFullPath(absoluteFilePath).Trim(Path.DirectorySeparatorChar);

            string[] firstPathParts = mainDirPath.Split(Path.DirectorySeparatorChar);
            string[] secondPathParts = absoluteFilePath.Split(Path.DirectorySeparatorChar);

            int sameCounter = 0;
            while (sameCounter < Math.Min(firstPathParts.Length, secondPathParts.Length) &&
                String.Equals(firstPathParts[sameCounter], secondPathParts[sameCounter], StringComparison.CurrentCultureIgnoreCase))
            {
                ++sameCounter;
            }

            // Different volumes, relative path not possible.
            if (sameCounter == 0)
            {
                return absoluteFilePath;
            }

            // Pop back up to the common point.
            string newPath = "." + Path.DirectorySeparatorChar;
            for (int i = sameCounter; i < firstPathParts.Length; ++i)
            {
                newPath += ".." + Path.DirectorySeparatorChar;
            }

            // Descend to the target.
            for (int i = sameCounter; i < secondPathParts.Length; ++i)
            {
                newPath += secondPathParts[i] + Path.DirectorySeparatorChar;
            }
            return newPath.TrimEnd(Path.DirectorySeparatorChar);
        }

        // Relative paths are relative to project.
        // Inverse of Util.relativePath().
        public static string absolutePath(Project p, string f)
        {
            return Path.GetFullPath(Path.Combine(Path.GetDirectoryName(p.FileName), f));
        }

        public static ProjectItem getSelectedProjectItem(_DTE dte)
        {
            UIHierarchyItem uiItem = getSelectedUIHierearchyItem(dte);
            if (uiItem == null)
            {
                return null;
            }
            return uiItem.Object as ProjectItem;
        }

        public static Project getSelectedProject()
        {
            return Util.getSelectedProject(Util.getCurrentDTE());
        }

        public static Project getSelectedProject(_DTE dte)
        {
            UIHierarchyItem uiItem = getSelectedUIHierearchyItem(dte);
            if (uiItem == null)
            {
                return null;
            }
            return uiItem.Object as Project;
        }

        public static UIHierarchyItem getSelectedUIHierearchyItem(_DTE dte)
        {
            if (dte == null)
            {
                return null;
            }

            UIHierarchy uiHierarchy =
                (EnvDTE.UIHierarchy)dte.Windows.Item(EnvDTE.Constants.vsWindowKindSolutionExplorer).Object;
            if (uiHierarchy == null)
            {
                return null;
            }

            if (uiHierarchy.SelectedItems == null)
            {
                return null;
            }

            if (((Array)uiHierarchy.SelectedItems).Length <= 0)
            {
                return null;
            }
            return (UIHierarchyItem)((Array)uiHierarchy.SelectedItems).GetValue(0);
        }

        public static void updateIceHome(Project project, string iceHomeRaw, bool force)
        {
            if (project == null || String.IsNullOrEmpty(iceHomeRaw))
            {
                return;
            }

            // TODO: is this optimization required?
            if (!force)
            {
                // TODO: should be path comparison
                if (String.Equals(Util.getIceHomeRaw(project, true), iceHomeRaw, StringComparison.CurrentCultureIgnoreCase))
                {
                    return;
                }
            }

            if (Util.isCSharpProject(project) || Util.isVBProject(project))
            {
                updateIceHomeDotNetProject(project, iceHomeRaw);
            }
            else if (Util.isCppProject(project))
            {
                updateIceHomeCppProject(project, iceHomeRaw);
            }
        }

        private static void updateIceHomeCppProject(Project project, string iceHomeRawNew)
        {
            Util.removeIceCppConfigurations(project, getIceHomeRaw(project, true));
            Util.setIceHome(project, iceHomeRawNew);
            Util.addIceCppConfigurations(project, getIceHomeRaw(project, true));
        }

        private static bool getCopyLocal(Project project, string name)
        {
            VSLangProj.VSProject vsProject = (VSLangProj.VSProject)project.Object;
            foreach (Reference r in vsProject.References)
            {
                if (r.Name.Equals(name))
                {
                    return r.CopyLocal;
                }
            }
            return true;
        }

        private static void setCopyLocal(Project project, string name, bool copyLocal)
        {
            VSLangProj.VSProject vsProject = (VSLangProj.VSProject)project.Object;
            foreach (Reference r in vsProject.References)
            {
                if (r.Name.Equals(name))
                {
                    r.CopyLocal = copyLocal;
                    break;
                }
            }
        }

        private static void updateIceHomeDotNetProject(Project project, string iceHomeRaw)
        {
            Util.setIceHome(project, iceHomeRaw);
            string iceHomeRel = getIceHome(project);

            ComponentList components = Util.getIceDotNetComponents(project);
            foreach (string s in components)
            {
                if (String.IsNullOrEmpty(s))
                {
                    continue;
                }

                bool copyLocal = getCopyLocal(project, s);
                Util.removeDotNetReference(project, s);

                Util.addDotNetReference(project, s, iceHomeRel);
                setCopyLocal(project, s, copyLocal);
            }
        }

        public static void setIceHome(Project project, string value)
        {
            string expanded = subEnvironmentVars(value);
            string fullPath = Util.absolutePath(project, expanded);

            if (Util.isSilverlightProject(project))
            {
                if (!File.Exists(fullPath + "\\bin\\slice2sl.exe") || !Directory.Exists(fullPath + "\\slice\\Ice"))
                {
                    if (!File.Exists(fullPath + "\\cpp\\bin\\slice2sl.exe") ||
                       !Directory.Exists(fullPath + "\\sl\\slice\\Ice"))
                    {
                        System.Windows.Forms.MessageBox.Show("Could not locate Ice for Silverlight installation in '"
                                                             + expanded + "' directory.\n",
                                                             "Ice Visual Studio Extension", MessageBoxButtons.OK,
                                                             MessageBoxIcon.Error,
                                                             System.Windows.Forms.MessageBoxDefaultButton.Button1,
                                                             System.Windows.Forms.MessageBoxOptions.RightAlign);
                        return;
                    }
                }
            }
            else if (Util.isCppProject(project))
            {
                if (!Directory.Exists(fullPath + "\\slice\\Ice") ||
                   (!File.Exists(fullPath + "\\bin\\slice2cpp.exe") &&
                    !File.Exists(fullPath + "\\cpp\\bin\\slice2cpp.exe")))
                {
                    System.Windows.Forms.MessageBox.Show("Could not locate Ice installation in '"
                                                         + expanded + "' directory.\n",
                                                         "Ice Visual Studio Extension", MessageBoxButtons.OK,
                                                         MessageBoxIcon.Error,
                                                         System.Windows.Forms.MessageBoxDefaultButton.Button1,
                                                         System.Windows.Forms.MessageBoxOptions.RightAlign);

                    return;
                }
            }
            else if (Util.isCSharpProject(project))
            {
                if (!Directory.Exists(fullPath + "\\slice\\Ice") ||
                   (!File.Exists(fullPath + "\\bin\\slice2cs.exe") &&
                    !File.Exists(fullPath + "\\cpp\\bin\\slice2cs.exe")))
                {
                    System.Windows.Forms.MessageBox.Show("Could not locate Ice installation in '"
                                                         + expanded + "' directory.\n",
                                                         "Ice Visual Studio Extension", MessageBoxButtons.OK,
                                                         MessageBoxIcon.Error,
                                                         System.Windows.Forms.MessageBoxDefaultButton.Button1,
                                                         System.Windows.Forms.MessageBoxOptions.RightAlign);

                    return;
                }
            }
            else if (Util.isVBProject(project))
            {
                if (!File.Exists(fullPath + "\\bin\\Ice.dll") && !File.Exists(fullPath + "\\cs\\bin\\Ice.dll"))
                {
                    System.Windows.Forms.MessageBox.Show("Could not locate Ice installation in '"
                                                         + expanded + "' directory.\n",
                                                         "Ice Visual Studio Extension", MessageBoxButtons.OK,
                                                         MessageBoxIcon.Error,
                                                         System.Windows.Forms.MessageBoxDefaultButton.Button1,
                                                         System.Windows.Forms.MessageBoxOptions.RightAlign);

                    return;
                }
            }

            setProjectProperty(project, Util.PropertyIceHome, value);
        }

        public static bool getProjectPropertyAsBool(Project project, string name)
        {
            return Util.getProjectProperty(project, name).Equals(
                                        true.ToString(), StringComparison.CurrentCultureIgnoreCase);
        }

        public static string getProjectProperty(Project project, string name)
        {
            return Util.getProjectProperty(project, name, "", true);
        }

        public static string getProjectProperty(Project project, string name, string defaultValue)
        {
            return Util.getProjectProperty(project, name, defaultValue, true);
        }

        public static string getProjectProperty(Project project, string name, string defaultValue, bool update)
        {
            if (project == null || String.IsNullOrEmpty(name))
            {
                return defaultValue;
            }

            if (project.Globals == null)
            {
                return defaultValue;
            }

            if (project.Globals.get_VariableExists(name))
            {
                return project.Globals[name].ToString();
            }

            if (update && !String.IsNullOrEmpty(defaultValue))
            {
                project.Globals[name] = defaultValue;
                if (!project.Globals.get_VariablePersists(name))
                {
                    project.Globals.set_VariablePersists(name, true);
                }
            }
            return defaultValue;
        }

        public static void setProjectProperty(Project project, string name, string value)
        {
            if (project == null || String.IsNullOrEmpty(name))
            {
                return;
            }

            if (project.Globals == null)
            {
                return;
            }

            project.Globals[name] = value;
            if (!project.Globals.get_VariablePersists(name))
            {
                project.Globals.set_VariablePersists(name, true);
            }
        }

        // As the add-in evolves the code changes, but users still have project files
        // created by previous versions. This method is called when projects are opened
        // to account for old project files and to move them to the current requirements.
        public static void fix(Project p)
        {
            // This variable was removed for 3.4.2.
            if (p.Globals.get_VariableExists("ZerocIce_HomeExpanded"))
            {
                p.Globals.set_VariablePersists("ZerocIce_HomeExpanded", false);
            }
        }

        public static String getPrecompileHeader(Project project)
        {
            if (!Util.isCppProject(project))
            {
                return "";
            }
            ConfigurationManager configManager = project.ConfigurationManager;
            Configuration activeConfig = (Configuration)configManager.ActiveConfiguration;

            VCProject vcProject = (VCProject)project.Object;
            IVCCollection configurations = (IVCCollection)vcProject.Configurations;
            String preCompiledHeader = "";
            foreach (VCConfiguration conf in configurations)
            {
                if (conf.Name != (activeConfig.ConfigurationName + "|" + activeConfig.PlatformName))
                {
                    continue;
                }
                VCCLCompilerTool compilerTool =
                    (VCCLCompilerTool)(((IVCCollection)conf.Tools).Item("VCCLCompilerTool"));
                if (compilerTool == null)
                {
                    break;
                }
                if (compilerTool.UsePrecompiledHeader == pchOption.pchCreateUsingSpecific ||
                   compilerTool.UsePrecompiledHeader == pchOption.pchUseUsingSpecific)
                {
                    preCompiledHeader = compilerTool.PrecompiledHeaderThrough;
                }
            }
            return preCompiledHeader;
        }

        public static ComponentList getIceCppComponents(Project project)
        {
            ComponentList components = new ComponentList();
            ConfigurationManager configManager = project.ConfigurationManager;
            Configuration activeConfig = (Configuration)configManager.ActiveConfiguration;

            VCProject vcProject = (VCProject)project.Object;
            IVCCollection configurations = (IVCCollection)vcProject.Configurations;
            foreach (VCConfiguration conf in configurations)
            {
                if (conf.Name != (activeConfig.ConfigurationName + "|" + activeConfig.PlatformName))
                {
                    continue;
                }

                VCCLCompilerTool compilerTool =
                    (VCCLCompilerTool)(((IVCCollection)conf.Tools).Item("VCCLCompilerTool"));
                VCLinkerTool linkerTool = (VCLinkerTool)(((IVCCollection)conf.Tools).Item("VCLinkerTool"));
                if (linkerTool == null || compilerTool == null)
                {
                    break;
                }

                if (String.IsNullOrEmpty(linkerTool.AdditionalDependencies))
                {
                    break;
                }

                bool debug = false;
                if (!String.IsNullOrEmpty(compilerTool.PreprocessorDefinitions))
                {
                    debug = (compilerTool.PreprocessorDefinitions.Contains("DEBUG") &&
                             !compilerTool.PreprocessorDefinitions.Contains("NDEBUG"));
                }

                if (!debug)
                {
                    debug = conf.Name.Contains("Debug");
                }

                List<string> componentNames = new List<string>(linkerTool.AdditionalDependencies.Split(' '));
                foreach (string s in componentNames)
                {
                    if (String.IsNullOrEmpty(s))
                    {
                        continue;
                    }

                    int index = s.LastIndexOf('.');
                    if (index <= 0)
                    {
                        continue;
                    }

                    string libName = s.Substring(0, index);
                    if (debug)
                    {
                        libName = libName.Substring(0, libName.Length - 1);
                    }
                    if (String.IsNullOrEmpty(libName))
                    {
                        continue;
                    }

                    if (Array.BinarySearch(Util.getCppNames(), libName) < 0)
                    {
                        continue;
                    }
                    components.Add(libName.Trim());
                }
            }
            return components;
        }

        public static ComponentList getIceSilverlightComponents(Project project)
        {
            ComponentList components = new ComponentList();
            if (project == null)
            {
                return components;
            }

            VSLangProj.VSProject vsProject = (VSLangProj.VSProject)project.Object;
            foreach (Reference r in vsProject.References)
            {
                if (Array.BinarySearch(Util.getSilverlightNames(), r.Name) < 0)
                {
                    continue;
                }

                components.Add(r.Name);
            }
            return components;
        }

        public static ComponentList getIceDotNetComponents(Project project)
        {
            ComponentList components = new ComponentList();
            if (project == null)
            {
                return components;
            }

            VSLangProj.VSProject vsProject = (VSLangProj.VSProject)project.Object;
            foreach (Reference r in vsProject.References)
            {
                if (Array.BinarySearch(Util.getDotNetNames(), r.Name) < 0)
                {
                    continue;
                }

                components.Add(r.Name);
            }
            return components;
        }

        public static void addIceCppConfigurations(Project project, string iceHomeRaw)
        {
            if (!isCppProject(project))
            {
                return;
            }

            VCProject vcProject = (VCProject)project.Object;
            IVCCollection configurations = (IVCCollection)vcProject.Configurations;
            foreach (VCConfiguration conf in configurations)
            {
                if (conf != null)
                {
                    bool x64 = false;
                    VCPlatform platform = (VCPlatform)conf.Platform;
                    String platformName = platform.Name;
                    if (platformName.Equals("x64", StringComparison.CurrentCultureIgnoreCase) ||
                       platformName.Equals("Itanium", StringComparison.CurrentCultureIgnoreCase))
                    {
                        x64 = true;
                    }
                    VCCLCompilerTool compilerTool =
                        (VCCLCompilerTool)(((IVCCollection)conf.Tools).Item("VCCLCompilerTool"));
                    VCLinkerTool linkerTool = (VCLinkerTool)(((IVCCollection)conf.Tools).Item("VCLinkerTool"));

                    Util.addIceCppEnviroment((VCDebugSettings)conf.DebugSettings, project, iceHomeRaw, x64);
                    Util.addIceCppLibraryDir(linkerTool, project, iceHomeRaw, x64);
                    Util.addCppIncludes(compilerTool, project, iceHomeRaw);
                }
            }
        }

        public static void removeIceCppConfigurations(Project project, string iceHomeRaw)
        {
            if (!isCppProject(project))
            {
                return;
            }

            VCProject vcProject = (VCProject)project.Object;
            IVCCollection configurations = (IVCCollection)vcProject.Configurations;
            foreach (VCConfiguration conf in configurations)
            {
                if (conf != null)
                {
                    VCCLCompilerTool compilerTool =
                        (VCCLCompilerTool)(((IVCCollection)conf.Tools).Item("VCCLCompilerTool"));
                    VCLinkerTool linkerTool = (VCLinkerTool)(((IVCCollection)conf.Tools).Item("VCLinkerTool"));

                    Util.removeIceCppEnviroment((VCDebugSettings)conf.DebugSettings, iceHomeRaw);
                    Util.removeIceCppLibraryDir(linkerTool, iceHomeRaw);
                    Util.removeCppIncludes(compilerTool, iceHomeRaw);
                }
            }
        }

        public static void addIceCppLibs(Project project, ComponentList components)
        {
            if (!isCppProject(project))
            {
                return;
            }

            VCProject vcProject = (VCProject)project.Object;
            IVCCollection configurations = (IVCCollection)vcProject.Configurations;

            foreach (VCConfiguration conf in configurations)
            {
                if (conf != null)
                {
                    VCCLCompilerTool compilerTool =
                        (VCCLCompilerTool)(((IVCCollection)conf.Tools).Item("VCCLCompilerTool"));
                    VCLinkerTool linkerTool = (VCLinkerTool)(((IVCCollection)conf.Tools).Item("VCLinkerTool"));

                    if (compilerTool == null || linkerTool == null)
                    {
                        continue;
                    }

                    bool debug = false;
                    if (!String.IsNullOrEmpty(compilerTool.PreprocessorDefinitions))
                    {
                        debug = (compilerTool.PreprocessorDefinitions.Contains("DEBUG") &&
                                 !compilerTool.PreprocessorDefinitions.Contains("NDEBUG"));
                    }
                    if (!debug)
                    {
                        debug = conf.Name.Contains("Debug");
                    }
                    foreach (string component in components)
                    {
                        if (String.IsNullOrEmpty(component))
                        {
                            continue;
                        }
                        Util.addCppLib(linkerTool, component, debug);
                    }
                }
            }
        }

        public static ComponentList removeIceCppLibs(Project project)
        {
            return Util.removeIceCppLibs(project, new ComponentList(Util.getCppNames()));
        }

        public static ComponentList removeIceCppLibs(Project project, ComponentList components)
        {
            ComponentList removed = new ComponentList();
            if (!isCppProject(project))
            {
                return removed;
            }

            VCProject vcProject = (VCProject)project.Object;
            IVCCollection configurations = (IVCCollection)vcProject.Configurations;

            foreach (VCConfiguration conf in configurations)
            {
                if (conf != null)
                {
                    VCCLCompilerTool compilerTool =
                        (VCCLCompilerTool)(((IVCCollection)conf.Tools).Item("VCCLCompilerTool"));
                    VCLinkerTool linkerTool = (VCLinkerTool)(((IVCCollection)conf.Tools).Item("VCLinkerTool"));

                    if (compilerTool == null || linkerTool == null)
                    {
                        continue;
                    }

                    bool debug = false;
                    if (!String.IsNullOrEmpty(compilerTool.PreprocessorDefinitions))
                    {
                        debug = (compilerTool.PreprocessorDefinitions.Contains("DEBUG") &&
                                 !compilerTool.PreprocessorDefinitions.Contains("NDEBUG"));
                    }
                    if (!debug)
                    {
                        debug = conf.Name.Contains("Debug");
                    }

                    foreach (string s in components)
                    {
                        if (s != null)
                        {
                            if (Util.removeCppLib(linkerTool, s, debug) && !removed.Contains(s))
                            {
                                removed.Add(s);
                            }
                        }
                    }
                }
            }
            return removed;
        }

        public static DTE getCurrentDTE()
        {
            return Connect.getCurrentDTE();
        }

        public static string subEnvironmentVars(string s)
        {
            string result = s;
            int beg = 0;
            int end = 0;
            while (beg < result.Length && (beg = result.IndexOf("$(", beg)) != -1 && (end = result.IndexOf(")", beg)) != -1)
            {
                string variable = result.Substring(beg + "$(".Length, end - (beg + "$(".Length));
                string value = System.Environment.GetEnvironmentVariable(variable);
                if (value == null)
                {
                    // Skip undefined vars.
                    beg += "$(".Length + variable.Length + ")".Length;
                    continue;
                }
                result = result.Replace("$(" + variable + ")", value);
                beg += value.Length;
            }
            return result;
        }

        public static bool containsEnvironmentVars(string s)
        {
            return s.Contains("$(");
        }

        public static string quote(string arg)
        {
            return "\"" + arg + "\"";
        }
    }
}
