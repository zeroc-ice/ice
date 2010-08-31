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
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.CommandBars;
using Microsoft.VisualStudio.VCProjectEngine;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using System.Resources;
using System.Reflection;
using VSLangProj;
using System.Globalization;
using Microsoft.VisualStudio.OLE.Interop;
using System.Runtime.InteropServices;


namespace Ice.VisualStudio
{
    public class Builder : IDisposable
    {
        protected virtual void Dispose(bool disposing)
        {
            if(disposing)
            {
                _serviceProvider.Dispose();
                _errorListProvider.Dispose();
            }
        }
        
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        public DTE getCurrentDTE()
        {
            return _applicationObject.DTE;
        }

        public void init(DTE2 application, ext_ConnectMode connectMode, AddIn addInInstance)
        {
            _applicationObject = application;
            _addInInstance = addInInstance;
            _connectMode = connectMode;

            //
            // Subscribe to solution events.
            //
            _solutionEvents = application.Events.SolutionEvents;
            _solutionEvents.Opened += new _dispSolutionEvents_OpenedEventHandler(solutionOpened);
            _solutionEvents.AfterClosing += new _dispSolutionEvents_AfterClosingEventHandler(afterClosing);
            _solutionEvents.ProjectAdded += new _dispSolutionEvents_ProjectAddedEventHandler(projectAdded);
            _solutionEvents.ProjectRemoved += new _dispSolutionEvents_ProjectRemovedEventHandler(projectRemoved);
            _solutionEvents.ProjectRenamed += new _dispSolutionEvents_ProjectRenamedEventHandler(projectRenamed);

            _buildEvents = _applicationObject.Events.BuildEvents;
            _buildEvents.OnBuildBegin += new _dispBuildEvents_OnBuildBeginEventHandler(buildBegin);
            _buildEvents.OnBuildDone += new _dispBuildEvents_OnBuildDoneEventHandler(buildDone);
            if (_connectMode != ext_ConnectMode.ext_cm_CommandLine)
            {
                foreach (Command c in _applicationObject.Commands)
                {
                    if (c.Name.Equals("Project.AddNewItem"))
                    {
                        _addNewItemEvent = application.Events.get_CommandEvents(c.Guid, c.ID);
                        _addNewItemEvent.AfterExecute += new _dispCommandEvents_AfterExecuteEventHandler(afterAddNewItem);
                    }
                    else if (c.Name.Equals("Edit.Remove"))
                    {
                        _editRemoveEvent = application.Events.get_CommandEvents(c.Guid, c.ID);
                        _editRemoveEvent.AfterExecute += new _dispCommandEvents_AfterExecuteEventHandler(editDeleteEvent);
                    }
                    else if (c.Name.Equals("Edit.Delete"))
                    {
                        _editDeleteEvent = application.Events.get_CommandEvents(c.Guid, c.ID);
                        _editDeleteEvent.AfterExecute += new _dispCommandEvents_AfterExecuteEventHandler(editDeleteEvent);
                    }
                    else if (c.Name.Equals("Project.AddExistingItem"))
                    {
                        _addExistingItemEvent = application.Events.get_CommandEvents(c.Guid, c.ID);
                        _addExistingItemEvent.AfterExecute +=
                            new _dispCommandEvents_AfterExecuteEventHandler(afterAddExistingItem);
                    }
                }
            }
            
            //
            // Subscribe to active configuration changed.
            //
            _serviceProvider =
                new ServiceProvider((Microsoft.VisualStudio.OLE.Interop.IServiceProvider)_applicationObject.DTE);
            initErrorListProvider();
            if (_connectMode != ext_ConnectMode.ext_cm_CommandLine)
            {
                setupCommandBars();
            }
        }

        void editDeleteEvent(string Guid, int ID, object CustomIn, object CustomOut)
        {
            if(_deletedFile != null)
            {
                Project project = getActiveProject();
                if(project != null)
                {
                    removeDependency(project, _deletedFile);
                    _deletedFile = null;
                    clearErrors(project);
                    buildProject(project, false, vsBuildScope.vsBuildScopeProject);
                }
            }
        }

        public IVsSolution getIVsSolution()
        {
            return (IVsSolution) _serviceProvider.GetService(typeof(IVsSolution));
        }

        public void buildDone(vsBuildScope Scope, vsBuildAction Action)
        {
            _building = false;
        }
        
        public bool isBuilding()
        {
            return _building;
        }
        

        public void afterAddNewItem(string Guid, int ID, object obj, object CustomOut)
        {
            foreach(String path in _deleted)
            {
                if(path == null)
                {
                    continue;
                }
                if(File.Exists(path))
                {
                    File.Delete(path);
                }
            }
            _deleted.Clear();
        }

        public void afterAddExistingItem(string Guid, int ID, object obj, object CustomOut)
        {
            _deleted.Clear();
        }
        
        public void disconnect()
        {
            if(_iceConfigurationCmd != null)
            {
                _iceConfigurationCmd.Delete();
            }
            
            _solutionEvents.Opened -= new _dispSolutionEvents_OpenedEventHandler(solutionOpened);
            _solutionEvents.AfterClosing -= new _dispSolutionEvents_AfterClosingEventHandler(afterClosing);
            _solutionEvents.ProjectAdded -= new _dispSolutionEvents_ProjectAddedEventHandler(projectAdded);
            _solutionEvents.ProjectRemoved -= new _dispSolutionEvents_ProjectRemovedEventHandler(projectRemoved);
            _solutionEvents.ProjectRenamed -= new _dispSolutionEvents_ProjectRenamedEventHandler(projectRenamed);
            _solutionEvents = null;

            _buildEvents.OnBuildBegin -= new _dispBuildEvents_OnBuildBeginEventHandler(buildBegin);
            _buildEvents = null;
            
            if(_dependenciesMap != null)
            {
                _dependenciesMap.Clear();
                _dependenciesMap = null;
            }
            
            _errorCount = 0;
            if(_errors != null)
            {
                _errors.Clear();
                _errors = null;
            }

            if(_fileTracker != null)
            {
                _fileTracker.clear();
                _fileTracker = null;
            }
        }

        private void setupCommandBars()
        {
            _iceConfigurationCmd = null;
            try
            {
                _iceConfigurationCmd =
                    _applicationObject.Commands.Item(_addInInstance.ProgID + ".IceConfiguration", -1);
            }
            catch(ArgumentException)
            {
                object[] contextGUIDS = new object[] { };
                _iceConfigurationCmd = 
                    ((Commands2)_applicationObject.Commands).AddNamedCommand2(_addInInstance, 
                                "IceConfiguration",
                                "Ice Configuration...",
                                "Ice Configuration...",
                                true, -1, ref contextGUIDS,
                                (int)vsCommandStatus.vsCommandStatusSupported +
                                (int)vsCommandStatus.vsCommandStatusEnabled,
                                (int)vsCommandStyle.vsCommandStylePictAndText,
                                vsCommandControlType.vsCommandControlTypeButton);
            }

            if(_iceConfigurationCmd == null)
            {
                System.Windows.Forms.MessageBox.Show("Error initializing Ice Visual Studio Extension.\n" +
                                                     "Cannot create required commands",
                                                     "Ice Visual Studio Extension",
                                                     System.Windows.Forms.MessageBoxButtons.OK,
                                                     System.Windows.Forms.MessageBoxIcon.Error,
                                                     System.Windows.Forms.MessageBoxDefaultButton.Button1,
                                                     System.Windows.Forms.MessageBoxOptions.RightAlign);
                return;
            }

            CommandBar toolsCmdBar = ((CommandBars)_applicationObject.CommandBars)["Tools"];
            _iceConfigurationCmd.AddControl(toolsCmdBar, toolsCmdBar.Controls.Count + 1);

            CommandBar projectCmdBar = projectCommandBar();
            _iceConfigurationCmd.AddControl(projectCmdBar, projectCmdBar.Controls.Count + 1);   
        }

        public void afterClosing()
        {
            clearErrors();
            removeDocumentEvents();
            if(_dependenciesMap != null)
            {
                _dependenciesMap.Clear();
                _dependenciesMap = null;
            }
            
            trackFiles();
        }

        private void trackFiles()
        {
            if(_fileTracker == null)
            {
                return;
            }
            foreach(Project p in _applicationObject.Solution.Projects)
            {
                if(p == null)
                {
                    continue;
                }
                if(!Util.isSliceBuilderEnabled(p))
                {
                    continue;
                }
                _fileTracker.reap(p);
            }
        }

