// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
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

        public ComponentList(string value)
        {
            init(value, ';');
        }

        public ComponentList(string value, char separator)
        {
            init(value, separator);
        }

        public new void Add(string value)
        {
            value = value.Trim();
            if(String.IsNullOrEmpty(value))
            {
                return;
            }
            if(!base.Contains(value))
            {
                base.Add(value);
            }
        }

        public new bool Contains(string value)
        {
            string found = base.Find(delegate(string s)
                                    {
                                        return s.Equals(value.Trim(), StringComparison.CurrentCultureIgnoreCase);
                                    });
            return found != null;
        }

        public new void Remove(string value)
        {
            value = value.Trim();
            if(Contains(value))
            {
                base.Remove(value);
            }
        }

        private void init(string value, char separator)
        {
            Array items = value.Split(separator);
            foreach(string s in items)
            {
                Add(s);
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
                    sb.Append(separator);
                }
            }
            return sb.ToString();
        }

        public bool Equal(ComponentList other)
        {
            if(this.Count != other.Count)
            {
                return false;
            }
            bool equal = true;
            for(int i = 0; i < this.Count; ++i)
            {
                string val1 = this[i];
                string val2 = other[i];
                if(val1 == null && val2 == null)
                {
                    continue;
                }

                if(val1 == null || val2 == null)
                {
                    equal = false;
                    break;
                }

                if(!val1.Equals(val2, StringComparison.CurrentCultureIgnoreCase))
                {
                    equal = false;
                    break;
                }
            }
            return equal;
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

        public bool Contains(Project project, string value)
        {
            string path = Util.absolutePath(project, value);
            string found = base.Find(delegate(string s)
            {
                string other = Util.absolutePath(project, s);
                return path.Equals(other, StringComparison.CurrentCultureIgnoreCase);
            });
            return found != null;
        }
    }

    public static class Util
    {
        public enum msgLevel{ msgError, msgInfo, msgDebug };

        public const string slice2cs = "slice2cs.exe";
        public const string slice2cpp = "slice2cpp.exe";
        public const string slice2sl = "slice2sl.exe";

        //
        // Property names used to persist project configuration.
        //
        public const string PropertyIce = "ZerocIce_Enabled";
        public const string PropertyIceHome = "ZerocIce_Home";
        public const string PropertyIceOutputDir = "ZerocIce_OutputDir";
        public const string PropertyIceHeaderExt = "ZerocIce_HeaderExt";
        public const string PropertyIceSourceExt = "ZerocIce_SourceExt";
        public const string PropertyIceComponents = "ZerocIce_Components";
        public const string PropertyIceExtraOptions = "ZerocIce_ExtraOptions";
        public const string PropertyIceIncludePath = "ZerocIce_IncludePath";
        public const string PropertyIceStreaming = "ZerocIce_Streaming";
        public const string PropertyIceChecksum = "ZerocIce_Checksum";
        public const string PropertyIceTie = "ZerocIce_Tie";
        public const string PropertyIcePrefix = "ZerocIce_Prefix";
        public const string PropertyIceDllExport = "ZerocIce_DllExport";
        public const string PropertyVerboseLevel = "ZerocIce_VerboseLevel";
        public const string EnvCppBinDir = "ZeroC_VS_CppBinDir";
        //
        // TODO: change to ZeroC_VS_IceHome in 3.4.2
        // In 3.4.1 we need IceHome to support Freeze demos.
        //
        public const string EnvIceHome = "IceHome";

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

        // Silverlight handled separately because it is not part of the normal product install. 
        // It is a separate product.
        const string iceSilverlightHome = "C:\\IceSL-0.3.3";

        private static string getIceHomeDefault()
        {
            string defaultIceHome = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
            if(defaultIceHome.EndsWith("\\bin", StringComparison.CurrentCultureIgnoreCase))
            {
                defaultIceHome = defaultIceHome.Substring(0, defaultIceHome.Length - "\\bin".Length);
                if(defaultIceHome.EndsWith("\\vsaddin", StringComparison.CurrentCultureIgnoreCase))
                {
                    defaultIceHome = defaultIceHome.Substring(0, defaultIceHome.Length - "\\vsaddin".Length);
                }
            }
            return defaultIceHome;
        }

        public static string getIceHomeRaw(Project project, bool update)
        {
            if(Util.isSilverlightProject(project))
            {
                return Util.getProjectProperty(project, Util.PropertyIceHome, iceSilverlightHome, update);
            }

            string iceHome = Util.getProjectProperty(project, Util.PropertyIceHome, "", update);
            if(String.IsNullOrEmpty(iceHome))
            {
                iceHome = getIceHomeDefault();
            }
            return iceHome;
        }

        public static string getProjectOutputDirRaw(Project project)
        {
            return getProjectProperty(project, Util.PropertyIceOutputDir, "", false);
        }

        public static string getProjectOutputDir(Project project)
        {
            String path = getProjectOutputDirRaw(project);
            if(containsEnvironmentVars(path))
            {
                path = expandEnvironmentVars(path);
            }
            return path;
        }
        
        public static string getProjectAbsoluteOutputDir(Project project)
        {
            String path = getProjectOutputDir(project);
            if(!Path.IsPathRooted(path))
            {
                path = Util.absolutePath(project, path);
            }
            return path;
        }

#if VS2010
        public static VCPropertySheet findPropertySheet(IVCCollection propertySheets, string sheetName)
        {
            foreach(VCPropertySheet sheet in propertySheets)
            {
                if(sheet.Name == sheetName)
                {
                    return sheet;
                }
            }
            return null;
        }

        public static void addPropertySheet(Project project, string sheetName)
        {
            VCProject vcProj = (VCProject)project.Object;
            string propSheetFileName = vcProj.ProjectDirectory + sheetName + ".props";
            if(!File.Exists(propSheetFileName))
            {
                StreamWriter sw = new StreamWriter(propSheetFileName);
                sw.WriteLine(@"<?xml version=""1.0"" encoding=""utf-8""?>");
                sw.WriteLine(
                    @"<Project ToolsVersion=""4.0"" xmlns=""http://schemas.microsoft.com/developer/msbuild/2003"">");
                sw.WriteLine(@"<ImportGroup Label=""PropertySheets"" />");
                sw.WriteLine(@"<PropertyGroup Label=""UserMacros"" />");
                sw.WriteLine(@"<PropertyGroup />");
                sw.WriteLine(@"<ItemDefinitionGroup />");
                sw.WriteLine(@"<ItemGroup />");
                sw.WriteLine(@"</Project>");
                sw.Close();
            }

            foreach(VCConfiguration vcConfig in vcProj.Configurations as IVCCollection)
            {
                VCPropertySheet newSheet = findPropertySheet(vcConfig.PropertySheets as IVCCollection, sheetName);
                if(newSheet == null)
                {
                    newSheet = vcConfig.AddPropertySheet(propSheetFileName);
                }
            }
        }

        public static VCUserMacro findUserMacro(IVCCollection macros, string macroName)
        {
            foreach(VCUserMacro macro in macros)
            {
                if(macro.Name == macroName)
                {
                    return macro;
                }
            }
            return null;
        }

        public static void updateIceHomePropertySheet(Project project, string iceHome, string cppBinDir)
        {
            addPropertySheet(project, "ice");

            VCProject vcProj = (VCProject)project.Object;

            foreach(VCConfiguration vcConfig in vcProj.Configurations as IVCCollection)
            {
                VCPropertySheet icePropertySheet = findPropertySheet(vcConfig.PropertySheets as IVCCollection, "ice");

                VCUserMacro iceHomeMacro = findUserMacro(icePropertySheet.UserMacros, EnvIceHome);

                if(iceHomeMacro == null)
                {
                    iceHomeMacro = icePropertySheet.AddUserMacro(EnvIceHome, iceHome);
                    iceHomeMacro.PerformEnvironmentSet = true;
                    icePropertySheet.Save();
                }
                else if(iceHomeMacro.Value != iceHome)
                {
                    iceHomeMacro.Value = iceHome;
                    iceHomeMacro.PerformEnvironmentSet = true;
                    icePropertySheet.Save();
                }

                VCUserMacro cppBinDirMacro = findUserMacro(icePropertySheet.UserMacros, EnvCppBinDir);

                if(cppBinDirMacro == null)
                {
                    cppBinDirMacro = icePropertySheet.AddUserMacro(EnvCppBinDir, cppBinDir);
                    cppBinDirMacro.PerformEnvironmentSet = true;
                    icePropertySheet.Save();
                }
                else if(cppBinDirMacro.Value != cppBinDir)
                {
                    cppBinDirMacro.Value = cppBinDir;
                    cppBinDirMacro.PerformEnvironmentSet = true;
                    icePropertySheet.Save();
                }
            }
        }
#endif

        public static string getIceHome(Project project)
        {
            return expandEnvironmentVars(getIceHomeRaw(project, true));
        }

        public static string getCsBinDir(Project project)
        {
            string binDir = getIceHome(project);
            foreach(string dir in _csBinDirs)
            {
                if(Directory.Exists(binDir + dir))
                {
                    binDir += dir;
                    break;
                }
            }
            return binDir;
        }

        public static string getCsBinDirDefault()
        {
            string binDir = getIceHomeDefault();
            foreach(string dir in _csBinDirs)
            {
                if(Directory.Exists(binDir + dir))
                {
                    binDir += dir;
                    break;
                }
            }
            return binDir;
        }

        public static string getPathRelativeToProject(ProjectItem item)
        {
            StringBuilder path = new StringBuilder();
            if(item != null)
            {
                path.Append(Util.getPathRelativeToProject(item, item.ContainingProject.ProjectItems));
            }
            return path.ToString();
        }

        public static string getPathRelativeToProject(ProjectItem item, ProjectItems items)
        {
            StringBuilder path = new StringBuilder();
            foreach(ProjectItem i in items)
            {
                if(i == item)
                {
                    if(path.Length > 0)
                    {
                        path.Append("\\");
                    }
                    path.Append(i.Name);
                    break;
                }
                else if(Util.isProjectItemFilter(i) || Util.isProjectItemFolder(i))
                {
                    string token = Util.getPathRelativeToProject(item, i.ProjectItems);
                    if(!String.IsNullOrEmpty(token))
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
            if(tool == null || project == null)
            {
                return;
            }

            removeCppIncludes(tool, iceHomeRaw, getProjectOutputDirRaw(project));

            string iceIncludeDir = iceHomeRaw;     
            if(Directory.Exists(Path.Combine(Util.absolutePath(project, expandEnvironmentVars(iceIncludeDir)), "cpp\\include")))
            {
                iceIncludeDir = Path.Combine(iceIncludeDir, "cpp");
            }
            iceIncludeDir = Path.Combine(iceIncludeDir, "include");

            string additionalIncludeDirectories = tool.AdditionalIncludeDirectories;
            ComponentList includes;
            if(String.IsNullOrEmpty(additionalIncludeDirectories))
            {
                includes = new ComponentList();
            }
            else
            {
                includes = new ComponentList(additionalIncludeDirectories);
            }

            bool changed = false;
            if(!includes.Contains(iceIncludeDir) && !includes.Contains(quote(iceIncludeDir)))
            {
                changed = true;
                includes.Add(quote(iceIncludeDir));
            }

            string outputDir = Util.getProjectOutputDirRaw(project);
            if(outputDir.Equals(""))
            {
                outputDir = ".";
            }

            if(!includes.Contains(outputDir) && !includes.Contains(quote(outputDir)))
            {
                changed = true;
                includes.Add(outputDir);
            }

            if(changed)
            {
                tool.AdditionalIncludeDirectories = includes.ToString();
            }
        }

        public static void removeCppIncludes(VCCLCompilerTool tool, string iceHomeRaw, string generatedDir)
        {
            if(tool == null || String.IsNullOrEmpty(tool.AdditionalIncludeDirectories))
            {
                return;
            }

            string[] _cppIncludeDirs =
            {
                "include",
                "cpp\\include",
            };

            ComponentList includes = new ComponentList(tool.AdditionalIncludeDirectories);
            foreach(string dir in _cppIncludeDirs)
            {
                includes.Remove(quote(Path.Combine(iceHomeRaw, dir)));
                includes.Remove(Path.Combine(iceHomeRaw, dir));
            }

            if(!generatedDir.Equals("."))
            {
                includes.Remove(generatedDir);
            }

            tool.AdditionalIncludeDirectories = includes.ToString();
        }

        private static readonly string[] _csBinDirs =
        {
            "\\bin\\",
            "\\cs\\bin\\",
            "\\sl\\bin\\",
        };

        public static bool addDotNetReference(Project project, string component, string iceHomeRel, bool development)
        {
            if(project == null || String.IsNullOrEmpty(component))
            {
                return false;
            }

            string iceHome = Util.absolutePath(project, iceHomeRel);
            foreach(string dir in _csBinDirs)
            {
                string reference = iceHome + dir + component + ".dll";
                if(!File.Exists(reference))
                {
                    continue;
                }
                VSLangProj.VSProject vsProject = (VSLangProj.VSProject)project.Object;
                try
                {
                    Reference r = vsProject.References.Add(reference);
                    if(development)
                    {
                        r.CopyLocal = false;
                    }
                    return true;
                }
                catch(COMException ex)
                {
                    Console.WriteLine(ex);
                }
            }

            MessageBox.Show("Could not locate '" + component +
                            ".dll'. Review your 'Ice Home' setting.",
                            "Ice Visual Studio Add-In", MessageBoxButtons.OK,
                            MessageBoxIcon.Error,
                            MessageBoxDefaultButton.Button1,
                            (MessageBoxOptions)0);
            return false;
        }

        public static bool removeDotNetReference(Project project, string component)
        {
            if(project == null || String.IsNullOrEmpty(component))
            {
                return false;
            }

            foreach(Reference r in ((VSProject)project.Object).References)
            {
                if(r.Identity.Equals(component, StringComparison.OrdinalIgnoreCase))
                {
                    r.Remove();
                    return true;
                }
            }
            return false;
        }

        public static void addCppLib(VCLinkerTool tool, string component, bool debug)
        {
            if(tool == null || String.IsNullOrEmpty(component))
            {
                return;
            }

            if(Array.BinarySearch(Util.getCppNames(), component) < 0)
            {
                return;
            }

            string libName = component;
            if(debug)
            {
                libName += "d";
            }
            libName += ".lib";

            string additionalDependencies = tool.AdditionalDependencies;
            if(String.IsNullOrEmpty(additionalDependencies))
            {
                additionalDependencies = "";
            }

            ComponentList components = new ComponentList(additionalDependencies.Split(' '));
            if(!components.Contains(libName))
            {
                components.Add(libName);
                additionalDependencies = components.ToString(' ');
                tool.AdditionalDependencies = additionalDependencies;
            }
        }

        public static bool removeCppLib(VCLinkerTool tool, string component, bool debug)
        {
            if(tool == null || String.IsNullOrEmpty(tool.AdditionalDependencies))
            {
                return false;
            }

            string libName = component;
            if(debug)
            {
                libName += "d";
            }
            libName += ".lib";

            ComponentList components = new ComponentList(tool.AdditionalDependencies.Split(' '));
            if(components.Contains(libName))
            {
                components.Remove(libName);
                tool.AdditionalDependencies = components.ToString(' ');
                return true;
            }
            return false;
        }

        //
        // Add the Ice bin path to the debug environment.
        //
        // Note: Only the last setting in the environment has effect.
        //
        public static void addIceCppEnvironment(VCDebugSettings debugSettings, Project project, string iceHomeRaw,
                                                bool x64)
        {
            if(debugSettings == null || project == null)
            {
                return;
            }

            removeIceCppEnvironment(debugSettings, iceHomeRaw);

            if(String.IsNullOrEmpty(debugSettings.Environment))
            {
                debugSettings.Environment = "PATH=" + cppBinDir(project, iceHomeRaw, x64);
                return;
            }

            ComponentList envs = new ComponentList(debugSettings.Environment, '\n');

            string path = "";

            //
            // Find the last in the list that begins: "PATH=" accounting for case and whitespace.
            //
            int index = -1;
            for(int i = 0; i < envs.Count; ++i)
            {
                string s = envs[i].Trim();
                if(s.StartsWith("PATH", StringComparison.CurrentCultureIgnoreCase))
                {
                    if(s.Substring("PATH".Length).Trim().StartsWith("=", StringComparison.CurrentCultureIgnoreCase))
                    {
                        path = s;
                        index = i;
                    }
                }
            }

            if(index == -1)
            {
                envs.Add("PATH=" + cppBinDir(project, iceHomeRaw, x64));
            }
            else
            {
                string binDir = cppBinDir(project, iceHomeRaw, x64);
                ComponentList paths = new ComponentList(assignmentValue(path), ';');
                while(paths.Contains(binDir))
                {
                    paths.Remove(binDir);
                }
                path = "PATH=" + binDir + Path.PathSeparator + paths.ToString(Path.PathSeparator);
                path = path.TrimEnd(Path.PathSeparator);
                envs[index] = path;
            }
            debugSettings.Environment = envs.ToString('\n'); 
            return;
        }

        private static string removeFromPath(string path, string dir)
        {
            ComponentList list = new ComponentList(path.Split(Path.PathSeparator));
            while(list.Contains(dir))
            {
                list.Remove(dir);
            }
            return list.ToString(Path.PathSeparator);
        }

        private static string assignmentValue(string expr)
        {
            int i = expr.IndexOf('=');
            if(i < 0)
            {
                return "";
            }
            try
            {
                return expr.Substring(i).Substring(1).Trim();
            }
            catch(ArgumentOutOfRangeException)
            {
                return "";
            }
        }

        private static string prependToPath(string path, string dir)
        {
            path = removeFromPath(path, dir);
            return dir + Path.PathSeparator + path;
        }

        public static string cppBinDir(Project project, string iceHomeRaw, bool x64)
        {
            string cppBinDir = iceHomeRaw;
            if(Directory.Exists(Path.Combine(Util.absolutePath(project, expandEnvironmentVars(cppBinDir)), "cpp\\bin")))
            {
                cppBinDir = Path.Combine(cppBinDir, "cpp\\bin");
            }
            else
            {
                cppBinDir = Path.Combine(cppBinDir, "bin");
#if VS2010
                cppBinDir = Path.Combine(cppBinDir, "vc100");
#endif
                if(x64)
                {
                    cppBinDir = Path.Combine(cppBinDir, "x64");
                }
            }
            return cppBinDir;
        }

        public static void removeIceCppEnvironment(VCDebugSettings debugSettings, string iceHomeRaw)
        {
            if(debugSettings == null || String.IsNullOrEmpty(debugSettings.Environment))
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
            /* Find the last in the list that begins: "PATH=" accounting for case and whitespace. */
            string path = "";
            int index = -1;
            for(int i = 0; i < envs.Count; ++i)
            {
                string s = envs[i];
                if(s.StartsWith("PATH", StringComparison.CurrentCultureIgnoreCase))
                {
                    if(s.Substring("PATH".Length).Trim().StartsWith("=", StringComparison.CurrentCultureIgnoreCase))
                    {
                        path = s;
                        index = i;
                    }
                }
            }

            if(index == -1)
            {
                return;
            }

            foreach(string dir in _cppBinDirs)
            {
                path = "PATH=" + removeFromPath(assignmentValue(path).Trim(), Path.Combine(iceHomeRaw, dir));
            }
 
            if(path.Equals("PATH="))
            {
                envs.RemoveAt(index);
            }
            else
            {
                envs[index] = path;
            }
            debugSettings.Environment = envs.ToString('\n');
            return;
        }

        public static void addIceCppLibraryDir(VCLinkerTool tool, Project project, string iceHomeRaw, bool x64)
        {
            if(tool == null || project == null)
            {
                return;
            }

            removeIceCppLibraryDir(tool, iceHomeRaw);

            string iceLibDir = iceHomeRaw; 

            if(Directory.Exists(Path.Combine(Util.absolutePath(project, expandEnvironmentVars(iceLibDir)), "cpp\\lib")))
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
                if(x64)
                {
                    iceLibDir += "\\x64";
                }
            }

            string additionalLibraryDirectories = tool.AdditionalLibraryDirectories;
            if(String.IsNullOrEmpty(additionalLibraryDirectories))
            {
                tool.AdditionalLibraryDirectories = quote(iceLibDir);
                return;
            }

            ComponentList libs = new ComponentList(additionalLibraryDirectories);
            if(!libs.Contains(iceLibDir) && !libs.Contains(quote(iceLibDir)))
            {
                libs.Add(quote(iceLibDir));
                tool.AdditionalLibraryDirectories = libs.ToString();
                return;
            }
        }

        public static void removeIceCppLibraryDir(VCLinkerTool tool, string iceHomeRaw)
        {
            if(tool == null || String.IsNullOrEmpty(tool.AdditionalLibraryDirectories))
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
            foreach(string dir in _cppLibDirs)
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

        public static bool equalPath(string p1, string p2, string basePath)
        {
            if(p1 == p2)
            {
                return true;
            }

            if(String.IsNullOrEmpty(p1) || String.IsNullOrEmpty(p2))
            {
                return false;
            }

            //
            // Convert both paths to absolute paths if necessary
            //
            if(!Path.IsPathRooted(p1))
            {
                p1 = Path.Combine(basePath, p1);
            }

            if(!Path.IsPathRooted(p2))
            {
                p2 = Path.Combine(basePath, p2);
            }

            try
            {
                //
                // Note that in Windows white space at the beginning or end of a file are ignored.
                // When comparing the filenames "Foo ", " Foo", and "foo", all refer to the same file.
                //
                // We also need to trim / (directory separator) from the end in case it's present.
                // 
                return 0 == String.Compare(
                                Path.GetFullPath(p1).Trim().TrimEnd(Path.DirectorySeparatorChar),
                                Path.GetFullPath(p2).Trim().TrimEnd(Path.DirectorySeparatorChar),
                                StringComparison.CurrentCultureIgnoreCase);
            }
            catch(ArgumentException)
            {
            }
            catch(NotSupportedException)
            {
            }
            catch(System.Security.SecurityException)
            {
            }
            catch(PathTooLongException)
            {
            }
            return false;
        }

        public static bool isSliceBuilderEnabled(Project project)
        {
            return Util.getProjectPropertyAsBool(project, Util.PropertyIce);
        }

        public static bool isCSharpProject(Project project)
        {
            if(project == null)
            {
                return false;
            }

            if(String.IsNullOrEmpty(project.Kind))
            {
                return false;
            }

            return project.Kind == VSLangProj.PrjKind.prjKindCSharpProject;
        }

        public static bool isVBProject(Project project)
        {
            if(project == null)
            {
                return false;
            }

            if(String.IsNullOrEmpty(project.Kind))
            {
                return false;
            }

            return project.Kind == VSLangProj.PrjKind.prjKindVBProject;
        }

        public static bool isSilverlightProject(Project project)
        {
            if(!Util.isCSharpProject(project))
            {
                return false;
            }

            Array extenders = (Array)project.ExtenderNames;
            foreach(string s in extenders)
            {
                if(String.IsNullOrEmpty(s))
                {
                    continue;
                }
                if(s.Equals("SilverlightProject"))
                {
                    return true;
                }
            }
            return false;
        }

        public static bool isCppProject(Project project)
        {
            if(project == null)
            {
                return false;
            }

            if(String.IsNullOrEmpty(project.Kind))
            {
                return false;
            }
            return project.Kind == vcContextGuids.vcContextGuidVCProject;
        }

        public static bool isProjectItemFolder(ProjectItem item)
        {
            if(item == null)
            {
                return false;
            }

            if(String.IsNullOrEmpty(item.Kind))
            {
                return false;
            }
            return item.Kind == "{6BB5F8EF-4483-11D3-8BCF-00C04F8EC28C}";
        }

        public static bool isProjectItemFilter(ProjectItem item)
        {
            if(item == null)
            {
                return false;
            }

            if(String.IsNullOrEmpty(item.Kind))
            {
                return false;
            }
            return item.Kind == "{6BB5F8F0-4483-11D3-8BCF-00C04F8EC28C}";
        }

        public static bool isProjectItemFile(ProjectItem item)
        {
            if(item == null)
            {
                return false;
            }

            if(String.IsNullOrEmpty(item.Kind))
            {
                return false;
            }
            return item.Kind == "{6BB5F8EE-4483-11D3-8BCF-00C04F8EC28C}";
        }

        public static bool hasItemNamed(ProjectItems items, string name)
        {
            bool found = false;
            foreach(ProjectItem item in items)
            {
                if(item == null)
                {
                    continue;
                }

                if(item.Name == null)
                {
                    continue;
                }

                if(item.Name.Equals(name, StringComparison.OrdinalIgnoreCase))
                {
                    found = true;
                    break;
                }
            }
            return found;
        }

        public static ProjectItem findItem(string path)
        {
            Builder builder = Connect.getBuilder();
            DTE dte = builder.getCurrentDTE();
            if(dte == null)
            {
                return null;
            }
            if(dte.Solution == null)
            {
                return null;
            }
            if(dte.Solution.Projects == null)
            {
                return null;
            }

            ProjectItem item = null;
            foreach(Project project in dte.Solution.Projects)
            {
                item = findItem(path, project.ProjectItems);
                if(item != null)
                {
                    break;
                }
            }
            return item;
        }

        public static ProjectItem findItem(string path, ProjectItems items)
        {
            if(String.IsNullOrEmpty(path))
            {
                return null;
            }
            ProjectItem item = null;
            foreach(ProjectItem i in items)
            {
                if(i == null)
                {
                    continue;
                }
                else if(Util.isProjectItemFile(i))
                {
                    string fullPath = i.Properties.Item("FullPath").Value.ToString();
                    Project project = i.ContainingProject;
                    if(Util.equalPath(fullPath, path, Path.GetDirectoryName(project.FileName)))
                    {
                        item = i;
                        break;
                    }
                }
                else if(Util.isProjectItemFolder(i))
                {
                    string p = Path.GetDirectoryName(i.Properties.Item("FullPath").Value.ToString());
                    Project project = i.ContainingProject;
                    if(Util.equalPath(p, path, Path.GetDirectoryName(project.FileName)))
                    {
                        item = i;
                        break;
                    }

                    item = findItem(path, i.ProjectItems);
                    if(item != null)
                    {
                        break;
                    }
                }
                else if(Util.isProjectItemFilter(i))
                {
                    string p = Path.GetFullPath(Path.Combine(Path.GetDirectoryName(i.ContainingProject.FileName),
                                            Util.getPathRelativeToProject(i)));

                    Project project = i.ContainingProject;
                    if(Util.equalPath(p, path, Path.GetDirectoryName(project.FileName)))
                    {
                        item = i;
                        break;
                    }

                    item = findItem(path, i.ProjectItems);
                    if(item != null)
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
            foreach(VCFile file in files)
            {
                if(file.ItemName == name)
                {
                    Project project = (Project)((VCProject) file.project).Object;
                    if(!Util.equalPath(file.FullPath, fullPath, Path.GetDirectoryName(project.FileName)))
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

        public static string relativePath(Project project, string absoluteFilePath)
        {
            if(absoluteFilePath == null)
            {
                return "";
            }

            if(project == null)
            {
                return absoluteFilePath;
            }

            return relativePath(Path.GetDirectoryName(project.FileName), absoluteFilePath);
        }

        public static string relativePath(string mainDirPath, string absoluteFilePath)
        {
            if(absoluteFilePath == null)
            {
                return "";
            }

            if(mainDirPath == null)
            {
                return absoluteFilePath;
            }

            if(!Path.IsPathRooted(absoluteFilePath))
            {
                return absoluteFilePath;
            }

            mainDirPath = Path.GetFullPath(mainDirPath).Trim(Path.DirectorySeparatorChar);
            absoluteFilePath = Path.GetFullPath(absoluteFilePath).Trim(Path.DirectorySeparatorChar);

            string[] firstPathParts = mainDirPath.Split(Path.DirectorySeparatorChar);
            string[] secondPathParts = absoluteFilePath.Split(Path.DirectorySeparatorChar);

            int sameCounter = 0;
            while(sameCounter < Math.Min(firstPathParts.Length, secondPathParts.Length) &&
                String.Equals(firstPathParts[sameCounter], secondPathParts[sameCounter],
                StringComparison.CurrentCultureIgnoreCase))
            {
                ++sameCounter;
            }

            // Different volumes, relative path not possible.
            if(sameCounter == 0)
            {
                return absoluteFilePath;
            }

            // Pop back up to the common point.
            string newPath = "." + Path.DirectorySeparatorChar;
            for(int i = sameCounter; i < firstPathParts.Length; ++i)
            {
                newPath += ".." + Path.DirectorySeparatorChar;
            }

            // Descend to the target.
            for(int i = sameCounter; i < secondPathParts.Length; ++i)
            {
                newPath += secondPathParts[i] + Path.DirectorySeparatorChar;
            }
            return newPath.TrimEnd(Path.DirectorySeparatorChar);
        }

        // Relative paths are relative to project.
        // Inverse of Util.relativePath().
        public static string absolutePath(Project project, string path)
        {
            if(Path.IsPathRooted(path)) // If path is absolute return that path
            {
                return path;
            }
            return Path.GetFullPath(Path.Combine(Path.GetDirectoryName(project.FileName), path)).TrimEnd(
                                                                                         Path.DirectorySeparatorChar);
        }

        public static ProjectItem getSelectedProjectItem(_DTE dte)
        {
            UIHierarchyItem uiItem = getSelectedUIHierearchyItem(dte);
            if(uiItem == null)
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
            if(uiItem == null)
            {
                return null;
            }
            return uiItem.Object as Project;
        }

        public static UIHierarchyItem getSelectedUIHierearchyItem(_DTE dte)
        {
            if(dte == null)
            {
                return null;
            }

            UIHierarchy uiHierarchy =
                (EnvDTE.UIHierarchy)dte.Windows.Item(EnvDTE.Constants.vsWindowKindSolutionExplorer).Object;
            if(uiHierarchy == null)
            {
                return null;
            }

            if(uiHierarchy.SelectedItems == null)
            {
                return null;
            }

            if(((Array)uiHierarchy.SelectedItems).Length <= 0)
            {
                return null;
            }
            return (UIHierarchyItem)((Array)uiHierarchy.SelectedItems).GetValue(0);
        }

        public static bool updateOutputDir(Project project, String outputDir)
        {
            //
            // Note that ouputDir could be empty, which means the project dir will
            // be used as the output directory, and is also the default.
            //
            if(project == null || outputDir == null)
            {
                return false;
            }

            Util.cleanProject(project, true);

            string oldOutputDir = getProjectAbsoluteOutputDir(project);
            
            string projectDir = Path.GetDirectoryName(project.FileName);

            // Remove old output directory if necessary.
            if(!equalPath(oldOutputDir, projectDir, projectDir) && 
               isEmptyDir(oldOutputDir))
            {
                // In C# projects, diretories are project items, but not in C++ projects.
                if(isCSharpProject(project))
                {
                    ProjectItem item = findItem(oldOutputDir, project.ProjectItems);
                    if(item != null)
                    {
                        item.Delete();
                    }
                }
                if(Directory.Exists(oldOutputDir))
                {
                    Directory.Delete(oldOutputDir, true);
                }
            }

            if(isCppProject(project))
            {
                // We must remove old output directory from C++ project include path.
                removeCppIncludes(project);
            }

            Util.setProjectProperty(project, Util.PropertyIceOutputDir, outputDir);

            if(isCppProject(project))
            {
                // We must add the new output directory to C++ project include path.
                addCppIncludes(project);
            }

            return true;
        }

        public static void removeCppIncludes(Project project)
        {
            if(!isCppProject(project))
            {
                return;
            }

            VCProject vcProject = (VCProject)project.Object;
            IVCCollection configurations = (IVCCollection)vcProject.Configurations;
            foreach(VCConfiguration conf in configurations)
            {
                if(conf != null)
                {
                    VCCLCompilerTool compilerTool =
                        (VCCLCompilerTool)(((IVCCollection)conf.Tools).Item("VCCLCompilerTool"));
                    Util.removeCppIncludes(compilerTool, getIceHomeRaw(project, false),
                        Util.getProjectOutputDirRaw(project));
                }
            }
        }

        public static void addCppIncludes(Project project)
        {
            if(!isCppProject(project))
            {
                return;
            }

            VCProject vcProject = (VCProject)project.Object;
            IVCCollection configurations = (IVCCollection)vcProject.Configurations;
            foreach(VCConfiguration conf in configurations)
            {
                if(conf != null)
                {
                    VCCLCompilerTool compilerTool =
                        (VCCLCompilerTool)(((IVCCollection)conf.Tools).Item("VCCLCompilerTool"));
                    Util.addCppIncludes(compilerTool, project, getIceHomeRaw(project, false));
                }
            }
        }

        //
        // Return true if the directory doesn't contain any
        // files. We use this to check if it is safe to delete
        // the output-dir after the user changes the setting.
        //
        public static bool isEmptyDir(String path)
        {
            if(String.IsNullOrEmpty(path))
            {
                return false;
            }

            if(!Directory.Exists(path))
            {
                return false;
            }

            DirectoryInfo dir = new DirectoryInfo(path);
            FileInfo[] files = dir.GetFiles();
            if(files.Length == 0)
            {
                return true;
            }
            bool empty = true;
            foreach(FileInfo f in files)
            {
                if(File.Exists(f.FullName))
                {
                    empty = false;
                    break;
                }
                else if(Directory.Exists(f.FullName))
                {
                    empty = isEmptyDir(f.FullName);
                    if(!empty)
                    {
                        break;
                    }
                }
            }
            return empty;
        }

        public static bool updateIceHome(Project project, string iceHomeRaw, bool force)
        {
            if(project == null || String.IsNullOrEmpty(iceHomeRaw))
            {
                return false;
            }

            //
            // We want to only update Ice Home if it is different
            // from the current setting or force is true.
            //
            // We compare the Ice Home raw value, note that we don't do
            // a path comparison as Ice Home raw could contain unexpanded
            // environment variables in it, so we just do a case insensitive
            // string comparision.
            //
            if(!force && String.Equals(Util.getIceHomeRaw(project, true), iceHomeRaw,
                                       StringComparison.CurrentCultureIgnoreCase))
            {
                return false;
            }

            if(Util.isCSharpProject(project) || Util.isVBProject(project))
            {
                return updateIceHomeDotNetProject(project, iceHomeRaw);
            }
            else if(Util.isCppProject(project))
            {
                return updateIceHomeCppProject(project, iceHomeRaw);
            }
            return false;
        }

        private static bool updateIceHomeCppProject(Project project, string iceHomeRawNew)
        {
            Util.removeIceCppConfigurations(project, getIceHomeRaw(project, true));
            bool success = Util.setIceHome(project, iceHomeRawNew);
            Util.addIceCppConfigurations(project, getIceHomeRaw(project, true));
            return success;
        }

        //
        // The CopyLocal property doesn't work consistently, as sometimes it is set to false
        // when the reference isn't found. This happens when project demos are fisrt 
        // opened, as at this point the reference path has not been fixed to use the
        // correct IceHome value. This method returns the private metadata of a 
        // Reference from the project file; this value doesn't change as does CopyLocal.
        //

#if VS2010
        //
        // This method requires .NET 4. Microsoft.Build.BuildEngine is deprecated 
        // in .NET 4, so this method uses the new API Microsoft.Build.Evaluation.
        //
        private static bool getCopyLocal(Project project, string name)
        {
            Microsoft.Build.Evaluation.ProjectItem referenceItem = null;

            Microsoft.Build.Evaluation.Project p =
                Microsoft.Build.Evaluation.ProjectCollection.GlobalProjectCollection.LoadProject(project.FullName);

            foreach(Microsoft.Build.Evaluation.ProjectItem item in p.Items)
            {
                if(!item.ItemType.Equals("Reference"))
                {
                    continue;
                }

                if(!item.HasMetadata("private"))
                {
                    continue;
                }

                string[] tokens = item.EvaluatedInclude.Split(',');
                if(tokens.Length <= 0)
                {
                    continue;
                }

                if(!tokens[0].Trim().Equals(name, StringComparison.CurrentCultureIgnoreCase))
                {
                    continue;
                }

                referenceItem = item;

                if(referenceItem != null)
                {
                    break;
                }
            }

            if(referenceItem != null)
            {
                return referenceItem.GetMetadata("Private").EvaluatedValue.Equals(true.ToString(), 
                    StringComparison.CurrentCultureIgnoreCase);
            }
            return true;
        }
#elif VS2008
        //
        // This method uses the .NET 3.5 API Microsoft.Build.BuildEngine. This API
        // should not be used with .NET 4 because it has been deprecated.
        //
        private static bool getCopyLocal(Project project, string name)
        {
            Microsoft.Build.BuildEngine.BuildItem referenceItem = null;
            Microsoft.Build.BuildEngine.Project p =
                Microsoft.Build.BuildEngine.Engine.GlobalEngine.GetLoadedProject(project.FileName);

            foreach(Microsoft.Build.BuildEngine.BuildItemGroup itemGroup in p.ItemGroups)
            {
                foreach(Microsoft.Build.BuildEngine.BuildItem item in itemGroup)
                {
                    if(!item.Name.Equals("Reference"))
                    {
                        continue;
                    }

                    string[] tokens = item.Include.Split(',');
                    if(tokens.Length <= 0)
                    {
                        continue;
                    }

                    if(!tokens[0].Trim().Equals(name, StringComparison.CurrentCultureIgnoreCase))
                    {
                        continue;
                    }

                    referenceItem = item;

                    if(referenceItem != null)
                    {
                        break;
                    }
                }
                if (referenceItem != null)
                {
                    break;
                }
            }
            if(referenceItem != null)
            {
                return referenceItem.GetMetadata("Private").Equals(true.ToString(),
                                                    StringComparison.CurrentCultureIgnoreCase);
            }
            return true;
        }
#endif
        private static void setCopyLocal(Project project, string name, bool copyLocal)
        {
            VSLangProj.VSProject vsProject = (VSLangProj.VSProject)project.Object;
            foreach(Reference r in vsProject.References)
            {
                if(r.Name.Equals(name))
                {
                    r.CopyLocal = copyLocal;
                    break;
                }
            }
        }

        private static bool updateIceHomeDotNetProject(Project project, string iceHomeRaw)
        {
            bool success = Util.setIceHome(project, iceHomeRaw);
            string iceHomeRel = getIceHome(project);

            ComponentList components = Util.getIceDotNetComponents(project);
            bool development = developmentMode(project);
            foreach(string s in components)
            {
                if(String.IsNullOrEmpty(s))
                {
                    continue;
                }

                bool copyLocal = getCopyLocal(project, s);
                Util.removeDotNetReference(project, s);

                Util.addDotNetReference(project, s, iceHomeRel, development);
                setCopyLocal(project, s, copyLocal);
            }
            return success;
        }

        //
        // Return true if value is a valid Ice installation for the given project
        // type, otherwise returns false. This method doesn't show an error message 
        // box, if you want the error message box to be displayed
        // use checkIceHomeAndShowWarning instead.
        //
        public static bool checkIceHome(Project project, string value)
        {
            string expanded = expandEnvironmentVars(value);
            string fullPath = Util.absolutePath(project, expanded);
            if(Util.isSilverlightProject(project))
            {
                if(!File.Exists(fullPath + "\\bin\\slice2sl.exe") || !Directory.Exists(fullPath + "\\slice\\Ice"))
                {
                    if(!File.Exists(fullPath + "\\cpp\\bin\\slice2sl.exe") ||
                       !Directory.Exists(fullPath + "\\sl\\slice\\Ice"))
                    {
                        return false;
                    }
                }
            }
            else if(Util.isCppProject(project))
            {
                if(!Directory.Exists(fullPath + "\\slice\\Ice") ||
                   (!File.Exists(fullPath + "\\bin\\slice2cpp.exe") &&
                    !File.Exists(fullPath + "\\cpp\\bin\\slice2cpp.exe")))
                {
                    return false;
                }
            }
            else if(Util.isCSharpProject(project))
            {
                if(!Directory.Exists(fullPath + "\\slice\\Ice") ||
                   (!File.Exists(fullPath + "\\bin\\slice2cs.exe") &&
                    !File.Exists(fullPath + "\\cpp\\bin\\slice2cs.exe")))
                {
                    return false;
                }
            }
            else if(Util.isVBProject(project))
            {
                if(!File.Exists(fullPath + "\\bin\\Ice.dll") && !File.Exists(fullPath + "\\cs\\bin\\Ice.dll"))
                {
                    return false;
                }
            }
            return true;
        }

        //
        // Check if value is a valid value for IceHome property with the
        // current project.
        //
        public static bool checkIceHomeAndShowWarning(Project project, string value)
        {
            string expanded = expandEnvironmentVars(value);
            
            if(!Util.checkIceHome(project, value))
            {
                if(isSilverlightProject(project))
                {
                    MessageBox.Show("Could not locate Ice for Silverlight installation in '"
                                    + expanded + "' directory.\n",
                                    "Ice Visual Studio Add-In", MessageBoxButtons.OK,
                                    MessageBoxIcon.Error,
                                    MessageBoxDefaultButton.Button1,
                                    (MessageBoxOptions)0);
                }
                else
                {
                    MessageBox.Show("Could not locate Ice installation in '"
                                    + expanded + "' directory.\n",
                                    "Ice Visual Studio Add-In", MessageBoxButtons.OK,
                                    MessageBoxIcon.Error,
                                    MessageBoxDefaultButton.Button1,
                                    (MessageBoxOptions)0);
                }
                return false;
            }
            return true;
        }

        public static bool setIceHome(Project project, string value)
        {
            if(!checkIceHomeAndShowWarning(project, value))
            {
                return false;
            }
            
            setProjectProperty(project, Util.PropertyIceHome, value);
            return true;
        }

        public static int getProjectPropertyAsInt(Project project, string name)
        {
            int value = -1;
            try
            {
                value = Convert.ToInt32(Util.getProjectProperty(project, name), CultureInfo.InvariantCulture);
            }
            catch(FormatException)
            {
            }
            catch(OverflowException)
            {
            }
            return value;
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
            if(project == null || String.IsNullOrEmpty(name) || project.Globals == null)
            {
                return defaultValue;
            }

            if(project.Globals.get_VariableExists(name))
            {
                return project.Globals[name].ToString();
            }

            if(update && !String.IsNullOrEmpty(defaultValue))
            {
                project.Globals[name] = defaultValue;
                if(!project.Globals.get_VariablePersists(name))
                {
                    project.Globals.set_VariablePersists(name, true);
                }
            }
            return defaultValue;
        }

        public static void setProjectProperty(Project project, string name, string value)
        {
            if(project == null || String.IsNullOrEmpty(name) || project.Globals == null)
            {
                return;
            }
            if(!project.Globals.get_VariableExists(name) || (string)project.Globals[name] != value)
            {
                project.Globals[name] = value;
                if(!project.Globals.get_VariablePersists(name))
                {
                    project.Globals.set_VariablePersists(name, true);
                }
            }
        }

        //
        // As the add-in evolves the code changes, but users still have project files
        // created by previous versions. This method is called when projects are opened
        // to account for old project files and to move them to the current requirements.
        //
        public static void fix(Project p)
        {
            if(p == null || p.Globals == null)
            {
                return;
            }

            // This variable was removed for 3.4.1.2.
            if(p.Globals.get_VariableExists("ZerocIce_HomeExpanded"))
            {
                p.Globals.set_VariablePersists("ZerocIce_HomeExpanded", false);
            }

            // This feature was made more general for 3.4.1.2.
            if(p.Globals.get_VariableExists("ZerocIce_ConsoleOutput"))
            {
                if(!p.Globals.get_VariableExists(PropertyVerboseLevel))
                {
                    setProjectProperty(p, PropertyVerboseLevel, "0");
                    if(getProjectPropertyAsBool(p, "ZerocIce_ConsoleOutput"))
                    {
                        setProjectProperty(p, PropertyVerboseLevel, "1");
                    }
                }
                p.Globals.set_VariablePersists("ZerocIce_ConsoleOutput", false);
            }
        }

        public static String getPrecompileHeader(Project project)
        {
            if(!Util.isCppProject(project))
            {
                return "";
            }
            ConfigurationManager configManager = project.ConfigurationManager;
            if(configManager == null)
            {
                return "";
            }
            
            Configuration activeConfig;
            try
            {
                activeConfig = (Configuration)configManager.ActiveConfiguration;
            }
            catch(COMException)
            {
                return "";
            }

            if(activeConfig == null)
            {
                return "";
            }

            VCProject vcProject = (VCProject)project.Object;
            IVCCollection configurations = (IVCCollection)vcProject.Configurations;
            String preCompiledHeader = "";
            foreach(VCConfiguration conf in configurations)
            {
                if(conf.Name != (activeConfig.ConfigurationName + "|" + activeConfig.PlatformName))
                {
                    continue;
                }
                VCCLCompilerTool compilerTool =
                    (VCCLCompilerTool)(((IVCCollection)conf.Tools).Item("VCCLCompilerTool"));
                if(compilerTool == null)
                {
                    break;
                }
                if(compilerTool.UsePrecompiledHeader == pchOption.pchCreateUsingSpecific ||
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
            foreach(VCConfiguration conf in configurations)
            {
                if(conf == null)
                {
                    continue;
                }
                if(String.IsNullOrEmpty(conf.Name))
                {
                    continue;
                }
                if(!conf.Name.Equals(activeConfig.ConfigurationName + "|" + activeConfig.PlatformName))
                {
                    continue;
                }

                VCCLCompilerTool compilerTool =
                    (VCCLCompilerTool)(((IVCCollection)conf.Tools).Item("VCCLCompilerTool"));
                VCLinkerTool linkerTool = (VCLinkerTool)(((IVCCollection)conf.Tools).Item("VCLinkerTool"));
                if(linkerTool == null || compilerTool == null)
                {
                    break;
                }

                if(String.IsNullOrEmpty(linkerTool.AdditionalDependencies))
                {
                    break;
                }

                bool debug = false;
                if(!String.IsNullOrEmpty(compilerTool.PreprocessorDefinitions))
                {
                    debug = (compilerTool.PreprocessorDefinitions.Contains("DEBUG") &&
                             !compilerTool.PreprocessorDefinitions.Contains("NDEBUG"));
                }

                if(!debug)
                {
                    debug = conf.Name.Contains("Debug");
                }

                List<string> componentNames = new List<string>(linkerTool.AdditionalDependencies.Split(' '));
                foreach(string s in componentNames)
                {
                    if(String.IsNullOrEmpty(s))
                    {
                        continue;
                    }

                    int index = s.LastIndexOf('.');
                    if(index <= 0)
                    {
                        continue;
                    }

                    string libName = s.Substring(0, index);
                    if(debug)
                    {
                        libName = libName.Substring(0, libName.Length - 1);
                    }
                    if(String.IsNullOrEmpty(libName))
                    {
                        continue;
                    }

                    if(Array.BinarySearch(Util.getCppNames(), libName) < 0)
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
            if(project == null)
            {
                return components;
            }

            VSLangProj.VSProject vsProject = (VSLangProj.VSProject)project.Object;
            foreach(Reference r in vsProject.References)
            {
                if(Array.BinarySearch(Util.getSilverlightNames(), r.Name) < 0)
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
            if(project == null)
            {
                return components;
            }

            VSLangProj.VSProject vsProject = (VSLangProj.VSProject)project.Object;
            foreach(Reference r in vsProject.References)
            {
                if(Array.BinarySearch(Util.getDotNetNames(), r.Name) < 0)
                {
                    continue;
                }

                components.Add(r.Name);
            }
            return components;
        }

        public static void addIceCppConfigurations(Project project, string iceHomeRaw)
        {
            if(!isCppProject(project))
            {
                return;
            }

            VCProject vcProject = (VCProject)project.Object;
            IVCCollection configurations = (IVCCollection)vcProject.Configurations;
            foreach(VCConfiguration conf in configurations)
            {
                if(conf == null)
                {
                    continue;
                }
                bool x64 = false;
                VCPlatform platform = (VCPlatform)conf.Platform;
                String platformName = platform.Name;
                if(platformName.Equals("x64", StringComparison.CurrentCultureIgnoreCase) ||
                   platformName.Equals("Itanium", StringComparison.CurrentCultureIgnoreCase))
                {
                    x64 = true;
                }
                VCCLCompilerTool compilerTool =
                    (VCCLCompilerTool)(((IVCCollection)conf.Tools).Item("VCCLCompilerTool"));
                VCLinkerTool linkerTool = (VCLinkerTool)(((IVCCollection)conf.Tools).Item("VCLinkerTool"));

                Util.addIceCppEnvironment((VCDebugSettings)conf.DebugSettings, project, iceHomeRaw, x64);
                Util.addIceCppLibraryDir(linkerTool, project, iceHomeRaw, x64);
                Util.addCppIncludes(compilerTool, project, iceHomeRaw);
            }
        }

        public static void removeIceCppConfigurations(Project project, string iceHomeRaw)
        {
            if(!isCppProject(project))
            {
                return;
            }

            VCProject vcProject = (VCProject)project.Object;
            IVCCollection configurations = (IVCCollection)vcProject.Configurations;
            foreach(VCConfiguration conf in configurations)
            {
                if(conf != null)
                {
                    VCCLCompilerTool compilerTool =
                        (VCCLCompilerTool)(((IVCCollection)conf.Tools).Item("VCCLCompilerTool"));
                    VCLinkerTool linkerTool = (VCLinkerTool)(((IVCCollection)conf.Tools).Item("VCLinkerTool"));

                    Util.removeIceCppEnvironment((VCDebugSettings)conf.DebugSettings, iceHomeRaw);
                    Util.removeIceCppLibraryDir(linkerTool, iceHomeRaw);
                    Util.removeCppIncludes(compilerTool, iceHomeRaw, 
                        Util.getProjectOutputDirRaw(project));
                }
            }
        }

        public static void addIceCppLibs(Project project, ComponentList components)
        {
            if(!isCppProject(project))
            {
                return;
            }

            VCProject vcProject = (VCProject)project.Object;
            IVCCollection configurations = (IVCCollection)vcProject.Configurations;

            foreach(VCConfiguration conf in configurations)
            {
                if(conf == null)
                {
                    continue;
                }
                VCCLCompilerTool compilerTool =
                    (VCCLCompilerTool)(((IVCCollection)conf.Tools).Item("VCCLCompilerTool"));
                VCLinkerTool linkerTool = (VCLinkerTool)(((IVCCollection)conf.Tools).Item("VCLinkerTool"));

                if(compilerTool == null || linkerTool == null)
                {
                    continue;
                }

                bool debug = false;
                if(!String.IsNullOrEmpty(compilerTool.PreprocessorDefinitions))
                {
                    debug = (compilerTool.PreprocessorDefinitions.Contains("DEBUG") &&
                             !compilerTool.PreprocessorDefinitions.Contains("NDEBUG"));
                }
                if(!debug)
                {
                    debug = conf.Name.IndexOf("Debug", StringComparison.CurrentCultureIgnoreCase) >= 0;
                }

                foreach(string component in components)
                {
                    if(String.IsNullOrEmpty(component))
                    {
                        continue;
                    }
                    Util.addCppLib(linkerTool, component, debug);
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
            if(!isCppProject(project))
            {
                return removed;
            }

            VCProject vcProject = (VCProject)project.Object;
            IVCCollection configurations = (IVCCollection)vcProject.Configurations;

            foreach(VCConfiguration conf in configurations)
            {
                if(conf == null)
                {
                    continue;
                }
                VCCLCompilerTool compilerTool =
                    (VCCLCompilerTool)(((IVCCollection)conf.Tools).Item("VCCLCompilerTool"));
                VCLinkerTool linkerTool = (VCLinkerTool)(((IVCCollection)conf.Tools).Item("VCLinkerTool"));

                if(compilerTool == null || linkerTool == null)
                {
                    continue;
                }

                bool debug = false;
                if(!String.IsNullOrEmpty(compilerTool.PreprocessorDefinitions))
                {
                    debug = (compilerTool.PreprocessorDefinitions.Contains("DEBUG") &&
                             !compilerTool.PreprocessorDefinitions.Contains("NDEBUG"));
                }
                if(!debug)
                {
                    debug = conf.Name.Contains("Debug");
                }

                foreach(string s in components)
                {
                    if(String.IsNullOrEmpty(s))
                    {
                        continue;
                    }

                    if(Util.removeCppLib(linkerTool, s, debug) && !removed.Contains(s))
                    {
                        removed.Add(s);
                    }
                }
            }
            return removed;
        }

        //
        // Return true if the .NET reference that corresponds to the Ice component 
        // is present in the given project.
        //
        public static bool hasDotNetReference(Project project, string component)
        {
            bool found = false;
            foreach(Reference r in ((VSProject)project.Object).References)
            {
                if(r.Name.Equals(component, StringComparison.OrdinalIgnoreCase))
                {
                    found = true;
                    break;
                }
            }
            return found;
        }

        //
        // Return true if at least one of the C++ project configurations contains
        // the library corresponding to the given component.
        //
        public static bool hasIceCppLib(Project project, string component)
        {
            if(!isCppProject(project))
            {
                return false;
            }

            if(Array.BinarySearch(Util.getCppNames(), component) < 0)
            {
                return false;
            }

            VCProject vcProject = (VCProject)project.Object;
            IVCCollection configurations = (IVCCollection)vcProject.Configurations;
            bool found = false;

            foreach(VCConfiguration conf in configurations)
            {
                if(conf == null)
                {
                    continue;
                }

                VCCLCompilerTool compilerTool =
                    (VCCLCompilerTool)(((IVCCollection)conf.Tools).Item("VCCLCompilerTool"));
                VCLinkerTool linkerTool = (VCLinkerTool)(((IVCCollection)conf.Tools).Item("VCLinkerTool"));

                if(compilerTool == null || linkerTool == null)
                {
                    continue;
                }

                bool debug = false;
                if(!String.IsNullOrEmpty(compilerTool.PreprocessorDefinitions))
                {
                    debug = (compilerTool.PreprocessorDefinitions.Contains("DEBUG") &&
                             !compilerTool.PreprocessorDefinitions.Contains("NDEBUG"));
                }
                if(!debug)
                {
                    debug = conf.Name.Contains("Debug");
                }

                string libName = component;
                if(debug)
                {
                    libName += "d";
                }
                libName += ".lib";

                string additionalDependencies = linkerTool.AdditionalDependencies;
                if(String.IsNullOrEmpty(additionalDependencies))
                {
                    continue;
                }

                ComponentList components = new ComponentList(additionalDependencies.Split(' '));
                if(components.Contains(libName))
                {
                    found = true;
                    break;
                }
            }
            return found;
        }

        public static DTE getCurrentDTE()
        {
            return Connect.getCurrentDTE();
        }

        public static string expandEnvironmentVars(string s)
        {
            if(String.IsNullOrEmpty(s))
            {
                return s;
            }
            string result = s;
            int beg = 0;
            int end = 0;
            while(beg < result.Length && 
                  (beg = result.IndexOf("$(", beg, StringComparison.Ordinal)) != -1 &&
                  (end = result.IndexOf(")", beg, StringComparison.Ordinal)) != -1)
            {
                string variable = result.Substring(beg + "$(".Length, end - (beg + "$(".Length));
                string value = System.Environment.GetEnvironmentVariable(variable);
                if(value == null)
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
            if(String.IsNullOrEmpty(s))
            {
                return false;
            }
            return s.Contains("$(");
        }

        public static string quote(string arg)
        {
            if(String.IsNullOrEmpty(arg))
            {
                return "\"\"";
            }
            return "\"" + arg + "\"";
        }

        public static void verifyProjectSettings(Project project)
        {
            if(isCppProject(project))
            {
                addIceCppConfigurations(project, getIceHomeRaw(project, false));
            }
            else
            {
                string iceHome = getIceHome(project);
                string binDir = getCsBinDir(project);

                ComponentList components = Util.getIceDotNetComponents(project);
                foreach(string component in components)
                {
                    if(String.IsNullOrEmpty(component))
                    {
                        continue;
                    }


                    string reference = binDir + component + ".dll";

                    //
                    // If Ice components are not from the current Ice home binary
                    // directory, we update the references to use the new value of
                    // Ice home.
                    //
                    bool development = developmentMode(project);
                    foreach(Reference r in ((VSProject)project.Object).References)
                    {
                        if(r.Name.Equals(component, StringComparison.OrdinalIgnoreCase))
                        {
                            if(!r.Path.Equals(reference, StringComparison.OrdinalIgnoreCase))
                            {
                                bool copyLocal = getCopyLocal(project, component);
                                Util.removeDotNetReference(project, component);

                                Util.addDotNetReference(project, component, iceHome, development);
                                setCopyLocal(project, component, copyLocal);
                            }
                            break;
                        }
                    }
                }
            }
        }

        public static bool addBuilderToProject(Project project, ComponentList components)
        {
            Builder builder = Connect.getBuilder();
            if(builder == null || project == null)
            {
                return false;
            }

            if(Util.isSliceBuilderEnabled(project))
            {
                return false; // Already enabled.
            }

            builder.addBuilderToProject(project, components);
            return true;
        }

        public static bool removeBuilderFromProject(Project project)
        {
            Builder builder = Connect.getBuilder();
            if(builder == null || project == null)
            {
                return false;
            }

            if(!Util.isSliceBuilderEnabled(project))
            {
                return false; // Already disabled.
            }

            builder.removeBuilderFromProject(project);
            return true;
        }

        public static void cleanProject(Project project, bool remove)
        {
            Builder builder = Connect.getBuilder();
            builder.cleanProject(project, remove);
        }

        public static void rebuildProject(Project project)
        {
            Builder builder = Connect.getBuilder();
            builder.cleanProject(project, false);
            builder.buildProject(project, true, vsBuildScope.vsBuildScopeProject);
        }

        public static int getVerboseLevel(Project p)
        {
            int verboseLevel = (int)Util.msgLevel.msgDebug;
            if(p != null)
            {
                try
                {
                    verboseLevel = Int32.Parse(Util.getProjectProperty(p, Util.PropertyVerboseLevel),
                                               CultureInfo.InvariantCulture);
                }
                catch(ArgumentNullException)
                {
                }
                catch(FormatException)
                {
                }
                catch(OverflowException)
                {
                }
            }
            return verboseLevel;
        }

        public static void write(Project p, Util.msgLevel msgLevel, string message)
        {
            Builder builder = Connect.getBuilder();
            if(builder == null)
            {
                return;
            }

            int verboseLevel = getVerboseLevel(p);

            if((int)msgLevel <= verboseLevel)
            {
                if(builder.connectMode() != ext_ConnectMode.ext_cm_CommandLine)
                {
                    OutputWindowPane pane = builder.buildOutput();
                    if(pane == null)
                    {
                        return;
                    }
                    pane.Activate();
                    pane.OutputString(message);
                }
                else
                {
                    System.Console.Write(message);
                }
            }
        }

        //
        // Get the assembly name of a .NET project
        //
        public static string assemblyName(Project project)
        {
            if(project == null)
            {
                return "";
            }
            if(!isCSharpProject(project) && !isVBProject(project))
            {
                return "";
            }

            Property assemblyName = project.Properties.Item("AssemblyName");
            
            if(assemblyName.Value == null)
            {
                return "";
            }
            return assemblyName.Value.ToString();
        }

        //
        // Check if the Visual Studio hosting process is enabled in any of
        // the project configurations.
        //
        public static bool useVSHostingProcess(Project p)
        {
            bool enabled = false;
            foreach(Configuration config in p.ConfigurationManager)
            {
                if(config.Properties == null)
                {
                    continue;
                }
                Property property = config.Properties.Item("UseVSHostingProcess");
                if(property.Value.ToString().Equals(true.ToString(), StringComparison.CurrentCultureIgnoreCase))
                {
                    enabled = true;
                    break;
                }
            }
            return enabled;
        }

        //
        // Enable/Disable the Visual Studio hosting process. We use this
        // to force Visual Studio to restart the hosting process, which is
        // necessary for the process to run with updated environment variables.
        //
        public static void setVsHostingProcess(Project p, bool value)
        {
            foreach(Configuration config in p.ConfigurationManager)
            {
                if(config == null)
                {
                    continue;
                }

                if(config.Properties == null)
                {
                    continue;
                }

                Property property = config.Properties.Item("UseVSHostingProcess");
                if(property == null)
                {
                    continue;
                }

                if(property.Value != null)
                { 
                    if(property.Value.ToString().Equals(value.ToString(), StringComparison.CurrentCultureIgnoreCase))
                    {
                        continue;
                    }
                }
                property.Value = value;
            }
        }

        //
        // Check if the .NET development mode is enabled in the application config that resides in
        // the file <applicationName>.config.exe.
        //
        public static bool developmentMode(Project project)
        {
            string configPath = 
                        Path.Combine(Path.GetDirectoryName(project.FileName), assemblyName(project) + ".exe.config");
            if(!File.Exists(configPath))
            {
                return false;
            }

            System.Xml.XmlDocument dom = new System.Xml.XmlDocument();

            try
            {
                dom.Load(configPath);
                System.Xml.XmlNodeList nodes = dom.SelectNodes("/configuration/runtime/developmentMode");
                if(nodes.Count <= 0)
                {
                    return false;
                }
                System.Xml.XmlNode node = nodes[0];

                System.Xml.XmlAttribute attribute = node.Attributes["developerInstallation"];

                if(attribute == null)
                {
                    return false;
                }

                if(String.IsNullOrEmpty(attribute.Value))
                {
                    return false;
                }
                return attribute.Value.Equals("true", StringComparison.CurrentCultureIgnoreCase);
            }
            catch(System.Xml.XmlException)
            {
                return false; // There was an error parsing the XML
            }
        }

        public static bool parseSlice2slOptions(string args, bool showWarning, ref Options opts)
        {
            try
            {
                opts = null;

                if(String.IsNullOrEmpty(args))
                {
                    return true; //No options to parse
                }

                opts = new Options();
                opts.addOpt("h", "help");
                opts.addOpt("v", "version");
                opts.addOpt("D", "", Options.ArgType.NeedArg, "", Options.RepeatType.Repeat);
                opts.addOpt("U", "", Options.ArgType.NeedArg, "", Options.RepeatType.Repeat);
                opts.addOpt("I", "", Options.ArgType.NeedArg, "", Options.RepeatType.Repeat);
                opts.addOpt("E");
                opts.addOpt("", "output-dir", Options.ArgType.NeedArg);
                opts.addOpt("", "depend");
                opts.addOpt("d", "debug");
                opts.addOpt("", "ice");
                opts.addOpt("", "case-sensitive");

                opts.parse(Options.split(args));

                checkInvalidOptions(opts);
                
                return true;
            }
            catch(BadOptionException ex)
            {
                if(showWarning)
                {
                    showExtraOptionsWarning(ex);
                }
                return false;
            }
        }

        public static bool parseSlice2csOptions(string args, bool showWarning, ref Options opts)
        {
            try
            {
                opts = null;

                if(String.IsNullOrEmpty(args))
                {
                    return true; //No options to parse
                }

                opts = new Options();
                opts.addOpt("h", "help");
                opts.addOpt("v", "version");
                opts.addOpt("D", "", Options.ArgType.NeedArg, "", Options.RepeatType.Repeat);
                opts.addOpt("U", "", Options.ArgType.NeedArg, "", Options.RepeatType.Repeat);
                opts.addOpt("I", "", Options.ArgType.NeedArg, "", Options.RepeatType.Repeat);
                opts.addOpt("E");
                opts.addOpt("", "output-dir", Options.ArgType.NeedArg);
                opts.addOpt("", "tie");
                opts.addOpt("", "impl");
                opts.addOpt("", "impl-tie");
                opts.addOpt("", "depend");
                opts.addOpt("d", "debug");
                opts.addOpt("", "ice");
                opts.addOpt("", "underscore");
                opts.addOpt("", "checksum");
                opts.addOpt("", "stream");

                opts.parse(Options.split(args));

                checkInvalidOptions(opts);

                return true;
            }
            catch(BadOptionException ex)
            {
                if(showWarning)
                {
                    showExtraOptionsWarning(ex);
                }
                return false;
            }
        }

        public static bool parseSlice2cppOptions(string args, bool showWarning, ref Options opts,
                                                 ref string headerExt, ref string sourceExt)
        {
            try
            {
                opts = null;
                headerExt = "";
                sourceExt = "";

                if(String.IsNullOrEmpty(args))
                {
                    return true; //No options to parse
                }

                opts = new Options();
                opts.addOpt("h", "help");
                opts.addOpt("v", "version");
                opts.addOpt("", "header-ext", Options.ArgType.NeedArg, "h");
                opts.addOpt("", "source-ext", Options.ArgType.NeedArg, "cpp");
                opts.addOpt("", "add-header", Options.ArgType.NeedArg, "", Options.RepeatType.Repeat);
                opts.addOpt("D", "", Options.ArgType.NeedArg, "", Options.RepeatType.Repeat);
                opts.addOpt("U", "", Options.ArgType.NeedArg, "", Options.RepeatType.Repeat);
                opts.addOpt("I", "", Options.ArgType.NeedArg, "", Options.RepeatType.Repeat);
                opts.addOpt("E");
                opts.addOpt("", "include-dir", Options.ArgType.NeedArg);
                opts.addOpt("", "output-dir", Options.ArgType.NeedArg);
                opts.addOpt("", "dll-export", Options.ArgType.NeedArg);
                opts.addOpt("", "impl");
                opts.addOpt("", "depend");
                opts.addOpt("d", "debug");
                opts.addOpt("", "ice");
                opts.addOpt("", "underscore");
                opts.addOpt("", "checksum");
                opts.addOpt("", "stream");

                opts.parse(Options.split(args));

                checkInvalidOptions(opts);

                string hExt = opts.optArg("header-ext");
                string cppExt = opts.optArg("source-ext");

                if(Util.containsEnvironmentVars(hExt) ||
                   Util.containsEnvironmentVars(cppExt))
                {
                    throw new BadOptionException(
                                "You cannot use environment variables in `--header-ext' or `--source-ext' options.");
                }
                headerExt = hExt;
                sourceExt = cppExt;

                if(opts.isSet("dll-export"))
                {
                    throw new BadOptionException(
                                        "Use the `DLL Export Symbol' text box below to set the --dll-export option\n" +
                                        "instead of the `Extra Compiler Options' text box.");
                }

                return true;
            }
            catch(BadOptionException ex)
            {
                if(showWarning)
                {
                    showExtraOptionsWarning(ex);
                }
                return false;
            }
        }

        //
        // Check for common options that must not appear in Extra Options textbox.
        //
        private static void checkInvalidOptions(Options opts)
        {
            if(opts.isSet("output-dir"))
            {
                throw new BadOptionException(
                                    "Use the `Output Dir' text box above to set `--output-dir' option\n" +
                                    "instead of the `Extra Compiler Options' text box.");
            }

            if(opts.isSet("depend"))
            {
                throw new BadOptionException("Don't use `--depend' option in `Extra Options' text box.");
            }

            if(opts.isSet("help"))
            {
                throw new BadOptionException("Don't use `--help' or `-h' option in `Extra Options' text box.");
            }

            if(opts.isSet("version"))
            {
                throw new BadOptionException("Don't use `--version' or `-v' option in `Extra Options' text box.");
            }
        }

        private static void showExtraOptionsWarning(BadOptionException ex)
        {
            MessageBox.Show("Extra Options field contains some errors:\n" +
                                                 ex.reason,
                                                 "Ice Visual Studio Add-In", MessageBoxButtons.OK,
                                                 MessageBoxIcon.Error,
                                                 MessageBoxDefaultButton.Button1,
                                                 (MessageBoxOptions)0);
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

        public static string getHeaderExt(Project p)
        {
            String extension = getProjectProperty(p, Util.PropertyIceHeaderExt);
            if(String.IsNullOrEmpty(extension))
            {
                extension = "h";
            }
            return extension;
        }

        public static string getSourceExt(Project p)
        {
            String extension = getProjectProperty(p, Util.PropertyIceSourceExt);
            if(String.IsNullOrEmpty(extension))
            {
                extension = "cpp";
            }
            return extension;
        }

        //
        // Warn the user about unsaved changes. Returns true if the user 
        // wants to discard changes, otherwise returns false.
        //
        public static bool warnUnsavedChanges(IWin32Window owner)
        {
            if(MessageBox.Show(owner, "Are you sure you want to discard all changes?",
                               "Ice Visual Studio Add-In", MessageBoxButtons.YesNo,
                               MessageBoxIcon.Question, MessageBoxDefaultButton.Button1,
                               (MessageBoxOptions)0) == DialogResult.Yes)
            {
                return true;
            }
            return false;
        }

        public static List<ProjectItem> clone(ProjectItems items)
        {
            List<ProjectItem> list = new List<ProjectItem>();
            foreach(ProjectItem i in items)
            {
                list.Add(i);
            }
            return list;
        }
    }
}