        public void solutionOpened()
        {
            try
            {
                _dependenciesMap = new Dictionary<string, Dictionary<string, List<string>>>();
                _fileTracker = new FileTracker();
                initDocumentEvents();
                foreach(Project p in _applicationObject.Solution.Projects)
                {
                    Util.fix(p);
                    if((Util.isCSharpProject(p) || Util.isVBProject(p) || Util.isCppProject(p)) && 
                        Util.isSliceBuilderEnabled(p))
                    {
                        // Call for side-effect of setting environment variable.
                        Util.getIceHome(p);

                        if (!Util.isVBProject(p))
                        {
                            _dependenciesMap[p.Name] = new Dictionary<string, List<string>>();
                            buildProject(p, true, vsBuildScope.vsBuildScopeSolution);
                        }
                    }
                }
                if(hasErrors())
                {
                    bringErrorsToFront();
                }
            }
            catch(Exception ex)
            {
                writeBuildOutput(ex.ToString() + "\n");
            }
        }

        public void addBuilderToProject(Project project)
        {
            string iceHomeRaw = Util.getIceHomeRaw(project, true);
            // TODO: needed?
            string iceHomeRel = Util.getIceHome(project);

            if(Util.isCppProject(project))
            {
                Util.addIceCppConfigurations(project, iceHomeRaw);
                ComponentList components = 
                    new ComponentList(Util.getProjectProperty(project, Util.PropertyIceComponents));
                if(components.Count == 0)
                {
                    components.Add("Ice");
                    components.Add("IceUtil");
                }
                Util.addIceCppLibs(project, components);
                Util.setProjectProperty(project, Util.PropertyIce, true.ToString());
                buildCppProject(project, true);
            }
            else if(Util.isCSharpProject(project))
            {
                if(Util.isSilverlightProject(project))
                {
                    Util.addDotNetReference(project, "IceSL", iceHomeRel);
                }
                else
                {
                    ComponentList components = 
                        new ComponentList(Util.getProjectProperty(project, Util.PropertyIceComponents));
                    if(components.Count == 0)
                    {
                        components.Add("Ice");
                    }
                    foreach(string component in components)
                    {
                        Util.addDotNetReference(project, component, iceHomeRel);
                    }
                }
                Util.setProjectProperty(project, Util.PropertyIce, true.ToString());
                buildCSharpProject(project, true);
            }
            else if(Util.isVBProject(project))
            {
                ComponentList components = 
                    new ComponentList(Util.getProjectProperty(project, Util.PropertyIceComponents));
                if(components.Count == 0)
                {
                    components.Add("Ice");
                }
                foreach(string component in components)
                {
                    Util.addDotNetReference(project, component, iceHomeRel);
                }
                Util.setProjectProperty(project, Util.PropertyIce, true.ToString());
            }
            if(hasErrors(project))
            {
                bringErrorsToFront();
            }
        }

        public void removeBuilderFromProject(Project project)
        {
            cleanProject(project);
            if(Util.isCppProject(project))
            {
                Util.removeIceCppConfigurations(project, Util.getIceHomeRaw(project, true));
                ComponentList libs = Util.removeIceCppLibs(project);
                Util.setProjectProperty(project, Util.PropertyIceComponents, libs.ToString());
                Util.setProjectProperty(project, Util.PropertyIce, false.ToString());
            }
            else if(Util.isCSharpProject(project))
            {
                if(Util.isSilverlightProject(project))
                {
                    Util.removeDotNetReference(project, "IceSL");
                }
                else
                {
                    ComponentList refs = new ComponentList();
                    foreach(string component in Util.getDotNetNames())
                    {
                        if(Util.removeDotNetReference(project, component))
                        {
                            refs.Add(component);
                        }
                        Util.setProjectProperty(project, Util.PropertyIceComponents, refs.ToString());
                    }
                }
                Util.setProjectProperty(project, Util.PropertyIce, false.ToString());
            }
            else if(Util.isVBProject(project))
            {
                ComponentList refs = new ComponentList();
                foreach(string component in Util.getDotNetNames())
                {
                    if(Util.removeDotNetReference(project, component))
                    {
                        refs.Add(component);
                    }
                    Util.setProjectProperty(project, Util.PropertyIceComponents, refs.ToString());
                }
                Util.setProjectProperty(project, Util.PropertyIce, false.ToString());
            }
        }

        private void documentOpened(Document document)
        {
            if(_fileTracker.hasGeneratedFile(document.ProjectItem.ContainingProject, document.FullName))
            {
                if(!document.ReadOnly)
                {
                    document.ReadOnly = true;
                }
            }
        }

        public void documentSaved(Document document)
        {
            Project project = null;
            try
            {
                project = document.ProjectItem.ContainingProject;
            }
            catch(COMException)
            {
                // Expected when documents are create during project initialization
                // and the ProjectItem is not yet available.
                return;
            }
            if(!Util.isSliceBuilderEnabled(project))
            {
                return;
            }
            if (!Util.isSliceFilename(document.Name))
            {
                return;
            }

            _fileTracker.reap(project);
            clearErrors(project);
            buildProject(project, false, vsBuildScope.vsBuildScopeProject);
        }
        
        public void projectAdded(Project project)
        {
            if(Util.isSliceBuilderEnabled(project))
            {
                updateDependencies(project);
            }
        }

        public void projectRemoved(Project project)
        {
            if(_dependenciesMap.ContainsKey(project.Name))
            {
                _dependenciesMap.Remove(project.Name);
            }
        }

        public void projectRenamed(Project project, string oldName)
        {
            if(_dependenciesMap.ContainsKey(oldName))
            {
                _dependenciesMap.Remove(oldName);
            }
            updateDependencies(project);
        }

        public void cleanProject(Project project)
        {
            if(project == null)
            {
                return;
            }
            if(!Util.isSliceBuilderEnabled(project))
            {
                return;
            }
            clearErrors(project);
            _fileTracker.reap(project);

            if(Util.isCSharpProject(project))
            {
                removeCSharpGeneratedItems(project, project.ProjectItems, false);
            }
            else if(Util.isCppProject(project))
            {
                removeCppGeneratedItems(project.ProjectItems, false);
            }
        }

        public void removeCSharpGeneratedItems(Project project, ProjectItems items, bool remove)
        {
            if(project == null)
            {
                return;
            }
            if(items == null)
            {
                return;
            }

            foreach(ProjectItem i in items)
            {
                if(i == null)
                {
                    continue;
                }

                if(Util.isProjectItemFolder(i))
                {
                    removeCSharpGeneratedItems(project, i.ProjectItems, remove);
                }
                else if(Util.isProjectItemFile(i))
                {
                    removeCSharpGeneratedItems(i, remove);
                }
            }
        }

        public void buildProject(Project project, bool force, vsBuildScope scope)
        {
            buildProject(project, force, null, scope);
        }

        public void buildProject(Project project, bool force, ProjectItem excludeItem, vsBuildScope scope)
        {
            if(project == null)
            {
                return;
            }

            if(!Util.isSliceBuilderEnabled(project))
            {
                return;
            }

            if(vsBuildScope.vsBuildScopeProject == scope)
            {
                BuildDependencies dependencies = _applicationObject.Solution.SolutionBuild.BuildDependencies;
                for(int i = 0; i < dependencies.Count; ++i)
                {
                    BuildDependency dp = dependencies.Item(i + 1);
                    if(dp.Project.Equals(project))
                    {
                        System.Array projects = dp.RequiredProjects as System.Array;
                        foreach(Project p in projects)
                        {
                            buildProject(p, force, vsBuildScope.vsBuildScopeProject);
                        }
                    }
                }
            }

            bool consoleOutput = Util.getProjectPropertyAsBool(project, Util.PropertyConsoleOutput);
            if(consoleOutput)
            {
                writeBuildOutput("------ Slice compilation started: Project: " + project.Name + " ------\n");
            }
            _fileTracker.reap(project);
            if(Util.isCSharpProject(project))
            {
                buildCSharpProject(project, force, excludeItem);
            }
            else if(Util.isCppProject(project))
            {
                buildCppProject(project, force);
            }
            if(consoleOutput)
            {
                if(hasErrors(project))
                {
                    writeBuildOutput("------ Slice compilation failed: Project: " + project.Name + " ------\n");
                }
                else
                {
                    writeBuildOutput("------ Slice compilation succeeded: Project: " + project.Name + " ------\n");
                }
            }
        }

        public bool buildCppProject(Project project, bool force)
        {
            // string argsDepend = getSliceCompilerArgs(project, true);
            // string argsCompile = getSliceCompilerArgs(project, false);
            string sliceCompiler = getSliceCompilerPath(project);
            return buildCppProject(project, project.ProjectItems, sliceCompiler, force);
        }

        public bool buildCppProject(Project project, ProjectItems items, string sliceCompiler, bool force)
        {
            bool success = true;
            foreach(ProjectItem i in items)
            {
                if(i == null)
                {
                    continue;
                }

                if(Util.isProjectItemFilter(i))
                {
                    if(!buildCppProject(project, i.ProjectItems, sliceCompiler, force))
                    {
                        success = false;
                    }
                }
                else if(Util.isProjectItemFile(i))
                {
                    if(!buildCppProjectItem(project, i, sliceCompiler, force))
                    {
                        success = false;
                    }
                }
            }
            return success;
        }

        public bool buildCppProjectItem(Project project, ProjectItem item, string sliceCompiler, bool force)
        {
            if(project == null)
            {
                return true;
            }

            if(item == null)
            {
                return true;
            }

            if(item.Name == null)
            {
                return true;
            }

            if(!Util.isSliceFilename(item.Name))
            {
                return true;
            }

            FileInfo iceFileInfo = new FileInfo(item.Properties.Item("FullPath").Value.ToString());
            FileInfo hFileInfo = new FileInfo(getCppGeneratedFileName(Path.GetDirectoryName(project.FullName),
                                              iceFileInfo.FullName, "h"));
            FileInfo cppFileInfo = new FileInfo(Path.ChangeExtension(hFileInfo.FullName, "cpp"));

            string output = Path.GetDirectoryName(cppFileInfo.FullName);
            return buildCppProjectItem(project, output, iceFileInfo, cppFileInfo, hFileInfo, sliceCompiler, force);
        }

        public bool buildCppProjectItem(Project project, String output, FileSystemInfo ice, FileSystemInfo cpp,
                                        FileSystemInfo h, string sliceCompiler, bool force)
        {
            bool updated = false;
            bool success = false;
            
            if(!h.Exists || !cpp.Exists)
            {
                updated = true;
            }
            else if(Util.findItem(h.FullName, project.ProjectItems) == null || 
                    Util.findItem(cpp.FullName, project.ProjectItems) == null)
            {
                updated = true;
            }
            else if(ice.LastWriteTime > h.LastWriteTime || ice.LastWriteTime > cpp.LastWriteTime)
            {
                if(!Directory.Exists(output))
                {
                    Directory.CreateDirectory(output);
                }
                updated = true;
            }
            else
            {
                //
                // Now check it any of the dependencies has changed.
                //
                if(_dependenciesMap.ContainsKey(project.Name))
                {
                    Dictionary<string, List<string>> dependenciesMap = _dependenciesMap[project.Name];
                    if(dependenciesMap.ContainsKey(ice.FullName))
                    {
                        List<string> fileDependencies = dependenciesMap[ice.FullName];
                        foreach(string name in fileDependencies)
                        {
                            FileInfo dependency =
                                new FileInfo(Util.absolutePath(project, name));
                            if(!dependency.Exists)
                            {
                                updated = true;
                                break;
                            }
                            
                            if(dependency.LastWriteTime > cpp.LastWriteTime ||
                               dependency.LastWriteTime > h.LastWriteTime)
                            {
                                updated = true;
                                break;
                            }
                        }
                    }
                }
            }
            
            if(updated || force)
            {
                if(!Directory.Exists(output))
                {
                    Directory.CreateDirectory(output);
                }

                if(updateDependencies(project, null, ice.FullName, sliceCompiler) && updated)
                {
                    if(runSliceCompiler(project, sliceCompiler, ice.FullName, output))
                    {
                        addCppGeneratedFiles(project, ice, cpp, h);
                        success = true;
                    }
                }
            }
            else
            {
                //
                // Make sure generated files are part of project.
                //
                addCppGeneratedFiles(project, ice, cpp, h);
            }
            return !updated | success;
        }

        public void addCppGeneratedFiles(Project project, FileSystemInfo ice, FileSystemInfo cpp, FileSystemInfo h)
        {
            if(project == null)
            {
                return;
            }

            VCProject vcProject = (VCProject)project.Object;

            if(File.Exists(cpp.FullName))
            {
                _fileTracker.trackFile(project, ice.FullName, h.FullName);
                VCFile file = Util.findVCFile((IVCCollection)vcProject.Files, cpp.Name, cpp.FullName);
                if(file == null)
                {
                    vcProject.AddFile(cpp.FullName);
                }
            }

            if(File.Exists(h.FullName))
            {
                _fileTracker.trackFile(project, ice.FullName, cpp.FullName);
                VCFile file = Util.findVCFile((IVCCollection)vcProject.Files, h.Name, h.FullName);            
                if(file == null)
                {
                    vcProject.AddFile(h.FullName);
                }
            }
        }

        public void buildCSharpProject(Project project, bool force)
        {
            buildCSharpProject(project, force, null);
        }
        
        public void buildCSharpProject(Project project, bool force, ProjectItem excludeItem)
        {
            string projectDir = Path.GetDirectoryName(project.FileName);
            string sliceCompiler = getSliceCompilerPath(project);
            buildCSharpProject(project, projectDir, project.ProjectItems, sliceCompiler, force, excludeItem);
        }

        public void buildCSharpProject(Project project, string projectDir, ProjectItems items, string sliceCompiler, bool force, 
                                       ProjectItem excludeItem)
        {
            foreach(ProjectItem i in items)
            {
                if(i == null || i == excludeItem)
                {
                    continue;
                }

                if(Util.isProjectItemFolder(i))
                {
                    buildCSharpProject(project, projectDir, i.ProjectItems, sliceCompiler, force, excludeItem);
                }
                else if(Util.isProjectItemFile(i))
                {
                    buildCSharpProjectItem(project, i, sliceCompiler, force);
                }
            }
        }

        public static String getCppGeneratedFileName(String projectDir, String fullPath, string extension)
        {
            if(String.IsNullOrEmpty(projectDir) || String.IsNullOrEmpty(fullPath))
            {
                return "";
            }

            if(!Util.isSliceFilename(fullPath))
            {
                return "";
            }

            if(Path.GetFullPath(fullPath).StartsWith(Path.GetFullPath(projectDir),
                                                     StringComparison.CurrentCultureIgnoreCase))
            {
                return Path.ChangeExtension(fullPath, extension);
            }
            // If two projects reference the same file, Studio will put the build outputs in the project
            // folders, not with the file. We do the same. The makes sense because compiler settings may
            // vary project to project. The example for slice is whether you generate C# or C++.
            return Path.ChangeExtension(Path.Combine(projectDir, Path.GetFileName(fullPath)), extension);
        }

        public static string getCSharpGeneratedFileName(Project project, ProjectItem item, string extension)
        {
            if(project == null)
            {
                return "";
            }

            if(item == null)
            {
                return "";
            }

            if(!Util.isSliceFilename(item.Name))
            {
                return "";
            }

            string projectDir = Path.GetDirectoryName(project.FileName);
            string itemRelativePath = Util.getPathRelativeToProject(item);
            if(!String.IsNullOrEmpty(itemRelativePath))
            {
                string generatedDir = Path.GetDirectoryName(itemRelativePath);
                string path = System.IO.Path.Combine(projectDir, generatedDir);
                return System.IO.Path.Combine(path, Path.ChangeExtension(item.Name, extension));
            }
            return "";
        }

        public bool buildCSharpProjectItem(Project project, ProjectItem item, string sliceCompiler, bool force)
        {
            if(project == null)
            {
                return true;
            }

            if(item == null)
            {
                return true;
            }

            if(item.Name == null)
            {
                return true;
            }

            if(!Util.isSliceFilename(item.Name))
            {
                return true;
            }

            FileInfo iceFileInfo = new FileInfo(item.Properties.Item("FullPath").Value.ToString());
            FileInfo generatedFileInfo = new FileInfo(getCSharpGeneratedFileName(project, item, "cs"));
            bool success = false;
            bool updated = false;
            if(!generatedFileInfo.Exists)
            {
                updated = true;
            }
            else if(iceFileInfo.LastWriteTime > generatedFileInfo.LastWriteTime)
            {
                updated = true;
            }
            else
            {
                //
                // Now check it any of the dependencies has changed.
                //
                //
                if(_dependenciesMap.ContainsKey(project.Name))
                {
                    Dictionary<string, List<string>> dependenciesMap = _dependenciesMap[project.Name];
                    if(dependenciesMap.ContainsKey(iceFileInfo.FullName))
                    {
                        List<string> fileDependencies = dependenciesMap[iceFileInfo.FullName];
                        foreach(string name in fileDependencies)
                        {
                            FileInfo dependency =
                                new FileInfo(Util.absolutePath(project, name));
                            if(!dependency.Exists)
                            {
                                updated = true;
                                break;
                            }
    
                            if(dependency.LastWriteTime > generatedFileInfo.LastWriteTime)
                            {
                                updated = true;
                                break;
                            }
                        }
                    }
                }
            }
            if(updated || force)
            {
                if(updateDependencies(project, item, iceFileInfo.FullName, sliceCompiler) && 
                   updated)
                {
                    if(runSliceCompiler(project, sliceCompiler, iceFileInfo.FullName, generatedFileInfo.DirectoryName))
                    {
                        addCSharpGeneratedFiles(project, iceFileInfo, generatedFileInfo);
                        success = true;
                    }
                }
            }
            else
            {
                //
                // Make sure generated files are part of project.
                //
                addCSharpGeneratedFiles(project, iceFileInfo, generatedFileInfo);
            }
            return !updated | success;
        }

        private void addCSharpGeneratedFiles(Project project, FileInfo ice, FileInfo file)
        {
            if(File.Exists(file.FullName))
            {
                _fileTracker.trackFile(project, ice.FullName, file.FullName);

                ProjectItem generatedItem = Util.findItem(file.FullName, project.ProjectItems);
                if(generatedItem == null)
                {
                    project.ProjectItems.AddFromFile(file.FullName);
                }
            }
        }
        
        private static string getSliceCompilerPath(Project project)
        {
            string compiler = Util.slice2cpp;
            if (Util.isCSharpProject(project))
            {
                compiler = Util.slice2cs;
                if (Util.isSilverlightProject(project))
                {
                    compiler = Util.slice2sl;
                }
            }

            string iceHome = Util.absolutePath(project, Util.getIceHome(project)); 
            // "cpp" implies source distribution
            if (Directory.Exists(Path.Combine(iceHome, "cpp")))
            {
                iceHome = Path.Combine(iceHome, "cpp");
            }
            return Path.Combine(Path.Combine(iceHome, "bin"), compiler);
        }
        
        private static string getSliceCompilerVersion(Project project, string sliceCompiler)
        {
            System.Diagnostics.Process process;
            ProcessStartInfo processInfo = new ProcessStartInfo(sliceCompiler, "-v");
            processInfo.CreateNoWindow = true;
            processInfo.UseShellExecute = false;
            processInfo.RedirectStandardError = true;
            processInfo.RedirectStandardOutput = true;
            processInfo.WorkingDirectory = Path.GetDirectoryName(project.FileName);

            process = System.Diagnostics.Process.Start(processInfo);
            process.WaitForExit();
            string version = process.StandardOutput.ReadLine();
            return version;
        }

        private static string getSliceCompilerArgs(Project project, bool depend)
        {
            IncludePathList includes = 
                new IncludePathList(Util.getProjectProperty(project, Util.PropertyIceIncludePath));
            string extraOpts = Util.getProjectProperty(project, Util.PropertyIceExtraOptions).Trim();
            bool tie = Util.getProjectPropertyAsBool(project, Util.PropertyIceTie);
            bool ice = Util.getProjectPropertyAsBool(project, Util.PropertyIcePrefix);
            bool streaming = Util.getProjectPropertyAsBool(project, Util.PropertyIceStreaming);
            bool checksum = Util.getProjectPropertyAsBool(project, Util.PropertyIceChecksum);

            string args = "";

            if(depend)
            {
                args += "--depend ";
            }

            if(Util.isCppProject(project))
            {
                String dllExportSymbol = Util.getProjectProperty(project, Util.PropertyIceDllExport);
                if(!String.IsNullOrEmpty(dllExportSymbol))
                {
                    args += "--dll-export=" + dllExportSymbol + " ";
                }

                String preCompiledHeader = Util.getPrecompileHeader(project);
                if(!String.IsNullOrEmpty(preCompiledHeader))
                {
                    args += "--add-header=" + Util.quote(preCompiledHeader) + " ";
                }
            }

            args += "-I\"" + Util.getIceHome(project) + "\\slice\" "; 

            foreach(string i in includes)
            {
                if(String.IsNullOrEmpty(i))
                {
                    continue;
                }
                String include = Util.subEnvironmentVars(i);
                if(include.EndsWith("\\", StringComparison.Ordinal) &&
                   include.Split(new char[]{'\\'}, StringSplitOptions.RemoveEmptyEntries).Length == 1)
                {
                    include += ".";
                }

                if(include.EndsWith("\\", StringComparison.Ordinal) && 
                   !include.EndsWith("\\\\", StringComparison.Ordinal))
                {
                   include += "\\";
                }
                args += "-I" + Util.quote(include) + " ";
            }

            if(extraOpts.Length != 0)
            {
                args += Util.subEnvironmentVars(extraOpts) + " ";
            }

            if(tie && Util.isCSharpProject(project) && !Util.isSilverlightProject(project))
            {
                args += "--tie ";
            }

            if(ice)
            {
                args += "--ice ";
            }

            if(streaming)
            {
                args += "--stream ";
            }

            if(checksum)
            {
                args += "--checksum ";
            }

            return args;
        }
        
        public bool updateDependencies(Project project)
        {
            return updateDependencies(project, null);
        }
        
        public bool updateDependencies(Project project, ProjectItem excludeItem)
        {
            _dependenciesMap[project.Name] = new Dictionary<string, List<string>>();
            string sliceCompiler = getSliceCompilerPath(project);
            return updateDependencies(project, project.ProjectItems, sliceCompiler, excludeItem);
        }

        public void cleanDependencies(Project project, string file)
        {
            if(project == null || file == null)
            {
                return;
            }
            if(String.IsNullOrEmpty(project.Name))
            {
                return;
            }
            if(!_dependenciesMap.ContainsKey(project.Name))
            {
                return;
            }

            Dictionary<string, List<string>> projectDependencies = _dependenciesMap[project.Name];
            if(!projectDependencies.ContainsKey(file))
            {
                return;
            }
            projectDependencies.Remove(file);
            _dependenciesMap[project.Name] = projectDependencies;
        }

        public bool updateDependencies(Project project, ProjectItems items, string sliceCompiler, ProjectItem excludeItem)
        {
            bool success = true;
            foreach(ProjectItem item in items)
            {
                if(item == null || item == excludeItem)
                {
                    continue;
                }

                if(Util.isProjectItemFolder(item) || Util.isProjectItemFilter(item))
                {
                    if(!updateDependencies(project, item.ProjectItems, sliceCompiler, excludeItem))
                    {
                        success = false;
                    }
                }
                else if(Util.isProjectItemFile(item))
                {
                    if(!Util.isSliceFilename(item.Name))
                    {
                        continue;
                    }

                    string fullPath = item.Properties.Item("FullPath").Value.ToString();
                    if(!updateDependencies(project, item, fullPath, sliceCompiler))
                    {
                        success = false;
                    }
                }
            }
            return success;
        }

        public bool updateDependencies(Project project, ProjectItem item, string file, string sliceCompiler)
        {
            bool consoleOutput = Util.getProjectPropertyAsBool(project, Util.PropertyConsoleOutput);
            ProcessStartInfo processInfo;
            System.Diagnostics.Process process;

            string args = getSliceCompilerArgs(project, true) + " " + Util.quote(file);
            processInfo = new ProcessStartInfo(sliceCompiler, args);
            processInfo.CreateNoWindow = true;
            processInfo.UseShellExecute = false;
            processInfo.RedirectStandardError = true;
            processInfo.RedirectStandardOutput = true;
            processInfo.WorkingDirectory = Path.GetDirectoryName(project.FileName);

            if(!File.Exists(sliceCompiler))
            {
                if (consoleOutput)
                {
                    writeBuildOutput("'" + sliceCompiler + "' not found. Review 'Ice Home' setting.\n");
                }
                addError(project, file, TaskErrorCategory.Error, 0, 0, sliceCompiler +
                                            " not found. Review 'Ice Home' setting.");
                return false;
            }

            if(consoleOutput)
            {
                writeBuildOutput(sliceCompiler + " " + args + "\n");
            }
            
            process = System.Diagnostics.Process.Start(processInfo);
            process.WaitForExit();
            
            if (parseErrors(project, sliceCompiler, file, process.StandardError, consoleOutput))
            {
                bringErrorsToFront();
                process.Close();
                if(Util.isCppProject(project))
                {
                    removeCppGeneratedItems(project, file, false);
                }
                else if(Util.isCSharpProject(project))
                {
                    removeCSharpGeneratedItems(item, false);
                }
                return false;
            }
            
            List<string> dependencies = new List<string>();
            TextReader output = process.StandardOutput;
            
            string line = null;

            if(!_dependenciesMap.ContainsKey(project.Name))
            {
                _dependenciesMap[project.Name] = new Dictionary<string,List<string>>();
            }
            
            Dictionary<string, List<string>> projectDeps = _dependenciesMap[project.Name];
            while((line = output.ReadLine()) != null)
            {
                writeBuildOutput(line + "\n");
                if(!String.IsNullOrEmpty(line))
                {
                    if(line.EndsWith(" \\", StringComparison.Ordinal))
                    {
                        line = line.Substring(0, line.Length - 2);
                    }
                    line = line.Trim();
                    //
                    // Unescape white spaces.
                    //
                    line = line.Replace("\\ ", " ");

                    // TODO: != doing case-sensitive filename comparison?
                    if(line.EndsWith(".ice", StringComparison.Ordinal) &&
                       System.IO.Path.GetFileName(line) != System.IO.Path.GetFileName(file))
                    {
                        line = line.Replace('/', '\\');
                        dependencies.Add(line);
                    }
                }
            }
            projectDeps[file] = dependencies;
            _dependenciesMap[project.Name] = projectDeps;

            process.Close();
            return true;
        }

        public void initDocumentEvents()
        {
            //Csharp project item events.
            _csProjectItemsEvents = 
                (EnvDTE.ProjectItemsEvents)_applicationObject.Events.GetObject("CSharpProjectItemsEvents");
            if(_csProjectItemsEvents != null)
            {
                _csProjectItemsEvents.ItemAdded +=
                    new _dispProjectItemsEvents_ItemAddedEventHandler(csharpItemAdded);
                _csProjectItemsEvents.ItemRemoved +=
                    new _dispProjectItemsEvents_ItemRemovedEventHandler(csharpItemRemoved);
                _csProjectItemsEvents.ItemRenamed +=
                    new _dispProjectItemsEvents_ItemRenamedEventHandler(csharpItemRenamed);
            }

            //Cpp project item events.
            _vcProjectItemsEvents = 
                (VCProjectEngineEvents)_applicationObject.Events.GetObject("VCProjectEngineEventsObject");
            if(_vcProjectItemsEvents != null)
            {
                _vcProjectItemsEvents.ItemAdded +=
                    new _dispVCProjectEngineEvents_ItemAddedEventHandler(cppItemAdded);
                _vcProjectItemsEvents.ItemRemoved +=
                    new _dispVCProjectEngineEvents_ItemRemovedEventHandler(cppItemRemoved);
                _vcProjectItemsEvents.ItemRenamed +=
                    new _dispVCProjectEngineEvents_ItemRenamedEventHandler(cppItemRenamed);
            }

            //Visual Studio document events.
            _docEvents = _applicationObject.Events.get_DocumentEvents(null);
            if(_docEvents != null)
            {
                _docEvents.DocumentSaved += new _dispDocumentEvents_DocumentSavedEventHandler(documentSaved);
                _docEvents.DocumentOpened += new _dispDocumentEvents_DocumentOpenedEventHandler(documentOpened);
            }
        }

        public void removeDocumentEvents()
        {
            //Csharp project item events.
            if(_csProjectItemsEvents != null)
            {
                _csProjectItemsEvents.ItemAdded -= 
                    new _dispProjectItemsEvents_ItemAddedEventHandler(csharpItemAdded);
                _csProjectItemsEvents.ItemRemoved -= 
                    new _dispProjectItemsEvents_ItemRemovedEventHandler(csharpItemRemoved);
                _csProjectItemsEvents.ItemRenamed -=
                    new _dispProjectItemsEvents_ItemRenamedEventHandler(csharpItemRenamed);
                _csProjectItemsEvents = null;
            }

            //Cpp project item events.
            if(_vcProjectItemsEvents != null)
            {
                _vcProjectItemsEvents.ItemAdded -= 
                    new _dispVCProjectEngineEvents_ItemAddedEventHandler(cppItemAdded);
                _vcProjectItemsEvents.ItemRemoved -=
                    new _dispVCProjectEngineEvents_ItemRemovedEventHandler(cppItemRemoved);
                _vcProjectItemsEvents.ItemRenamed -=
                    new _dispVCProjectEngineEvents_ItemRenamedEventHandler(cppItemRenamed);
                _vcProjectItemsEvents = null;
            }

            //Visual Studio document events.
            if(_docEvents != null)
            {
                _docEvents.DocumentSaved -= new _dispDocumentEvents_DocumentSavedEventHandler(documentSaved);
                _docEvents.DocumentOpened -= new _dispDocumentEvents_DocumentOpenedEventHandler(documentOpened);
                _docEvents = null;
            }
        }

        public Project getSelectedProject()
        {
            return Util.getSelectedProject(_applicationObject.DTE);
        }
        
        public Project getActiveProject()
        {
            Array projects = (Array)_applicationObject.ActiveSolutionProjects;
            if(projects == null)
            {
                return null;
            }
            return projects.GetValue(0) as Project;
        }

        private void cppItemRenamed(object obj, object parent, string oldName)
        {
            try
            {
                if(obj == null)
                {
                    return;
                }
                VCFile file = obj as VCFile;
                if(file == null)
                {
                    return;
                }
                if(!Util.isSliceFilename(file.Name))
                {
                    return;
                }
                Array projects = (Array)_applicationObject.ActiveSolutionProjects;
                if(projects == null)
                {
                    return;
                }
                Project project = projects.GetValue(0) as Project;
                if(project == null)
                {
                    return;
                }
                if(!Util.isSliceBuilderEnabled(project))
                {
                    return;
                }
                _fileTracker.reap(project);
                ProjectItem item = Util.findItem(file.FullPath, project.ProjectItems);

                string fullPath = file.FullPath;
                if(Util.isCppProject(project))
                {
                    string cppPath = Path.ChangeExtension(fullPath, ".cpp");
                    string hPath = Path.ChangeExtension(cppPath, ".h");
                    if(File.Exists(cppPath) || Util.hasItemNamed(project.ProjectItems, Path.GetFileName(cppPath)))
                    {
                        System.Windows.Forms.MessageBox.Show("A file named '" + Path.GetFileName(cppPath) + 
                                                             "' already exists.\n" + "If you want to add '" + 
                                                             Path.GetFileName(fullPath) + "' first remove " + " '" + 
                                                             Path.GetFileName(cppPath) + "' and '" +
                                                             Path.GetFileName(hPath) + "' from your project.",
                                                             "Ice Visual Studio Extension",
                                                             System.Windows.Forms.MessageBoxButtons.OK,
                                                             System.Windows.Forms.MessageBoxIcon.Error,
                                                             System.Windows.Forms.MessageBoxDefaultButton.Button1,
                                                             System.Windows.Forms.MessageBoxOptions.RightAlign);
                        item.Name = oldName;
                        return;
                    }

                    if(File.Exists(hPath) || Util.hasItemNamed(project.ProjectItems, Path.GetFileName(hPath)))
                    {
                        System.Windows.Forms.MessageBox.Show("A file named '" + Path.GetFileName(hPath) +
                                                             "' already exists.\n" + "If you want to add '" +
                                                             Path.GetFileName(fullPath) + "' first remove " +
                                                             " '" + Path.GetFileName(cppPath) + "' and '" +
                                                             Path.GetFileName(hPath) + "' from your project.",
                                                             "Ice Visual Studio Extension",
                                                             System.Windows.Forms.MessageBoxButtons.OK,
                                                             System.Windows.Forms.MessageBoxIcon.Error,
                                                             System.Windows.Forms.MessageBoxDefaultButton.Button1,
                                                             System.Windows.Forms.MessageBoxOptions.RightAlign);
                        item.Name = oldName;
                        return;
                    }
                }

                // Do a full build on a rename
                clearErrors(project);
                buildProject(project, false, vsBuildScope.vsBuildScopeProject);
            }
            catch(Exception ex)
            {
                writeBuildOutput(ex.ToString() + "\n");
            }
        }
        
        private void removeDependency(Project project, String path)
        {
            if(_dependenciesMap.ContainsKey(project.Name))
            {
                // TODO: case-sensitive filename comparison?
                if(_dependenciesMap[project.Name].ContainsKey(path))
                {
                    _dependenciesMap[project.Name].Remove(path);
                }
            }
        }
        
        private void cppItemRemoved(object obj, object parent)
        {
            try
            {
                if(obj == null)
                {
                    return;
                }

                VCFile file = obj as VCFile;
                if(file == null)
                {
                    return;
                }

                Array projects = (Array)_applicationObject.ActiveSolutionProjects;
                if(projects == null)
                {
                    return;
                }

                if(projects.Length <= 0)
                {
                    return;
                }
                Project project = projects.GetValue(0) as Project;
                if(project == null)
                {
                    return;
                }
                if(!Util.isSliceBuilderEnabled(project))
                {
                    return;
                }
                if(!Util.isSliceFilename(file.Name))
                {
                    _fileTracker.reap(project);
                    return;
                }
                clearErrors(file.FullPath);
                removeCppGeneratedItems(project, file.FullPath, true);

                //
                // It appears that file is not actually removed from disk at this
                // point. Thus we need to delay dependency update until after delete,
                // or after remove command has been executed.
                //
                _deletedFile = file.FullPath;
            }
            catch(Exception ex)
            {
                writeBuildOutput(ex.ToString() + "\n");
            }
        }

        void cppItemAdded(object obj, object parent)
        {
            try
            {
                if(obj == null)
                {
                    return;
                }
                VCFile file = obj as VCFile;
                if(file == null)
                {
                    return;
                }
                if(!Util.isSliceFilename(file.Name))
                {
                    return;
                }

                string fullPath = file.FullPath;
                Array projects = (Array)_applicationObject.ActiveSolutionProjects;
                if(projects == null)
                {
                    return;
                }
                if(projects.Length <= 0)
                {
                    return;
                }
                Project project = projects.GetValue(0) as Project;
                if(project == null)
                {
                    return;
                }
                if(!Util.isSliceBuilderEnabled(project))
                {
                    return;
                }
                ProjectItem item = Util.findItem(fullPath, project.ProjectItems);
                if(item == null)
                {
                    return;
                }
                if(Util.isCppProject(project))
                {
                    string cppPath = 
                        getCppGeneratedFileName(Path.GetDirectoryName(project.FullName), file.FullPath, "cpp");
                    string hPath = Path.ChangeExtension(cppPath, ".h");
                    if(File.Exists(cppPath) || Util.hasItemNamed(project.ProjectItems, Path.GetFileName(cppPath)))
                    {
                        System.Windows.Forms.MessageBox.Show("A file named '" + Path.GetFileName(cppPath) +
                                                             "' already exists.\n" + "If you want to add '" +
                                                             Path.GetFileName(fullPath) + "' first remove " +
                                                             " '" + Path.GetFileName(cppPath) + "' and '" +
                                                             Path.GetFileName(hPath) + "'.",
                                                             "Ice Visual Studio Extension",
                                                             System.Windows.Forms.MessageBoxButtons.OK,
                                                             System.Windows.Forms.MessageBoxIcon.Error,
                                                             System.Windows.Forms.MessageBoxDefaultButton.Button1,
                                                             System.Windows.Forms.MessageBoxOptions.RightAlign);
                        _deleted.Add(fullPath);
                        item.Remove();
                        return;
                    }

                    if(File.Exists(hPath) || Util.hasItemNamed(project.ProjectItems, Path.GetFileName(hPath)))
                    {
                        System.Windows.Forms.MessageBox.Show("A file named '" + Path.GetFileName(hPath) +
                                                             "' already exists.\n" + "If you want to add '" +
                                                             Path.GetFileName(fullPath) + "' first remove " +
                                                             " '" + Path.GetFileName(cppPath) + "' and '" +
                                                             Path.GetFileName(hPath) + "'.",
                                                             "Ice Visual Studio Extension",
                                                             System.Windows.Forms.MessageBoxButtons.OK,
                                                             System.Windows.Forms.MessageBoxIcon.Error,
                                                             System.Windows.Forms.MessageBoxDefaultButton.Button1,
                                                             System.Windows.Forms.MessageBoxOptions.RightAlign);
                        _deleted.Add(fullPath);
                        item.Remove();
                        return;
                    }
                }

                clearErrors(project);
                buildProject(project, false, vsBuildScope.vsBuildScopeProject);
            }
            catch(Exception ex)
            {
                writeBuildOutput(ex.ToString() + "\n");
            }
        }

        private void csharpItemRenamed(ProjectItem item, string oldName)
        {
            try
            {
                if(item == null || _fileTracker == null || String.IsNullOrEmpty(oldName) || 
                   item.ContainingProject == null)
                {
                    return;
                }
                if(!Util.isSliceBuilderEnabled(item.ContainingProject))
                {
                    return;
                }
                if(!Util.isSliceFilename(oldName) || !Util.isProjectItemFile(item))
                {
                    return;
                }

                //Get rid of generated files, for the .ice removed file.
                _fileTracker.reap(item.ContainingProject);

                string fullPath = item.Properties.Item("FullPath").Value.ToString();
                if(Util.isCSharpProject(item.ContainingProject))
                {
                    string csPath = Path.ChangeExtension(fullPath, ".cs");
                    if(File.Exists(csPath) || 
                       Util.hasItemNamed(item.ContainingProject.ProjectItems, Path.GetFileName(csPath)))
                    {
                        System.Windows.Forms.MessageBox.Show("A file named '" + Path.GetFileName(csPath) +
                                                             "' already exists.\n" + oldName +
                                                             " could not be renamed to '" + item.Name + "'.",
                                                             "Ice Visual Studio Extension",
                                                             System.Windows.Forms.MessageBoxButtons.OK,
                                                             System.Windows.Forms.MessageBoxIcon.Error,
                                                             System.Windows.Forms.MessageBoxDefaultButton.Button1,
                                                             System.Windows.Forms.MessageBoxOptions.RightAlign);
                        item.Name = oldName;
                        return;
                    }
                }

                clearErrors(item.ContainingProject);
                buildProject(item.ContainingProject, false, vsBuildScope.vsBuildScopeProject);
            }
            catch(Exception ex)
            {
                writeBuildOutput(ex.ToString() + "\n");
            }
        }

        private void csharpItemRemoved(ProjectItem item)
        {
            try
            {
                if(item == null || _fileTracker == null)
                {
                    return;
                }
                if(String.IsNullOrEmpty(item.Name) ||  item.ContainingProject == null)
                {
                    return;
                }
                if(!Util.isSliceBuilderEnabled(item.ContainingProject))
                {
                    return;
                }
                if(!Util.isSliceFilename(item.Name))
                {
                    return;
                }

                string fullName = item.Properties.Item("FullPath").Value.ToString();
                clearErrors(fullName);
                removeCSharpGeneratedItems(item, true);
                _fileTracker.reap(item.ContainingProject);

                removeDependency(item.ContainingProject, fullName);
                clearErrors(item.ContainingProject);
                buildProject(item.ContainingProject, false, item, vsBuildScope.vsBuildScopeProject);
            }
            catch(Exception ex)
            {
                writeBuildOutput(ex.ToString() + "\n");
            }
        }

        private void csharpItemAdded(ProjectItem item)
        {
            try
            {
                if(item == null)
                {
                    return;
                }

                if(String.IsNullOrEmpty(item.Name) || item.ContainingProject == null)
                {
                    return;
                }

                if(!Util.isSliceBuilderEnabled(item.ContainingProject))
                {
                    return;
                }

                if(!Util.isSliceFilename(item.Name))
                {
                    return;
                }

                string fullPath = item.Properties.Item("FullPath").Value.ToString();
                Project project = item.ContainingProject;
                if(project == null)
                {
                    return;
                }

                String csPath = getCSharpGeneratedFileName(project, item, "cs");
                ProjectItem csItem = Util.findItem(csPath, project.ProjectItems);

                if(File.Exists(csPath) || csItem != null)
                {
                    System.Windows.Forms.MessageBox.Show("A file named '" + Path.GetFileName(csPath) +
                                                         "' already exists.\n" + "If you want to add '" +
                                                         Path.GetFileName(fullPath) + "' first remove " +
                                                         " '" + Path.GetFileName(csPath) + "'.",
                                                         "Ice Visual Studio Extension",
                                                         System.Windows.Forms.MessageBoxButtons.OK,
                                                         System.Windows.Forms.MessageBoxIcon.Error,
                                                         System.Windows.Forms.MessageBoxDefaultButton.Button1,
                                                         System.Windows.Forms.MessageBoxOptions.RightAlign);
                    _deleted.Add(fullPath);
                    item.Remove();
                    return;
                }

                clearErrors(project);
                buildProject(project, false, vsBuildScope.vsBuildScopeProject);
            }
            catch(Exception ex)
            {
                writeBuildOutput(ex.ToString() + "\n");
            }
        }

        private static void removeCSharpGeneratedItems(ProjectItem item, bool remove)
        {
            if(item == null)
            {
                return;
            }

            if(item.Name == null)
            {
                return;
            }

            if(!Util.isSliceFilename(item.Name))
            {
                return;
            }

            String generatedPath = getCSharpGeneratedFileName(item.ContainingProject, item, "cs");
            if(!String.IsNullOrEmpty(generatedPath))
            {
                FileInfo generatedFileInfo = new FileInfo(generatedPath);
                if(File.Exists(generatedFileInfo.FullName))
                {
                    File.Delete(generatedFileInfo.FullName);
                }

                if(remove)
                {
                    ProjectItem generated =
                        Util.findItem(generatedFileInfo.FullName, item.ContainingProject.ProjectItems);
                    if(generated != null)
                    {
                        generated.Remove();
                    }
                }
            }
        }

        private static void removeCppGeneratedItems(ProjectItems items, bool remove)
        {
            foreach(ProjectItem i in items)
            {
                if(Util.isProjectItemFile(i))
                {
                    string path = i.Properties.Item("FullPath").Value.ToString();
                    if(!String.IsNullOrEmpty(path))
                    {
                        if(Util.isSliceFilename(path))
                        {
                            removeCppGeneratedItems(i, remove);
                        }
                    }
                }
                else if(Util.isProjectItemFilter(i))
                {
                    removeCppGeneratedItems(i.ProjectItems, remove);
                }
            }
        }

        private static void removeCppGeneratedItems(ProjectItem item, bool remove)
        {
            if(item == null)
            {
                return;
            }

            if(item.Name == null)
            {
                return;
            }

            if(!Util.isSliceFilename(item.Name))
            {
                return;
            }
            removeCppGeneratedItems(item.ContainingProject, item.Properties.Item("FullPath").Value.ToString(), remove);
        }

        // Delete from disk, remove from project if remove=true
        public static void deleteProjectItem(Project project, string file, bool remove)
        {
            if (remove)
            {
                ProjectItem generated = Util.findItem(file, project.ProjectItems);
                if (generated != null)
                {
                    generated.Remove();
                }
            }
            if (File.Exists(file))
            {
                File.Delete(file);
            }
        }

        public static void removeCppGeneratedItems(Project project, String slice, bool remove)
        {
            String projectDir = Path.GetDirectoryName(project.FileName);
            FileInfo hFileInfo = new FileInfo(getCppGeneratedFileName(projectDir, slice, "h"));
            FileInfo cppFileInfo = new FileInfo(Path.ChangeExtension(hFileInfo.FullName, "cpp"));

            deleteProjectItem(project, hFileInfo.FullName, remove);
            deleteProjectItem(project, cppFileInfo.FullName, remove);
        }

        private bool runSliceCompiler(Project project, string sliceCompiler, string file, string outputDir)
        {
            bool consoleOutput = Util.getProjectPropertyAsBool(project, Util.PropertyConsoleOutput);
            string args = getSliceCompilerArgs(project, false);

            if(!String.IsNullOrEmpty(outputDir))
            {
                if(outputDir.EndsWith("\\", StringComparison.Ordinal))
                {
                    outputDir = outputDir.Replace("\\", "\\\\");
                }
                args += " --output-dir " + Util.quote(outputDir) + " ";
            }

            args += " " + Util.quote(file);
            ProcessStartInfo processInfo = new ProcessStartInfo(sliceCompiler, args);
            processInfo.CreateNoWindow = true;
            processInfo.UseShellExecute = false;
            processInfo.RedirectStandardOutput = true;
            processInfo.RedirectStandardError = true;
            processInfo.WorkingDirectory = System.IO.Path.GetDirectoryName(project.FileName);
      
            if(!File.Exists(sliceCompiler))
            {
                if (consoleOutput)
                {
                    writeBuildOutput("'" + sliceCompiler + "' not found. Review 'Ice Home' setting.\n");
                }
                addError(project, file, TaskErrorCategory.Error, 0, 0, sliceCompiler +
                                            " not found. Review 'Ice Home' setting.");
                return false;
            }

            if(consoleOutput)
            {
                writeBuildOutput(sliceCompiler + " " + args + "\n");
            }
            System.Diagnostics.Process process = System.Diagnostics.Process.Start(processInfo);

            process.WaitForExit();

            bool standardError = true;
            if(Util.isSilverlightProject(project))
            {
                string version = getSliceCompilerVersion(project, sliceCompiler);
                List<String> tokens = new List<string>(version.Split(new char[]{'.'}, 
                                                                     StringSplitOptions.RemoveEmptyEntries));
                                                                     
                int mayor = Int32.Parse(tokens[0], CultureInfo.InvariantCulture);
                int minor = Int32.Parse(tokens[1], CultureInfo.InvariantCulture);
                if(mayor == 0 && minor <= 3)
                {
                    standardError = false;
                }
            }

            bool hasErrors = parseErrors(project, sliceCompiler, file, process.StandardError, consoleOutput);
            if(!standardError)
            {
                hasErrors = hasErrors || parseErrors(project, sliceCompiler, file, process.StandardOutput, consoleOutput);
            }
            process.Close();
            if(hasErrors)
            {
                bringErrorsToFront();
                if(Util.isCppProject(project))
                {
                    removeCppGeneratedItems(project, file, false);
                }
                else if(Util.isCSharpProject(project))
                {
                    ProjectItem item = Util.findItem(file, project.ProjectItems);
                    if(item != null)
                    {
                        removeCSharpGeneratedItems(item, false);
                    }
                }
            }
            return !hasErrors;
        }

        private bool parseErrors(Project project, string sliceCompiler, string file, TextReader strer, bool consoleOutput)
        {
            bool hasErrors = false;
            string errorMessage = strer.ReadLine();
            bool firstLine = true;

            while(!String.IsNullOrEmpty(errorMessage))
            {
                if(errorMessage.StartsWith(sliceCompiler, StringComparison.Ordinal))
                {
                    hasErrors = true;
                    String message = strer.ReadLine();
                    while(!String.IsNullOrEmpty(message))
                    {
                        message = message.Trim();
                        if(message.StartsWith("Usage:", StringComparison.CurrentCultureIgnoreCase))
                        {
                            break;
                        }
                        errorMessage += "\n" + message;
                        message = strer.ReadLine();
                    }
                    if(consoleOutput)
                    {
                        writeBuildOutput(errorMessage + "\n");
                    }
                    addError(project, file, TaskErrorCategory.Error, 0, 0, errorMessage.Replace("error:", ""));
                    break;
                }
                int i = errorMessage.IndexOf(':');
                if(i == -1)
                {
                    if(firstLine)
                    {
                        errorMessage += strer.ReadToEnd();
                        if(consoleOutput)
                        {
                            writeBuildOutput(errorMessage + "\n");
                        }
                        addError(project, "", TaskErrorCategory.Error, 1, 1, errorMessage);
                        hasErrors = true;
                        break;
                    }
                    errorMessage = strer.ReadLine();
                    continue;
                }
                if(consoleOutput)
                {
                    writeBuildOutput(errorMessage + "\n");
                }

                if(errorMessage.StartsWith("    ", StringComparison.Ordinal)) // Still the same mcpp warning
                {
                    errorMessage = strer.ReadLine();
                    continue;
                }
                errorMessage = errorMessage.Trim();
                firstLine = false;
                i = errorMessage.IndexOf(':', i + 1);
                if(i == -1)
                {
                    errorMessage = strer.ReadLine();
                    continue;
                }
                string f = errorMessage.Substring(0, i);
                if(String.IsNullOrEmpty(f))
                {
                    errorMessage = strer.ReadLine();
                    continue;
                }

                if(!File.Exists(f))
                {
                    errorMessage = strer.ReadLine();
                    continue;
                }

                errorMessage = errorMessage.Substring(i + 1, errorMessage.Length - i - 1);
                i = errorMessage.IndexOf(':');
                string n = errorMessage.Substring(0, i);
                int l;
                try
                {
                    l = Int16.Parse(n, CultureInfo.InvariantCulture);
                }
                catch(Exception)
                {
                    l = 0;
                }

                errorMessage = errorMessage.Substring(i + 1, errorMessage.Length - i - 1).Trim();
                if(errorMessage.Equals("warning: End of input with no newline, supplemented newline"))
                {
                    errorMessage = strer.ReadLine();
                    continue;
                }

                if(!String.IsNullOrEmpty(errorMessage))
                {
                    //
                    // Display only errors from this file or files outside the project.
                    //
                    bool currentFile = Util.equalPath(f, file);
                    bool found = Util.findItem(f, project.ProjectItems) != null;
                    TaskErrorCategory category = TaskErrorCategory.Error;
                    if(errorMessage.StartsWith("warning:", StringComparison.CurrentCultureIgnoreCase))
                    {
                        category = TaskErrorCategory.Warning;
                    }
                    else
                    {
                        hasErrors = true;
                    }
                    if(currentFile || !found)
                    {
                        if(found)
                        {
                            if (consoleOutput)
                            {
                                writeBuildOutput(errorMessage + "\n");
                            }
                            addError(project, file, category, l, 1, errorMessage);
                        }
                        else
                        {
                            if (consoleOutput)
                            {
                                writeBuildOutput("from file: " + f + "\n" + errorMessage + "\n");
                            }
                            addError(project, file, category, l, 1, "from file: " + f + "\n" + errorMessage);
                        }
                    }
                }
                errorMessage = strer.ReadLine();
            }
            return hasErrors;
        }

        public CommandBar projectCommandBar()
        {
            return findCommandBar(new Guid("{D309F791-903F-11D0-9EFC-00A0C911004F}"), 1026);
        }

        public CommandBar findCommandBar(Guid guidCmdGroup, uint menuID)
        {
            // Retrieve IVsProfferComands via DTE's IOleServiceProvider interface
            IOleServiceProvider sp = (IOleServiceProvider)_applicationObject;
            Guid guidSvc = typeof(IVsProfferCommands).GUID;
            object objService;
            int rc = sp.QueryService(ref guidSvc, ref guidSvc, out objService);
            if(ErrorHandler.Failed(rc))
            {
                try
                {
                    ErrorHandler.ThrowOnFailure(rc);
                }
                catch(Exception ex)
                {
                    writeBuildOutput(ex.ToString() + "\n");
                }
                return null;
            }
            IVsProfferCommands vsProfferCmds = (IVsProfferCommands)objService;
            return vsProfferCmds.FindCommandBar(IntPtr.Zero, ref guidCmdGroup, menuID) as CommandBar;
        }

        [ComImport,Guid("6D5140C1-7436-11CE-8034-00AA006009FA"),
         InterfaceTypeAttribute(ComInterfaceType.InterfaceIsIUnknown)]
        internal interface IOleServiceProvider 
        {
            [PreserveSig]
            int QueryService([In]ref Guid guidService, [In]ref Guid riid, 
                             [MarshalAs(UnmanagedType.Interface)] out System.Object obj);
        }

        private void buildBegin(vsBuildScope scope, vsBuildAction action)
        {
            try
            {
                _building = true;
                if(action == vsBuildAction.vsBuildActionBuild || action == vsBuildAction.vsBuildActionRebuildAll)
                {
                    switch(scope)
                    {
                        case vsBuildScope.vsBuildScopeProject:
                        {
                            Project project = getSelectedProject();
                            if(project != null)
                            {
                                if(!Util.isSliceBuilderEnabled(project))
                                {
                                    break;
                                }
                                clearErrors(project);
                                if(action == vsBuildAction.vsBuildActionRebuildAll)
                                {
                                    cleanProject(project);
                                }
                                buildProject(project, false, scope);
                            }
                            if(hasErrors(project))
                            {
                                bringErrorsToFront();
                                writeBuildOutput("------ Slice compilation contains errors. Build canceled. ------\n");
                                if (_connectMode == ext_ConnectMode.ext_cm_CommandLine)
                                {
                                    // Is this the best we can do? Is there a clean way to exit?
                                    Environment.Exit(-1);
                                }
                                _applicationObject.ExecuteCommand("Build.Cancel", "");
                            }
                            break;
                        }
                        default:
                        {
                            clearErrors();
                            foreach(Project p in _applicationObject.Solution.Projects)
                            {
                                if(p != null)
                                {
                                    if(!Util.isSliceBuilderEnabled(p))
                                    {
                                        continue;
                                    }
                                    if(action == vsBuildAction.vsBuildActionRebuildAll)
                                    {
                                        cleanProject(p);
                                    }
                                    buildProject(p, false, scope);
                                }
                            }
                            if(hasErrors())
                            {
                                bringErrorsToFront();
                                writeBuildOutput("------ Slice compilation contains errors. Build canceled. ------\n");
                                if (_connectMode == ext_ConnectMode.ext_cm_CommandLine)
                                {
                                    // Is this the best we can do? Is there a clean way to exit?
                                    Environment.Exit(-1);
                                }
                                _applicationObject.ExecuteCommand("Build.Cancel", "");
                            }
                            break;
                        }
                    }
                }
                else if(action == vsBuildAction.vsBuildActionClean)
                {
                    switch(scope)
                    {
                        case vsBuildScope.vsBuildScopeProject:
                        {
                            Project project = getSelectedProject();
                            if(project != null)
                            {
                                cleanProject(project);
                            }
                            break;
                        }
                        default:
                        {
                            foreach(Project p in _applicationObject.Solution.Projects)
                            {
                                if(p != null)
                                {
                                    cleanProject(p);
                                }
                            }
                            break;
                        }
                    }
                }
            }
            catch(Exception ex)
            {
                writeBuildOutput(ex.ToString() + "\n");
            }
        }

        //
        // Initialize slice builder error list provider
        //
        private void initErrorListProvider()
        {
            _errors = new List<ErrorTask>();
            _errorListProvider = new Microsoft.VisualStudio.Shell.ErrorListProvider(_serviceProvider);
            _errorListProvider.ProviderName = "Slice Error Provider";
            _errorListProvider.ProviderGuid = new Guid("B8DA84E8-7AE3-4c71-8E43-F273A20D40D1");
            _errorListProvider.Show();
        }

        //
        // Remove all errors from slice builder error list provider
        //
        private void clearErrors()
        {
            _errorCount = 0;
            _errors.Clear();
            _errorListProvider.Tasks.Clear();
        }

        private void clearErrors(Project project)
        {
            if(project == null || _errors == null)
            {
                return;
            }

            List<ErrorTask> remove = new List<ErrorTask>();
            foreach(ErrorTask error in _errors)
            {
                if(!error.HierarchyItem.Equals(getProjectHierarchy(project)))
                {
                    continue;
                }
                if(!_errorListProvider.Tasks.Contains(error))
                {
                    continue;
                }
                remove.Add(error);
                _errorListProvider.Tasks.Remove(error);
            }

            foreach(ErrorTask error in remove)
            {
                _errors.Remove(error);
            }
        }

        private void clearErrors(String file)
        {
            if(file == null || _errors == null)
            {
                return;
            }

            List<ErrorTask> remove = new List<ErrorTask>();
            foreach(ErrorTask error in _errors)
            {
                if(error.Document.Equals(file, StringComparison.CurrentCultureIgnoreCase))
                {
                    remove.Add(error);
                    _errorListProvider.Tasks.Remove(error);
                }
            }

            foreach(ErrorTask error in remove)
            {
                _errors.Remove(error);
            }

        }
        
        private IVsHierarchy getProjectHierarchy(Project project)
        {
            IVsSolution ivSSolution = getIVsSolution();
            IVsHierarchy hierarchy = null;
            if(ivSSolution != null)
            {
                int hr = ivSSolution.GetProjectOfUniqueName(project.UniqueName, out hierarchy);
                if(ErrorHandler.Failed(hr))
                { 
                    try
                    {
                        ErrorHandler.ThrowOnFailure(hr);
                    }
                    catch(Exception ex)
                    {
                        writeBuildOutput(ex.ToString() + "\n");
                    }
                }
            }
            return hierarchy;
        }
        
        //
        // Add a error to slice builder error list provider.
        //
        private void addError(Project project, string file, TaskErrorCategory category, int line, int column,
                              string text)
        {
            IVsHierarchy hierarchy = getProjectHierarchy(project);

            ErrorTask errorTask = new ErrorTask();
            errorTask.ErrorCategory = category;
            // Visual Studio uses indexes starting at 0 
            // while the automation model uses indexes starting at 1
            errorTask.Line = line - 1;
            errorTask.Column = column - 1;
            if(hierarchy != null)
            {
                errorTask.HierarchyItem = hierarchy;
            }
            errorTask.Navigate += new EventHandler(errorTaskNavigate);
            errorTask.Document = file;
            errorTask.Category = TaskCategory.BuildCompile;
            errorTask.Text = text;
            _errors.Add(errorTask);
            _errorListProvider.Tasks.Add(errorTask);
            if(category == TaskErrorCategory.Error)
            {
                ++_errorCount;
            }
        }

        //
        // True if there was any errors in last slice compilation.
        //
        private bool hasErrors()
        {
            return _errorCount > 0;
        }

        private bool hasErrors(Project project)
        {
            if(project == null || _errors == null)
            {
                return false;
            }

            bool errors = false;
            foreach(ErrorTask error in _errors)
            {
                if(error.HierarchyItem.Equals(getProjectHierarchy(project)))
                {
                    if(error.ErrorCategory == TaskErrorCategory.Error)
                    {
                        errors = true;
                        break;
                    }
                }
            }
            return errors;
        }

        private OutputWindowPane buildOutput()
        {
            if(_output == null)
            {
                OutputWindow window = (OutputWindow)_applicationObject.Windows.Item(EnvDTE.Constants.vsWindowKindOutput).Object;
                _output = window.OutputWindowPanes.Item("Build");
            }
            return _output;
        }

        private void writeBuildOutput(string message)
        {
            if (_connectMode != ext_ConnectMode.ext_cm_CommandLine)
            {
                OutputWindowPane pane = buildOutput();
                if (pane == null)
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

        //
        // Force the error list to show.
        //
        private void bringErrorsToFront()
        {
            if(_errorListProvider == null)
            {
                return;
            }
            _errorListProvider.BringToFront();
            _errorListProvider.ForceShowErrors();
        }

        //
        // Navigate to a file when the error is clicked.
        //
        private void errorTaskNavigate(object sender, EventArgs e)
        {
            ErrorTask task;
            try
            {
                task = (ErrorTask)sender;
                task.Line += 1;
                _errorListProvider.Navigate(task, new Guid(EnvDTE.Constants.vsViewKindTextView));
                task.Line -= 1;
            }
            catch(Exception)
            {
            }
        }

        private DTE2 _applicationObject;
        private AddIn _addInInstance;
        private ext_ConnectMode _connectMode;
        private SolutionEvents _solutionEvents;
        private BuildEvents _buildEvents;
        private DocumentEvents _docEvents;
        private ProjectItemsEvents _csProjectItemsEvents;
        private VCProjectEngineEvents _vcProjectItemsEvents;
        private ServiceProvider _serviceProvider;

        private ErrorListProvider _errorListProvider;
        private List<ErrorTask> _errors;
        private int _errorCount;
        private FileTracker _fileTracker;
        private Dictionary<string, Dictionary<string, List<string>>> _dependenciesMap;
        private string _deletedFile;
        private OutputWindowPane _output;

        private CommandEvents _addNewItemEvent;
        private CommandEvents _addExistingItemEvent;
        private CommandEvents _editRemoveEvent;
        private CommandEvents _editDeleteEvent;
        private List<String> _deleted = new List<String>();
        private Command _iceConfigurationCmd;
        private bool _building;
    }
}
