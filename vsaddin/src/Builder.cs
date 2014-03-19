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
using System.Windows.Forms;

using DependenciesMap = System.Collections.Generic.Dictionary<string,
    System.Collections.Generic.Dictionary<string,
        System.Collections.Generic.List<string>>>;


namespace Ice.VisualStudio
{

    //
    // This class is used to asynchronously read the output of a Slice compiler
    // process.
    //
    public class StreamReader
    {
        public void appendData(object sendingProcess, DataReceivedEventArgs outLine)
        {
            if(outLine.Data != null)
            {
                _data  += outLine.Data + "\n";
            }
        }

        public string data()
        {
            return _data;
        }

        private string _data = "";
    }

    public class Builder : IDisposable, IVsTrackProjectDocumentsEvents2
    {
        protected virtual void Dispose(bool disposing)
        {
            if(disposing)
            {
                if(_serviceProvider != null)
                {
                    _serviceProvider.Dispose();
                }

                if(_errorListProvider != null)
                {
                    _errorListProvider.Dispose();
                }
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

        public DTE2 getApplicationObject()
        {
            return _applicationObject;
        }

        public void init(DTE2 application, ext_ConnectMode connectMode, AddIn addInInstance)
        {
            _applicationObject = application;
            _addInInstance = addInInstance;
            _connectMode = connectMode;

            //
            // Subscribe to solution events.
            //
            if(_connectMode != ext_ConnectMode.ext_cm_CommandLine)
            {
                _solutionEvents = application.Events.SolutionEvents;
                _solutionEvents.Opened += new _dispSolutionEvents_OpenedEventHandler(solutionOpened);
                _solutionEvents.AfterClosing += new _dispSolutionEvents_AfterClosingEventHandler(afterClosing);
                _solutionEvents.ProjectAdded += new _dispSolutionEvents_ProjectAddedEventHandler(projectAdded);
                _solutionEvents.ProjectRemoved += new _dispSolutionEvents_ProjectRemovedEventHandler(projectRemoved);
                _solutionEvents.ProjectRenamed += new _dispSolutionEvents_ProjectRenamedEventHandler(projectRenamed);

                _selectionEvents = application.Events.SelectionEvents;
                _selectionEvents.OnChange += new _dispSelectionEvents_OnChangeEventHandler(selectionChange);
            }

            _buildEvents = _applicationObject.Events.BuildEvents;
            _buildEvents.OnBuildBegin += new _dispBuildEvents_OnBuildBeginEventHandler(buildBegin);
            _buildEvents.OnBuildDone += new _dispBuildEvents_OnBuildDoneEventHandler(buildDone);
            
            if(_connectMode != ext_ConnectMode.ext_cm_CommandLine)
            {
                beginTrackDocumentEvents();
                //
                // Ensure DEVPATH isn't empty, if there is a project in development mode and DEVPATH is
                // empty vshosting process will crash.
                //
                string devPath = Environment.GetEnvironmentVariable("DEVPATH");
                if(String.IsNullOrEmpty(devPath))
                {
                    setDotNetDevPath(Util.getIceHome() + "\\bin\\");
                }

                //
                // Subscribe to command events.
                //
                foreach(Command c in _applicationObject.Commands)
                {
                    if(c.Name.Equals("Project.AddNewItem"))
                    {
                        _addNewItemEvent = application.Events.get_CommandEvents(c.Guid, c.ID);
                        _addNewItemEvent.AfterExecute +=
                            new _dispCommandEvents_AfterExecuteEventHandler(afterAddNewItem);
                    }
                    else if(c.Name.Equals("Edit.Remove"))
                    {
                        _editRemoveEvent = application.Events.get_CommandEvents(c.Guid, c.ID);
                        _editRemoveEvent.AfterExecute +=
                            new _dispCommandEvents_AfterExecuteEventHandler(editDeleteEvent);
                    }
                    else if(c.Name.Equals("Edit.Delete"))
                    {
                        _editDeleteEvent = application.Events.get_CommandEvents(c.Guid, c.ID);
                        _editDeleteEvent.AfterExecute +=
                            new _dispCommandEvents_AfterExecuteEventHandler(editDeleteEvent);
                    }
                    else if(c.Name.Equals("Project.ExcludeFromProject"))
                    {
                        _excludeFromProjectEvent = application.Events.get_CommandEvents(c.Guid, c.ID);
                        _excludeFromProjectEvent.BeforeExecute +=
                            new _dispCommandEvents_BeforeExecuteEventHandler(beforeExcludeFromProjectEvent);
                        _excludeFromProjectEvent.AfterExecute +=
                            new _dispCommandEvents_AfterExecuteEventHandler(afterExcludeFromProjectEvent);
                    }
                    else if(c.Name.Equals("Project.AddExistingItem"))
                    {
                        _addExistingItemEvent = application.Events.get_CommandEvents(c.Guid, c.ID);
                        _addExistingItemEvent.AfterExecute +=
                            new _dispCommandEvents_AfterExecuteEventHandler(afterAddExistingItem);
                    }
                    else if(c.Name.Equals("Build.Cancel"))
                    {
                        _buildCancelEvent = application.Events.get_CommandEvents(c.Guid, c.ID);
                        _buildCancelEvent.AfterExecute +=
                            new _dispCommandEvents_AfterExecuteEventHandler(afterBuildCancel);
                    }
                    else if(c.Name.Equals("Debug.Start"))
                    {
                        _debugStartEvent = application.Events.get_CommandEvents(c.Guid, c.ID);
                        _debugStartEvent.BeforeExecute +=
                                        new _dispCommandEvents_BeforeExecuteEventHandler(setDebugEnvironmentStartupProject);
                    }
                    else if(c.Name.Equals("Debug.StepInto"))
                    {
                        _debugStepIntoEvent = application.Events.get_CommandEvents(c.Guid, c.ID);
                        _debugStepIntoEvent.BeforeExecute +=
                                        new _dispCommandEvents_BeforeExecuteEventHandler(setDebugEnvironmentStartupProject);
                    }
                    else if(c.Name.Equals("ClassViewContextMenus.ClassViewProject.Debug.StepIntonewinstance"))
                    {
                        _debugStepIntoNewInstance = application.Events.get_CommandEvents(c.Guid, c.ID);
                        _debugStepIntoNewInstance.BeforeExecute +=
                                        new _dispCommandEvents_BeforeExecuteEventHandler(setDebugEnvironmentActiveProject);
                    }
                    else if (c.Name.Equals("Debug.StartWithoutDebugging"))
                    {
                        _debugStartWithoutDebuggingEvent = application.Events.get_CommandEvents(c.Guid, c.ID);
                        _debugStartWithoutDebuggingEvent.BeforeExecute +=
                                        new _dispCommandEvents_BeforeExecuteEventHandler(setDebugEnvironmentStartupProject);
                    }
                    else if (c.Name.Equals("ClassViewContextMenus.ClassViewProject.Debug.Startnewinstance"))
                    {
                        _debugStartNewInstance = application.Events.get_CommandEvents(c.Guid, c.ID);
                        _debugStartNewInstance.BeforeExecute +=
                                        new _dispCommandEvents_BeforeExecuteEventHandler(setDebugEnvironmentActiveProject);
                    }
                    else if (c.Guid.Equals(Util.refreshCommandGUID) && c.ID == Util.refreshCommandID)
                    {
                        Util.setRefreshCommand(c);
                    }
                }
            }

            _serviceProvider =
                    new ServiceProvider((Microsoft.VisualStudio.OLE.Interop.IServiceProvider)_applicationObject.DTE);
            initErrorListProvider();

            if(connectMode != ext_ConnectMode.ext_cm_CommandLine)
            {
                setupCommandBars();
            }
        }

        void selectionChange()
        {
            try
            {
                Project p = getActiveProject();
                if(p != null && Util.isSliceBuilderEnabled(p))
                {
                    initializeProject(p);
                }
            }
            catch(Exception ex)
            {
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
        }

        void initializeProject(Project p)
        {
            DependenciesMap dependenciesMap = getDependenciesMap();
            if(p != null && !dependenciesMap.ContainsKey(p.FullName))
            {
                if((Util.isCSharpProject(p) || Util.isCppProject(p)) && Util.isSliceBuilderEnabled(p))
                {
                    Util.fix(p);

                    Util.getCurrentDTE().StatusBar.Text = "Ice Add-in: checking/updating settings for project '" + p.FullName + "'";
                    Util.verifyProjectSettings(p);
                    Util.getCurrentDTE().StatusBar.Text = "Ice Add-in: loading project '" + p.FullName + "'";
                    if(!Util.isVBProject(p))
                    {
                        dependenciesMap[p.FullName] = new Dictionary<string, List<string>>();
                        buildProject(p, true, vsBuildScope.vsBuildScopeSolution, false);
                    }
                }
            }
            if(hasErrors(p))
            {
                bringErrorsToFront();
            }
        }

        void editDeleteEvent(string Guid, int ID, object CustomIn, object CustomOut)
        {
            try
            {
                if(_deletedFile == null)
                {
                    return;
                }

                Project project = getActiveProject();
                if(project == null)
                {
                    return;
                }

                removeDependency(project, _deletedFile);
                _deletedFile = null;
                clearErrors(project);
                buildProject(project, false, vsBuildScope.vsBuildScopeProject, false);
            }
            catch(Exception ex)
            {
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
        }

        //
        // In C# project type the delete event isn't triggered by exclude item command.
        // We use the before and after events of the command to remove generated items 
        // when a slice file is excluded. C++ projects handle that as part of delete item
        // event.
        //
        void afterExcludeFromProjectEvent(string Guid, int ID, object CustomIn, object CustomOut)
        {
            try
            {
                if(String.IsNullOrEmpty(_excludedItem))
                {
                    return;
                }
                
                Project p = getActiveProject();
                if(!Util.isCSharpProject(p) || !Util.isSliceBuilderEnabled(p))
                {
                    return;
                }
                ProjectItem item = Util.findItem(_excludedItem, p.ProjectItems);
                if(item != null)
                {
                    item.Delete();                
                }
                updateDependencies(p);
            }
            catch(Exception ex)
            {
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
        }

        //
        // In C# project type the delete event isn't triggered by exclude item command.
        // We use the before and after events of the command to remove generated items 
        // when a slice file is excluded. C++ projects handle that as part of delete item
        // event.
        //
        public void beforeExcludeFromProjectEvent(string Guid, int ID, object obj, object CustomOut, ref bool done)
        {
            try
            {
                Project p = getActiveProject();
                if(!Util.isCSharpProject(p) || !Util.isSliceBuilderEnabled(p))
                {
                    return;
                }
                ProjectItem item = Util.getSelectedProjectItem(p.DTE);
                if(item == null)
                {
                    return;
                }

                if(!Util.isSliceFilename(item.Name))
                {
                    return;
                }
                
                _excludedItem = getCSharpGeneratedFileName(p, item, "cs");
                return;
            }
            catch(Exception ex)
            {
                Util.write(null, Util.msgLevel.msgError, ex.ToString() + "\n");
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
        }

        public IVsSolution getIVsSolution()
        {
            return (IVsSolution) _serviceProvider.GetService(typeof(IVsSolution));
        }

        public void buildDone(vsBuildScope Scope, vsBuildAction Action)
        {
            try
            {
                Util.solutionExplorerRefresh();
                _sliceBuild = false;
                //
                // If a Slice file has changed during the build, we rebuild that project's
                // Slice files now that the build is done.
                //
                List<Project> rebuildProjects = getRebuildProjects();
                foreach(Project p in rebuildProjects)
                {
                    buildProject(p, false, vsBuildScope.vsBuildScopeProject, false);
                }
                rebuildProjects.Clear();
            }
            catch(Exception ex)
            {
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
            finally
            {
                _buildProject = null;
                _building = false;
            }
        }

        //
        // Return true if the project build is in process.
        //
        public bool isBuilding(Project project)
        {
            if(!isBuilding())
            {
                return false;
            }
            if(_buildScope == vsBuildScope.vsBuildScopeSolution)
            {
                return true;
            }
            if(_buildScope == vsBuildScope.vsBuildScopeProject &&
               _buildProject == project)
            {
                return true;
            }
            return false;
        }
        
        //
        // Is our project building?
        //
        public bool isBuilding()
        {
            return _building;
        }
        

        //
        // If a Slice file created with "Add New Item" command cannot be added 
        // to the project because the generated items will override an existing item,
        // the Slice file must be deleted from disk, here, after the command has 
        // been executed.
        //
        public void afterAddNewItem(string Guid, int ID, object obj, object CustomOut)
        {
            try
            {
                foreach(String path in _deleted)
                {
                    ProjectItem item = Util.findItem(path);
                    if(item != null)
                    {
                        item.Remove();
                    }

                    if(String.IsNullOrEmpty(path))
                    {
                        continue;
                    }
                    if(File.Exists(path))
                    {
                        try
                        {
                            File.Delete(path);
                        }
                        catch(System.SystemException)
                        { 
                            // Can happen if the file is used by another process.
                        }
                    }
                }
                _deleted.Clear();
            }
            catch(Exception ex)
            {
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
        }

        //
        // If a Slice file added with "Add Existing Item" command cannot be added 
        // to the project because the generated items will override an existing item,
        // the item must not be deleted here, we must empty the _deleted list so the
        // file isn't later removed.
        //
        public void afterAddExistingItem(string Guid, int ID, object obj, object CustomOut)
        {
            try
            {
                foreach (String path in _deleted)
                {
                    ProjectItem item = Util.findItem(path);
                    if(item != null)
                    {
                        item.Remove();
                    }
                }
                _deleted.Clear();
            }
            catch(Exception ex)
            {
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
        }

        public void afterBuildCancel(string Guid, int ID, object obj, object CustomOut)
        {
            try
            {
                Util.solutionExplorerRefresh();
                _sliceBuild = false;

                //
                // If a Slice file has changed during the build, we rebuild that project's
                // Slice files now that the build has been canceled.
                //
                List<Project> rebuildProjects = getRebuildProjects();
                foreach(Project p in rebuildProjects)
                {
                    buildProject(p, false, vsBuildScope.vsBuildScopeProject, false);
                }
                rebuildProjects.Clear();
            }
            catch(Exception ex)
            {
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
            finally
            {
                _buildProject = null;
                _building = false;
            }
        }

        public void setDebugEnvironmentStartupProject(string Guid, int ID, object obj, object CustomOut, ref bool done)
        {
            setDebugEnvironment(getStartupProject());
        }

        public void setDebugEnvironmentActiveProject(string Guid, int ID, object obj, object CustomOut, ref bool done)
        {
            setDebugEnvironment(getActiveProject());
        }

        public void setDebugEnvironment(Project project)
        {
            try
            {
                if(project != null && Util.isSliceBuilderEnabled(project))
                {
                    if(Util.isCppProject(project))
                    {
                        VCProject vcProject = (VCProject)project.Object;
                        IVCCollection configurations = (IVCCollection)vcProject.Configurations;
                        foreach(VCConfiguration conf in configurations)
                        {
                            Util.addIceCppEnvironment((VCDebugSettings)conf.DebugSettings, project);
                        }
                    }
                    else if(Util.isCSharpProject(project) || Util.isVBProject(project))
                    {
                        setDotNetDebugEnvironment(project);
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
        // Set DEVPATH if .NET development mode is enabled and project type is C# or VB,
        // otherwise do nothing.
        //
        // NOTE: for Silverlight projects we don't need to set DEVPATH.
        //
        private void setDotNetDebugEnvironment(Project project)
        {
            //
            // If development mode isn't enabled then don't set DEVPATH.
            //
            if(!Util.developmentMode(project))
            {
                return;
            }

            //
            // Check if vshosting process is enabled, if so disable it before updating
            // environment variables. If it is running it will be stopped.
            //
            bool vsHosting = false;
            if(Util.useVSHostingProcess(project))
            {
                Util.setVsHostingProcess(project, false);
                vsHosting = true;
            }

            setDotNetDevPath(Util.getCsBinDir(project));

            //
            // Re-enable the vshosting process if it was previously enabled, 
            // so it reads the new environment when it is started by Visual Studio.
            //
            if(vsHosting)
            {
                Util.setVsHostingProcess(project, true);
            }
        }

        private static void setDotNetDevPath(string csBinPath)
        {
            string devPath = Environment.GetEnvironmentVariable("DEVPATH");

            if(String.IsNullOrEmpty(devPath))
            {
                Environment.SetEnvironmentVariable("DEVPATH", csBinPath);
                return;
            }

            if(devPath.Contains(csBinPath))
            {
                ComponentList list = new ComponentList(csBinPath.Split(Path.PathSeparator));
                list.Remove(csBinPath);
                devPath = list.ToString(Path.PathSeparator);
            }

            devPath = csBinPath + Path.PathSeparator + devPath;
            devPath = devPath.Trim(Path.PathSeparator);
            Environment.SetEnvironmentVariable("DEVPATH", devPath);
        }
        
        public void disconnect()
        {
            if(_iceConfigurationCmd != null)
            {
                _iceConfigurationCmd.Delete();
            }
            
            if(_connectMode != ext_ConnectMode.ext_cm_CommandLine)
            {
                _solutionEvents.Opened -= new _dispSolutionEvents_OpenedEventHandler(solutionOpened);
                _solutionEvents.AfterClosing -= new _dispSolutionEvents_AfterClosingEventHandler(afterClosing);
                _solutionEvents.ProjectAdded -= new _dispSolutionEvents_ProjectAddedEventHandler(projectAdded);
                _solutionEvents.ProjectRemoved -= new _dispSolutionEvents_ProjectRemovedEventHandler(projectRemoved);
                _solutionEvents.ProjectRenamed -= new _dispSolutionEvents_ProjectRenamedEventHandler(projectRenamed);
                _solutionEvents = null;
            }
            _buildEvents.OnBuildBegin -= new _dispBuildEvents_OnBuildBeginEventHandler(buildBegin);
            _buildEvents.OnBuildDone -= new _dispBuildEvents_OnBuildDoneEventHandler(buildDone);

            _buildEvents = null;

            if(_connectMode != ext_ConnectMode.ext_cm_CommandLine)
            {
                endTrackDocumentEvents();
            }            
            if(_dependenciesMap != null)
            {
                _dependenciesMap.Clear();
                _dependenciesMap = null;
            }

            if(_rebuildProjects != null)
            {
                _rebuildProjects.Clear();
                _rebuildProjects = null;
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
                MessageBox.Show("Error initializing Ice Visual Studio Add-in.\n" +
                                "Cannot create required commands",
                                "Ice Visual Studio Add-in",
                                MessageBoxButtons.OK,
                                MessageBoxIcon.Error,
                                MessageBoxDefaultButton.Button1,
                                (MessageBoxOptions)0);
                return;
            }

            CommandBar toolsCmdBar = ((CommandBars)_applicationObject.CommandBars)["Tools"];
            _iceConfigurationCmd.AddControl(toolsCmdBar, toolsCmdBar.Controls.Count + 1);

            CommandBar projectCmdBar = projectCommandBar();
            _iceConfigurationCmd.AddControl(projectCmdBar, projectCmdBar.Controls.Count + 1);   
        }

        public void afterClosing()
        {
            try
            {
                clearErrors();
                removeDocumentEvents();
                if(_dependenciesMap != null)
                {
                    _dependenciesMap.Clear();
                    _dependenciesMap = null;
                }

                if(_rebuildProjects != null)
                {
                    _rebuildProjects.Clear();
                    _rebuildProjects = null;
                }
            }
            catch(Exception ex)
            {
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
            finally
            {
                _opened = false;
            }
        }

        public void solutionOpened()
        {
            try
            {
                _opening = true;
                DependenciesMap dependenciesMap = getDependenciesMap();
                initDocumentEvents();
            }
            catch(Exception ex)
            {
                _opening = false;
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
            Util.getCurrentDTE().StatusBar.Text = "Ready";
            _opening = false;
            _opened = false;
        }
        
        //
        // Enable slice builder for the project with default components.
        //
        public void addBuilderToProject(Project project)
        {
            addBuilderToProject(project, new ComponentList());
        }

        //
        // Enable Slice builder for the project, and enable the components that are
        // in components. If components list is empty, the default set of components
        // are added to the project.
        //
        // Note: Components in this context is the list of Ice libraries or assemblies
        // that will be added to the project.
        //
        public void addBuilderToProject(Project project, ComponentList components)
        {
            if(Util.isCppProject(project))
            {
                Util.addIceCppConfigurations(project);
                if(components.Count == 0)
                {
                    components = 
                        new ComponentList(Util.getProjectProperty(project, Util.PropertyIceComponents));
                }

                if(!components.Contains("Ice"))
                {
                    components.Add("Ice");
                }
                if(!Util.isWinRTProject(project))
                {
                    if(!components.Contains("IceUtil"))
                    {
                        components.Add("IceUtil");
                    }
                }
                Util.addIceCppLibs(project, components);
            }
            else
            {                
                if(Util.isCSharpProject(project))
                {
                    bool development = Util.developmentMode(project);                        
                    if(components.Count == 0)
                    {
                        components = 
                            new ComponentList(Util.getProjectProperty(project, Util.PropertyIceComponents));
                    }
                    if(!components.Contains("Ice"))
                    {
                        components.Add("Ice");
                    }
                    string iceHome = Util.getIceHome();
                    foreach(string component in components)
                    {
                        Util.addDotNetReference(project, component, development);
                    }
                }
                else if(Util.isVBProject(project))
                {
                    string iceHome = Util.getIceHome();
                    bool development = Util.developmentMode(project);
                    if(components.Count == 0)
                    {
                        components = 
                            new ComponentList(Util.getProjectProperty(project, Util.PropertyIceComponents));
                    }
                    if(!components.Contains("Ice"))
                    {
                        components.Add("Ice");
                    }
                    foreach(string component in components)
                    {
                        Util.addDotNetReference(project, component, development);
                    }
                }
            }

            Util.setProjectProperty(project, Util.PropertyIceComponents, "");
            Util.setProjectProperty(project, Util.PropertyIce, true.ToString());

            if(hasErrors(project))
            {
                bringErrorsToFront();
            }
        }

        public void removeBuilderFromProject(Project project, ComponentList components)
        {
            cleanProject(project, true);
            if(Util.isCppProject(project))
            {
                Util.removeIceCppConfigurations(project);
                Util.setProjectProperty(project, Util.PropertyIceComponents, components.ToString());
            }
            else if(Util.isCSharpProject(project))
            {
                Util.removeDotNetReference(project, "Ice");
            }

            Util.setProjectProperty(project, Util.PropertyIceComponents, components.ToString());
            Util.setProjectProperty(project, Util.PropertyIce, false.ToString());
        }

        //
        // Ensure that generated items are opened in read only mode.
        //
        private void documentOpened(Document document)
        {
            try
            {
                if(document == null || document.ProjectItem == null || document.ProjectItem.ContainingProject == null)
                {
                    return;
                }
                if(!Util.isSliceBuilderEnabled(document.ProjectItem.ContainingProject))
                {
                    return;
                }
                if(fileTracker().hasGeneratedFile(document.ProjectItem.ContainingProject, document.FullName))
                {
                    if(!document.ReadOnly)
                    {
                        document.ReadOnly = true;
                    }
                }
            }
            catch(Exception ex)
            {
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
        }

        public void documentSaved(Document document)
        {
            try
            {
                Project project = null;
                try
                {
                    project = document.ProjectItem.ContainingProject;
                }
                catch(Exception)
                {
                    //
                    // Expected when documents are created during project initialization
                    // and the ProjectItem is not yet available.
                    //
                    return;
                }

                if(!Util.isSliceBuilderEnabled(project))
                {
                    return;
                }
                if(!Util.isSliceFilename(document.Name))
                {
                    return;
                }

                //
                // If build is in proccess, we don't run the slice compiler now, we append the document
                // to a list of projects that have changes and return. The projects on this list 
                // will be rebuilt when the current build process is done or canceled, see 
                // "buildDone" and "afterBuildCancel" methods in this class.
                //
                if(isBuilding(project))
                {
                    List<Project> rebuildProjects = getRebuildProjects();
                    if(!rebuildProjects.Contains(project))
                    {
                        rebuildProjects.Add(project);
                    }
                    return;
                }

                clearErrors(project);
                buildProject(project, false, vsBuildScope.vsBuildScopeProject, false);
                Util.solutionExplorerRefresh();
            }
            catch(Exception ex)
            {
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
        }
        
        public void projectAdded(Project project)
        {
            if(!_opened)
            {
                return;
            }
            try
            {
                if(Util.isSliceBuilderEnabled(project))
                {
                    Util.verifyProjectSettings(project);
                    updateDependencies(project);
                    Util.solutionExplorerRefresh();
                }
            }
            catch(Exception ex)
            {
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
        }

        public void projectRemoved(Project project)
        {
            try
            {
                if(!Util.isSliceBuilderEnabled(project))
                {
                    return;
                }
                DependenciesMap dependenciesMap = getDependenciesMap();
                if(dependenciesMap.ContainsKey(project.FullName))
                {
                    dependenciesMap.Remove(project.FullName);
                }

                List<Project> rebuildProjects = getRebuildProjects();
                foreach(Project p in rebuildProjects)
                {
                    if(project == p)
                    {
                        rebuildProjects.Remove(p);
                        break;
                    }
                }
            }
            catch(Exception ex)
            {
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
        }

        public void projectRenamed(Project project, string oldName)
        {
            try
            {
                if(!Util.isSliceBuilderEnabled(project))
                {
                    return;
                }
                DependenciesMap dependenciesMap = getDependenciesMap();
                String oldPath = Path.Combine(Path.GetDirectoryName(project.FullName), oldName);
                if(dependenciesMap.ContainsKey(oldPath))
                {
                    dependenciesMap.Remove(oldPath);
                }
                updateDependencies(project);
            }
            catch(Exception ex)
            {
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
        }

        public void cleanProject(Project project, bool remove)
        {
            DTE dte = Util.getCurrentDTE();
            if(!_opening)
            {
                dte.StatusBar.Text = "Ice Add-in: cleaning project '" + project.FullName + "'";
            }
            if(project == null)
            {
                return;
            }
            if(!Util.isSliceBuilderEnabled(project))
            {
                return;
            }
            clearErrors(project);

            if(Util.isCSharpProject(project))
            {
                removeCSharpGeneratedItems(project, project.ProjectItems, remove);
            }
            else if(Util.isCppProject(project))
            {
                removeCppGeneratedItems(project.ProjectItems, remove);
            }
            if(!_opening)
            {
                dte.StatusBar.Text = "Ready";
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
            List<ProjectItem> tmpItems = Util.clone(items);
            foreach(ProjectItem i in tmpItems)
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

        public void buildProject(Project project, bool force, vsBuildScope scope, bool buildDependencies)
        {
            List<Project> builded = new List<Project>();
            buildProject(project, force, null, scope, buildDependencies, ref builded);
        }

        public void buildProject(Project project, bool force, vsBuildScope scope, bool buildDependencies, ref List<Project> builded)
        {
            buildProject(project, force, null, scope, buildDependencies, ref builded);
        }

        public void buildProject(Project project, bool force, ProjectItem excludeItem, vsBuildScope scope, bool buildDependencies)
        {
            List<Project> builded = new List<Project>();
            buildProject(project, force, excludeItem, scope, buildDependencies, ref builded);
        }

        public void buildProject(Project project, bool force, ProjectItem excludeItem, vsBuildScope scope, bool buildDependencies,
                                 ref List<Project> builded)
        {
            if(project == null)
            {
                return;
            }

            if(!Util.isSliceBuilderEnabled(project))
            {
                return;
            }

            if(builded.Contains(project))
            {
                return;
            }

            if(_deleted.Count > 0)
            {
                return;
            }

            initializeProject(project);

            builded.Add(project);

            List<ProjectItem> buildItems = new List<ProjectItem>();
            //
            // When building a single project, we must first build projects 
            // that this project depends on.
            //
            if(vsBuildScope.vsBuildScopeProject == scope && buildDependencies)
            {
                BuildDependencies dependencies = _applicationObject.Solution.SolutionBuild.BuildDependencies;
                for(int i = 0; i < dependencies.Count; ++i)
                {
                    BuildDependency dp = dependencies.Item(i + 1);
                    if(dp == null)
                    {
                        continue;
                    }

                    if(dp.Project.Equals(project))
                    {
                        System.Array projects = dp.RequiredProjects as System.Array;
                        if(projects != null)
                        {
                            foreach(Project p in projects)
                            {
                                buildProject(p, force, vsBuildScope.vsBuildScopeProject, buildDependencies, ref builded);
                            }
                        }
                    }
                }
            }

            if(Util.isVBProject(project))
            {
                //
                // For VB projects we just build dependencies.
                //
                return;
            }

            DTE dte = Util.getCurrentDTE();
            if(!_opening)
            {
                dte.StatusBar.Text = "Ice Add-in: building project '" + project.FullName + "'";
            }

            string msg = "------ Slice compilation started " + "Project: " + Util.getTraceProjectName(project) + " ------\n";
            Util.write(project, Util.msgLevel.msgInfo, msg);

            int verboseLevel = Util.getVerboseLevel(project);
            DateTime now = DateTime.Now;
            if(verboseLevel >= (int)Util.msgLevel.msgDebug)
            {
                Util.write(project, Util.msgLevel.msgDebug, "DEBUG Start Time: " + now.ToShortDateString() + " " + 
                                                            now.ToLongTimeString() + "\n");
            }

            if(Util.isCSharpProject(project))
            {
                buildCSharpProject(project, force, excludeItem);
            }
            else if(Util.isCppProject(project))
            {
                buildCppProject(project, force, ref buildItems);
            }

            if(verboseLevel >= (int)Util.msgLevel.msgDebug)
            {
                System.TimeSpan t = System.DateTime.Now - now;
                Util.write(project, Util.msgLevel.msgDebug, "DEBUG Output:\n");
                Util.write(project, Util.msgLevel.msgDebug, "DEBUG Time Elapsed: " + t.ToString() + "\n");
            }

            if(hasErrors(project))
            {
                Util.write(project, Util.msgLevel.msgError,
                    "------ Slice compilation failed: Project: " + Util.getTraceProjectName(project) +" ------\n\n");
            }
            else
            {
                Util.write(project, Util.msgLevel.msgInfo,
                    "------ Slice compilation succeeded: Project: " + Util.getTraceProjectName(project) + " ------\n\n");
            }
            if(!_opening)
            {
                dte.StatusBar.Text = "Ready";
            }
        }

        public bool buildCppProject(Project project, bool force, ref List<ProjectItem> buildedItems)
        {
            VCConfiguration conf = Util.getActiveVCConfiguration(project);
            if(conf.ConfigurationType == ConfigurationTypes.typeGeneric ||
               conf.ConfigurationType == ConfigurationTypes.typeUnknown)
            {
                string err = "Configuration Type: '" + conf.ConfigurationType.ToString() + "' not supported by Ice Visual Studio Add-in";
                Util.write(project, Util.msgLevel.msgError,
                    "------ Slice compilation failed: Project: " + Util.getTraceProjectName(project) + " ------\n\n" +
                    err);
                MessageBox.Show(err, "Ice Visual Studio Add-in", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1,
                                (MessageBoxOptions)0);

                Connect.getBuilder().addError(project, "", TaskErrorCategory.Error, 0, 0, err);

                return false;
            }

            VCCLCompilerTool compilerTool =
                    (VCCLCompilerTool)(((IVCCollection)conf.Tools).Item("VCCLCompilerTool"));

            bool staticLib = conf.ConfigurationType == Microsoft.VisualStudio.VCProjectEngine.ConfigurationTypes.typeStaticLibrary;
            LinkerAdapter linkerAdapter;
            if(staticLib)
            {
                linkerAdapter = new StaticLinkerAdapter((VCLibrarianTool)(((IVCCollection)conf.Tools).Item("VCLibrarianTool")));
            }
            else
            {
                linkerAdapter = new DynamicLinkerAdapter((VCLinkerTool)(((IVCCollection)conf.Tools).Item("VCLinkerTool")));
            }

            if(!_opening)
            {
                Util.checkCppRunTimeLibrary(this, project, compilerTool, linkerAdapter);
            }
            string sliceCompiler = getSliceCompilerPath(project);
            return buildCppProject(project, project.ProjectItems, sliceCompiler, force, ref buildedItems);
        }

        public bool buildCppProject(Project project, ProjectItems items, string sliceCompiler, bool force,
                                    ref List<ProjectItem> buildedItems)
        {
            bool success = true;
            List<ProjectItem> tmpItems = Util.clone(items);
            foreach(ProjectItem i in tmpItems)
            {
                if(i == null)
                {
                    continue;
                }

                if(Util.isProjectItemFilter(i))
                {
                    if(!buildCppProject(project, i.ProjectItems, sliceCompiler, force, ref buildedItems))
                    {
                        success = false;
                    }
                }
                else if(Util.isProjectItemFile(i))
                {
                    if(!buildCppProjectItem(project, i, sliceCompiler, force, ref buildedItems))
                    {
                        success = false;
                    }
                }
            }
            return success;
        }

        public bool buildCppProjectItem(Project project, ProjectItem item, string sliceCompiler, bool force,
                                        ref List<ProjectItem> buildedItems)
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
            FileInfo hFileInfo = new FileInfo(getCppGeneratedFileName(project,
                                              iceFileInfo.FullName, Util.getHeaderExt(project)));
            FileInfo cppFileInfo = new FileInfo(Path.ChangeExtension(hFileInfo.FullName, Util.getSourceExt(project)));

            string output = Path.GetDirectoryName(cppFileInfo.FullName);
            return buildCppProjectItem(project, output, iceFileInfo, cppFileInfo, hFileInfo, sliceCompiler, force, 
                                       ref buildedItems);
        }

        public bool buildCppProjectItem(Project project, String output, FileSystemInfo ice, FileSystemInfo cpp,
                                        FileSystemInfo h, string sliceCompiler, bool force,
                                        ref List<ProjectItem> buildedItems)
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
                // Now check if any of the dependencies have changed.
                //
                DependenciesMap solutionDependenciesMap = getDependenciesMap();
                if(solutionDependenciesMap.ContainsKey(project.FullName))
                {
                    Dictionary<string, List<string>> dependenciesMap = solutionDependenciesMap[project.FullName];
                    if(dependenciesMap.ContainsKey(ice.FullName))
                    {
                        List<string> fileDependencies = dependenciesMap[ice.FullName];
                        foreach(string name in fileDependencies)
                        {
                            FileInfo dependency = new FileInfo(Util.absolutePath(project, name));
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

            Util.write(project, Util.msgLevel.msgInfo, ice.Name + ":\n");
            if(updated || force)
            {
                if(!Directory.Exists(output))
                {
                    Directory.CreateDirectory(output);
                }

                if(updateDependencies(project, null, ice.FullName, sliceCompiler) && updated)
                {
                    Util.write(project, Util.msgLevel.msgInfo, "  Generating C++ files: " + cpp.Name + ", " + h.Name + "\n");

                    if(runSliceCompiler(project, sliceCompiler, ice.FullName, output))
                    {
                        buildedItems.Add(Util.findItem(ice.FullName, project.ProjectItems));
                        success = true;
                    }
                }
            }
            
            if(!updated)
            {
                if(!force)
                {
                    Util.write(project, Util.msgLevel.msgInfo, "  Dependencies data is up to date\n");
                }
                Util.write(project, Util.msgLevel.msgInfo, "  Generated C++ files are up to date\n");
            }
            //
            // Make sure generated files are part of project, and tracked by the FileTracker.
            //
            addCppGeneratedFiles(project, ice, cpp, h);
            return !updated || success;
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
                fileTracker().trackFile(project, ice.FullName, h.FullName);
                VCFile file = Util.findVCFile((IVCCollection)vcProject.Files, cpp.Name, cpp.FullName);
                if(file == null)
                {
                    vcProject.AddFile(cpp.FullName);
                }
            }

            if(File.Exists(h.FullName))
            {
                fileTracker().trackFile(project, ice.FullName, cpp.FullName);
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

        public void buildCSharpProject(Project project, string projectDir, ProjectItems items, string sliceCompiler,
            bool force, ProjectItem excludeItem)
        {
            List<ProjectItem> tmpItems = Util.clone(items);
            foreach(ProjectItem i in tmpItems)
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

        public static String getCppGeneratedFileName(Project project, String fullPath, string extension)
        {
            if(project == null || String.IsNullOrEmpty(fullPath))
            {
                return "";
            }

            if(!Util.isSliceFilename(fullPath))
            {
                return "";
            }

            string projectDir = Path.GetDirectoryName(project.FileName).Trim();
            string outputAbsolutePath = Util.getProjectAbsoluteOutputDir(project);
            
            string itemRelativePath = "";

            //
            // If source isn't inside project directory, we put the generated file in the
            // root of our output directory.
            //
            if(!Path.GetFullPath(fullPath).StartsWith(Path.GetFullPath(projectDir),
                                                      StringComparison.CurrentCultureIgnoreCase))
            {
                itemRelativePath = Path.GetFileName(fullPath);
            }
            //
            // If source file is in generated directory, just change the extension to the path.
            //
            else if(Path.GetFullPath(fullPath).StartsWith(outputAbsolutePath, 
                                                          StringComparison.CurrentCultureIgnoreCase))
            {
                return Path.ChangeExtension(fullPath, extension);
            }
            else
            {
                itemRelativePath = Util.relativePath(project, Path.GetDirectoryName(fullPath));
            }

            if(String.IsNullOrEmpty(itemRelativePath))
            {
                return "";
            }

            string generatedDir = Path.GetDirectoryName(itemRelativePath);

            string path = System.IO.Path.Combine(outputAbsolutePath, generatedDir);
            return Path.GetFullPath(
                            Path.Combine(path, Path.ChangeExtension(Path.GetFileName(fullPath), extension))).Trim();
        }

        public static string getCSharpGeneratedFileName(Project project, ProjectItem item, string extension)
        {
            if(project == null || item == null || String.IsNullOrEmpty(extension))
            {
                return "";
            }
            string fullPath = Path.GetFullPath(Path.Combine(Path.GetDirectoryName(project.FileName), 
                                                            Util.getPathRelativeToProject(item)));

            return getCSharpGeneratedFileName(project, fullPath, extension).Trim();
        }

        public static string getCSharpGeneratedFileName(Project project, string fullPath, string extension)
        {
            if(project == null || String.IsNullOrEmpty(fullPath) || String.IsNullOrEmpty(extension))
            {
                return "";
            }

            if(!Util.isSliceFilename(fullPath))
            {
                return "";
            }

            string projectDir = Path.GetDirectoryName(project.FileName);

            string itemRelativePath = Util.relativePath(project, fullPath);
            if(String.IsNullOrEmpty(itemRelativePath))
            {
                return "";
            }

            string outputAbsolutePath = Util.getProjectAbsoluteOutputDir(project);
            //
            // If source file is in generated directory, just change the extension to the path.
            //
            if(Path.GetFullPath(fullPath).StartsWith(outputAbsolutePath, StringComparison.CurrentCultureIgnoreCase))
            {
                return Path.ChangeExtension(fullPath, extension);
            }

            string generatedDir = Path.GetDirectoryName(itemRelativePath);

            string path = Path.Combine(outputAbsolutePath, generatedDir);
            return Path.GetFullPath(
                            Path.Combine(path, Path.ChangeExtension(Path.GetFileName(fullPath), extension))).Trim();
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
                // Now check if any of the dependencies have changed.
                //
                DependenciesMap solutionDependenciesMap = getDependenciesMap();
                if(solutionDependenciesMap.ContainsKey(project.FullName))
                {
                    Dictionary<string, List<string>> dependenciesMap = solutionDependenciesMap[project.FullName];
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

            Util.write(project, Util.msgLevel.msgInfo, iceFileInfo.Name + ":\n");

            if(updated || force)
            {
                if(updateDependencies(project, item, iceFileInfo.FullName, sliceCompiler) &&
                   updated)
                {
                    Util.write(project, Util.msgLevel.msgInfo, "  Generating C# file: " + generatedFileInfo.Name + "\n");

                    if(runSliceCompiler(project, sliceCompiler, iceFileInfo.FullName, generatedFileInfo.DirectoryName))
                    {
                        success = true;
                    }
                }
            }

            if(!updated)
            {
                if(!force)
                {
                    Util.write(project, Util.msgLevel.msgInfo, "  Dependencies data is up to date\n");
                }
                Util.write(project, Util.msgLevel.msgInfo, "  Generated C# files are up to date\n");
            }
            //
            // Make sure generated files are part of project, and tracked by FileTracker.
            //
            addCSharpGeneratedFiles(project, iceFileInfo, generatedFileInfo);
            return !updated || success;
        }

        private void addCSharpGeneratedFiles(Project project, FileInfo ice, FileInfo file)
        {
            if(File.Exists(file.FullName))
            {
                fileTracker().trackFile(project, ice.FullName, file.FullName);

                ProjectItem generatedItem = Util.findItem(file.FullName, project.ProjectItems);
                if(generatedItem == null)
                {
                    project.ProjectItems.AddFromFile(file.FullName);
                }
            }
        }
        
        public static string getSliceCompilerPath(Project project)
        {
            string compiler = Util.slice2cpp;
            if(Util.isCSharpProject(project))
            {
                compiler = Util.slice2cs;
            }

            if(!String.IsNullOrEmpty(Environment.GetEnvironmentVariable("IceSourceHome")))
            {
                return Path.Combine(Environment.GetEnvironmentVariable("IceSourceHome"), 
                                                                    Path.Combine("cpp", Path.Combine("bin", compiler)));
            }

            string iceHome = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
            if(iceHome.EndsWith("\\vsaddin", StringComparison.CurrentCultureIgnoreCase))
            {
                return Path.Combine(iceHome.Substring(0, iceHome.Length - "\\vsaddin".Length), 
                                                                                        Path.Combine("bin", compiler));
            }
            if(iceHome.EndsWith("\\vsaddin\\bin", StringComparison.CurrentCultureIgnoreCase))
            {
                return Path.Combine(iceHome.Substring(0, iceHome.Length - "\\vsaddin\\bin".Length), 
                                                                    Path.Combine("cpp", Path.Combine("bin", compiler)));
            }
            throw new ArgumentException("Unable to determite Slice compiler location");
        }

        private static string getSliceCompilerArgs(Project project, string file, bool depend)
        {
            IncludePathList includes = 
                new IncludePathList(Util.getProjectProperty(project, Util.PropertyIceIncludePath));
            string extraOpts = Util.getProjectProperty(project, Util.PropertyIceExtraOptions).Trim();

            //
            // Add per build configuration extra options.
            //
            Configuration activeConfiguration = Util.getActiveConfiguration(project);
            if(activeConfiguration != null && !activeConfiguration.ConfigurationName.Equals("All"))
            {
                string property = Util.PropertyIceExtraOptions + "_" + activeConfiguration.ConfigurationName;
                extraOpts += " " + Util.getProjectProperty(project, property).Trim();
                extraOpts = extraOpts.Trim();
            }

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

                String preCompiledHeader = Util.getPrecompileHeader(project, file);
                if(!String.IsNullOrEmpty(preCompiledHeader))
                {
                    args += "--add-header=" + Util.quote(preCompiledHeader) + " ";
                }
            }
            args += "-I\"" + Util.getIceHome() + "\\slice\" ";

            foreach(string i in includes)
            {
                if(String.IsNullOrEmpty(i))
                {
                    continue;
                }
                String include = Util.expandEnvironmentVars(i);
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

            if(!String.IsNullOrEmpty(extraOpts))
            {
                args += Util.expandEnvironmentVars(extraOpts) + " ";
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
            DependenciesMap dependenciesMap = getDependenciesMap();
            dependenciesMap[project.FullName] = new Dictionary<string, List<string>>();
            string sliceCompiler = getSliceCompilerPath(project);
            return updateDependencies(project, project.ProjectItems, sliceCompiler, excludeItem);
        }

        public void cleanDependencies(Project project, string file)
        {
            if(project == null || file == null)
            {
                return;
            }

            if(String.IsNullOrEmpty(project.FullName))
            {
                return;
            }

            DependenciesMap dependenciesMap = getDependenciesMap();
            if(!dependenciesMap.ContainsKey(project.FullName))
            {
                return;
            }

            Dictionary<string, List<string>> projectDependencies = dependenciesMap[project.FullName];
            if(!projectDependencies.ContainsKey(file))
            {
                return;
            }
            projectDependencies.Remove(file);
            dependenciesMap[project.FullName] = projectDependencies;
        }

        public bool updateDependencies(Project project, ProjectItems items, string sliceCompiler,
                                       ProjectItem excludeItem)
        {
            bool success = true;
            List<ProjectItem> tmpItems = Util.clone(items);
            foreach(ProjectItem item in tmpItems)
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
            Util.write(project, Util.msgLevel.msgInfo, "  Computing dependencies\n");

            if(!File.Exists(sliceCompiler))
            {
                Util.write(project, Util.msgLevel.msgError,
                           "'" + sliceCompiler + "' not found, review your Ice installation");
                addError(project, file, TaskErrorCategory.Error, 0, 0,
                         "'" + sliceCompiler + "' not found, review your Ice installation");
                return false;
            }

            string args = getSliceCompilerArgs(project, file, true) + " " + Util.quote(file);

            System.Diagnostics.Process process = new System.Diagnostics.Process();
            process.StartInfo.FileName = sliceCompiler;
            process.StartInfo.Arguments = args;
            process.StartInfo.CreateNoWindow = true;
            process.StartInfo.UseShellExecute = false;
            process.StartInfo.RedirectStandardError = true;
            process.StartInfo.RedirectStandardOutput = true;

            process.StartInfo.WorkingDirectory = Path.GetDirectoryName(project.FileName);
            StreamReader reader = new StreamReader();
            process.OutputDataReceived += new DataReceivedEventHandler(reader.appendData);

            Util.write(project, Util.msgLevel.msgDebug,"DEBUG Command-line: " + sliceCompiler + " " + args + "\n");
            
            try
            {
                process.Start();
            }
            catch(InvalidOperationException ex)
            {
                Util.write(project, Util.msgLevel.msgError,
                           "An exception was thrown when trying to start the Slice compiler\n" +
                           ex.ToString());
                addError(project, file, TaskErrorCategory.Error, 0, 0,
                         "An exception was thrown when trying to start the Slice compiler\n" +
                         ex.ToString());
                return false;
            }
            catch(System.ComponentModel.Win32Exception ex)
            {
                Util.write(project, Util.msgLevel.msgError,
                           "An exception was thrown when trying to start the Slice compiler\n" + 
                           ex.ToString());
                addError(project, file, TaskErrorCategory.Error, 0, 0,
                         "An exception was thrown when trying to start the Slice compiler\n" +
                         ex.ToString());
                return false;
            }

            //
            // When StandardError and StandardOutput are redirected, at least one
            // should use asynchronous reads to prevent deadlocks when calling
            // process.WaitForExit; the other can be read synchronously using ReadToEnd.
            //
            // See the Remarks section in the below link:
            //
            // http://msdn.microsoft.com/en-us/library/system.diagnostics.process.standarderror.aspx
            //

            // Start the asynchronous read of the standard output stream.
            process.BeginOutputReadLine();
            // Read Standard error.
            string stderr = process.StandardError.ReadToEnd();
            process.WaitForExit();
            
            if(parseErrors(project, sliceCompiler, file, stderr))
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

            if(process.ExitCode != 0)
            {
                addError(project, file, TaskErrorCategory.Error, 0, 0, 
                         "Slice compiler `" + sliceCompiler +
                         "' failed to start (error code " + process.ExitCode.ToString() + ")");
                return false;
            }
            
            List<string> dependencies = new List<string>();
            StringReader output = new StringReader(reader.data());
            
            string line = null;

            DependenciesMap dependenciesMap = getDependenciesMap();
            if(!dependenciesMap.ContainsKey(project.FullName))
            {
                dependenciesMap[project.FullName] = new Dictionary<string, List<string>>();
            }

            Dictionary<string, List<string>> projectDeps = dependenciesMap[project.FullName];
            bool firstLine = true;
            while((line = output.ReadLine()) != null)
            {
                if(firstLine)
                {
                    Util.write(project, Util.msgLevel.msgDebug, "DEBUG Output: " + line + "\n");
                    firstLine = false;
                }
                else
                {
                    Util.write(project, Util.msgLevel.msgDebug, line + "\n");
                }

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

                    if(line.EndsWith(".ice", StringComparison.CurrentCultureIgnoreCase) &&
                       !System.IO.Path.GetFileName(line).Trim().Equals(System.IO.Path.GetFileName(file)))
                    {
                        line = line.Replace('/', '\\');
                        dependencies.Add(line);
                    }
                }
            }
            projectDeps[file] = dependencies;
            dependenciesMap[project.FullName] = projectDeps;
            process.Close();

            return true;
        }

        public void initDocumentEvents()
        {
            try
            {
                // Csharp project item events.
                _csProjectItemsEvents = 
                    (EnvDTE.ProjectItemsEvents)_applicationObject.Events.GetObject("CSharpProjectItemsEvents");
                if(_csProjectItemsEvents != null)
                {
                    _csProjectItemsEvents.ItemAdded +=
                        new _dispProjectItemsEvents_ItemAddedEventHandler(csharpItemAdded);
                    _csProjectItemsEvents.ItemRemoved +=
                        new _dispProjectItemsEvents_ItemRemovedEventHandler(csharpItemRemoved);
                }
            }
            catch(COMException)
            {
                // Can happen if the Visual Studio install don't support C#
            }

            try
            {
                // Cpp project item events.
                _vcProjectItemsEvents =
                    (VCProjectEngineEvents)_applicationObject.Events.GetObject("VCProjectEngineEventsObject");
                if (_vcProjectItemsEvents != null)
                {
                    _vcProjectItemsEvents.ItemAdded +=
                        new _dispVCProjectEngineEvents_ItemAddedEventHandler(cppItemAdded);
                    _vcProjectItemsEvents.ItemRemoved +=
                        new _dispVCProjectEngineEvents_ItemRemovedEventHandler(cppItemRemoved);
                }
            }
            catch(COMException)
            {
                // Can happen if the Visual Studio install don't support C++
            }
            // Visual Studio document events.
            _docEvents = _applicationObject.Events.get_DocumentEvents(null);
            if(_docEvents != null)
            {
                _docEvents.DocumentSaved += new _dispDocumentEvents_DocumentSavedEventHandler(documentSaved);
                _docEvents.DocumentOpened += new _dispDocumentEvents_DocumentOpenedEventHandler(documentOpened);
            }
        }

        public void removeDocumentEvents()
        {
            // Csharp project item events.
            if(_csProjectItemsEvents != null)
            {
                _csProjectItemsEvents.ItemAdded -= 
                    new _dispProjectItemsEvents_ItemAddedEventHandler(csharpItemAdded);
                _csProjectItemsEvents.ItemRemoved -= 
                    new _dispProjectItemsEvents_ItemRemovedEventHandler(csharpItemRemoved);
                _csProjectItemsEvents = null;
            }

            // Cpp project item events.
            if(_vcProjectItemsEvents != null)
            {
                _vcProjectItemsEvents.ItemAdded -= 
                    new _dispVCProjectEngineEvents_ItemAddedEventHandler(cppItemAdded);
                _vcProjectItemsEvents.ItemRemoved -=
                    new _dispVCProjectEngineEvents_ItemRemovedEventHandler(cppItemRemoved);
                _vcProjectItemsEvents = null;
            }

            // Visual Studio document events.
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

        public Project getStartupProject()
        {
            try
            {
                Array projects = (Array)_applicationObject.Solution.SolutionBuild.StartupProjects;
                Project p = Util.getProjectByNameOrFile(_applicationObject.Solution, projects.GetValue(0) as String);
                if(p != null)
                {
                    return p;
                }
            }
            catch (COMException)
            {
                //
                // Ignore could happen if called while solution is being initialized.
                //
            }

            try
            {
                if(_applicationObject.Solution.Projects != null && _applicationObject.Solution.Projects.Count > 0)
                {
                    return _applicationObject.Solution.Projects.Item(1) as Project;
                }
            }
            catch(COMException)
            {
                //
                // Ignore could happen if called while solution is being initialized.
                //
            }

            return null;
        }

        public Project getActiveProject()
        {
            Array projects = null;
            try
            {
                if(_applicationObject.ActiveSolutionProjects != null)
                {
                    projects = (Array)_applicationObject.ActiveSolutionProjects;
                    if(projects != null && projects.Length > 0)
                    {
                        return projects.GetValue(0) as Project;
                    }
                }
            }
            catch(COMException)
            {
                //
                // Ignore could happen if called while solution is being initialized.
                //
            }

            try
            {

                if(_applicationObject.Solution.Projects != null && _applicationObject.Solution.Projects.Count > 0)
                {
                    return _applicationObject.Solution.Projects.Item(1) as Project;
                }
            }
            catch(COMException)
            {
                //
                // Ignore could happen if called while solution is being initialized.
                //
            }

            return null;
        }
        
        private void removeDependency(Project project, String path)
        {
            DependenciesMap dependenciesMap = getDependenciesMap();
            if(dependenciesMap.ContainsKey(project.FullName))
            {
                if(dependenciesMap[project.FullName].ContainsKey(path))
                {
                    dependenciesMap[project.FullName].Remove(path);
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

                if(file.project == null)
                {
                    return;
                }

                if(!Util.isSliceFilename(file.Name))
                {
                    return;
                }

                Project project = Util.findProject((VCProject)file.project);

                if(project == null)
                {
                    return;
                }

                if(!Util.isSliceBuilderEnabled(project))
                {
                    return;
                }
                clearErrors(file.FullPath);
                if(!_deleted.Contains(file.FullPath))
                {
                    removeCppGeneratedItems(project, file.FullPath, true);
                }
                //
                // It appears that the file is not actually removed from disk at this
                // point. Thus we need to delay dependency updates until after delete,
                // or after the remove command has been executed.
                //
                _deletedFile = file.FullPath;
            }
            catch(Exception ex)
            {
                Util.unexpectedExceptionWarning(ex);
                throw;
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

                if(file.project == null)
                {
                    return;
                }

                if(!Util.isSliceFilename(file.Name))
                {
                    return;
                }

                Project project = Util.findProject((VCProject)file.project);
                if (project == null)
                {
                    return;
                }

                string fullPath = file.FullPath;

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
                    string cppPath = getCppGeneratedFileName(project, file.FullPath, Util.getSourceExt(project));
                    string hPath = Path.ChangeExtension(cppPath, "." + Util.getHeaderExt(project));
                    if(File.Exists(cppPath) || Util.hasItemNamed(project.ProjectItems, Path.GetFileName(cppPath)))
                    {
                        MessageBox.Show("A file named '" + Path.GetFileName(cppPath) +
                                        "' already exists.\n" + "If you want to add '" +
                                        Path.GetFileName(fullPath) + "' first remove " +
                                        " '" + Path.GetFileName(cppPath) + "' and '" +
                                        Path.GetFileName(hPath) + "'.",
                                        "Ice Visual Studio Add-in",
                                        MessageBoxButtons.OK,
                                        MessageBoxIcon.Error,
                                        MessageBoxDefaultButton.Button1,
                                        (MessageBoxOptions)0);
                        _deleted.Add(fullPath);
                        return;
                    }

                    if(File.Exists(hPath) || Util.hasItemNamed(project.ProjectItems, Path.GetFileName(hPath)))
                    {
                        MessageBox.Show("A file named '" + Path.GetFileName(hPath) +
                                        "' already exists.\n" + "If you want to add '" +
                                        Path.GetFileName(fullPath) + "' first remove " +
                                        " '" + Path.GetFileName(cppPath) + "' and '" +
                                        Path.GetFileName(hPath) + "'.",
                                        "Ice Visual Studio Add-in",
                                        MessageBoxButtons.OK,
                                        MessageBoxIcon.Error,
                                        MessageBoxDefaultButton.Button1,
                                        (MessageBoxOptions)0);
                        _deleted.Add(fullPath);
                        return;
                    }
                }

                clearErrors(project);
                buildProject(project, false, vsBuildScope.vsBuildScopeProject, false);
            }
            catch(Exception ex)
            {
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
        }

        private void csharpItemRemoved(ProjectItem item)
        {
            try
            {
                if(item == null)
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

                removeDependency(item.ContainingProject, fullName);
                clearErrors(item.ContainingProject);
                buildProject(item.ContainingProject, false, item, vsBuildScope.vsBuildScopeProject, false);
            }
            catch(Exception ex)
            {
                Util.unexpectedExceptionWarning(ex);
                throw;
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

                String csPath = getCSharpGeneratedFileName(project, item, "cs");
                ProjectItem csItem = Util.findItem(csPath, project.ProjectItems);

                if(File.Exists(csPath) || csItem != null)
                {
                    MessageBox.Show("A file named '" + Path.GetFileName(csPath) +
                                    "' already exists.\n" + "If you want to add '" +
                                    Path.GetFileName(fullPath) + "' first remove " +
                                    " '" + Path.GetFileName(csPath) + "'.",
                                    "Ice Visual Studio Add-in",
                                    MessageBoxButtons.OK,
                                    MessageBoxIcon.Error,
                                    MessageBoxDefaultButton.Button1,
                                    (MessageBoxOptions)0);
                    _deleted.Add(fullPath);
                    item.Remove();
                    return;
                }

                clearErrors(project);
                buildProject(project, false, vsBuildScope.vsBuildScopeProject, false);
            }
            catch(Exception ex)
            {
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
        }

        private static void removeCSharpGeneratedItems(ProjectItem item, bool remove)
        {
            if(item == null)
            {
                return;
            }

            if(String.IsNullOrEmpty(item.Name))
            {
                return;
            }

            if(!Util.isSliceFilename(item.Name))
            {
                return;
            }

            String generatedPath = getCSharpGeneratedFileName(item.ContainingProject, item, "cs");
            if(String.IsNullOrEmpty(generatedPath))
            {
                return;
            }

            FileInfo generatedFileInfo = new FileInfo(generatedPath);
            if(File.Exists(generatedFileInfo.FullName))
            {
                try
                {
                    File.Delete(generatedFileInfo.FullName);
                }
                catch(System.SystemException)
                {
                    // Can happen if the file is used by another process.
                }
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

        private static void removeCppGeneratedItems(ProjectItems items, bool remove)
        {
            List<ProjectItem> tmpItems = Util.clone(items);
            foreach(ProjectItem i in tmpItems)
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

            if(String.IsNullOrEmpty(item.Name))
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
            if(File.Exists(file))
            {
                try
                {
                    File.Delete(file);
                }
                catch(System.SystemException)
                {
                    // Can happen if the file is being used by another process.
                }
            }

            if(remove)
            {
                ProjectItem generated = Util.findItem(file, project.ProjectItems);
                if(generated != null)
                {
                    generated.Remove();
                }
            }
        }

        public static void removeCppGeneratedItems(Project project, String slice, bool remove)
        {
            FileInfo hFileInfo = new FileInfo(getCppGeneratedFileName(project, slice, Util.getHeaderExt(project)));
            FileInfo cppFileInfo = new FileInfo(Path.ChangeExtension(hFileInfo.FullName, Util.getSourceExt(project)));

            deleteProjectItem(project, hFileInfo.FullName, remove);
            deleteProjectItem(project, cppFileInfo.FullName, remove);
        }

        private bool runSliceCompiler(Project project, string sliceCompiler, string file, string outputDir)
        {
            if(!File.Exists(sliceCompiler))
            {
                Util.write(project, Util.msgLevel.msgError,
                           "'" + sliceCompiler + "' not found. Review 'Ice' installation.\n");
                addError(project, file, TaskErrorCategory.Error, 0, 0,
                         "'" + sliceCompiler + "' not found. Review 'Ice' installation.\n");
                return false;
            }

            string args = getSliceCompilerArgs(project, file, false);

            if(!String.IsNullOrEmpty(outputDir))
            {
                if(outputDir.EndsWith("\\", StringComparison.Ordinal))
                {
                    outputDir = outputDir.Replace("\\", "\\\\");
                }
                args += " --output-dir " + Util.quote(outputDir) + " ";
            }

            args += " " + Util.quote(file);

            System.Diagnostics.Process process = new System.Diagnostics.Process();
            process.StartInfo.FileName = sliceCompiler;
            process.StartInfo.Arguments = args;
            process.StartInfo.CreateNoWindow = true;
            process.StartInfo.UseShellExecute = false;
            process.StartInfo.RedirectStandardOutput = true;
            process.StartInfo.RedirectStandardError = true;
            process.StartInfo.WorkingDirectory = System.IO.Path.GetDirectoryName(project.FileName);

            StreamReader reader = new StreamReader();
            process.OutputDataReceived += new DataReceivedEventHandler(reader.appendData);

            Util.write(project, Util.msgLevel.msgDebug, "DEBUG Command-line: " + sliceCompiler + " " + args + "\n");

            if(!Directory.Exists(outputDir))
            {
                try
                {
                    Directory.CreateDirectory(outputDir);
                }
                catch(System.IO.IOException ex)
                {
                    Util.write(project, Util.msgLevel.msgError,
                               "An exception was thrown when trying to create the output directory.\n" +
                               ex.ToString());
                    addError(project, file, TaskErrorCategory.Error, 0, 0,
                             "An exception was thrown when trying to create the output directory.\n" + ex.ToString());
                    return false;
                }
            }

            try
            {
                process.Start();
            }
            catch(InvalidOperationException ex)
            {
                Util.write(project, Util.msgLevel.msgError,
                           "An exception was thrown when trying to start the Slice compiler\n" +
                           ex.ToString());
                addError(project, file, TaskErrorCategory.Error, 0, 0,
                         "An exception was thrown when trying to start the Slice compiler\n" +
                         ex.ToString());
                return false;
            }
            catch(System.ComponentModel.Win32Exception ex)
            {
                Util.write(project, Util.msgLevel.msgError,
                           "An exception was thrown when trying to start the Slice compiler\n" +
                           ex.ToString());
                addError(project, file, TaskErrorCategory.Error, 0, 0,
                         "An exception was thrown when trying to start the Slice compiler\n" +
                         ex.ToString());
                return false;
            }

            //
            // When StandardError and StandardOutput are redirected, at least one
            // should use asynchronous reads to prevent deadlocks when calling
            // process.WaitForExit; the other can be read synchronously using ReadToEnd.
            //
            // See the Remarks section in the below link:
            //
            // http://msdn.microsoft.com/en-us/library/system.diagnostics.process.standarderror.aspx
            //

            string stderr = process.StandardError.ReadToEnd();
            // Start the asynchronous read of the standard output stream.
            process.BeginOutputReadLine();

            process.WaitForExit();
    
            if(process.ExitCode != 0)
            {
                addError(project, file, TaskErrorCategory.Error, 0, 0, "Slice compiler `" + sliceCompiler + 
                                                        "' failed to start (error code " + process.ExitCode.ToString() + ")");
                return false;
            }
            bool hasErrors = parseErrors(project, sliceCompiler, file, stderr);
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

        private bool parseErrors(Project project, string sliceCompiler, string file, string stderr)
        {
            bool hasErrors = false;
            StringReader strer = new StringReader(stderr);
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
                    Util.write(project, Util.msgLevel.msgError, errorMessage + "\n");
                    addError(project, file, TaskErrorCategory.Error, 0, 0, errorMessage.Replace("error:", ""));
                    break;
                }
                int i = errorMessage.IndexOf(':');
                if(i == -1)
                {
                    if(firstLine)
                    {
                        errorMessage += strer.ReadToEnd();
                        Util.write(project, Util.msgLevel.msgError, errorMessage + "\n");
                        addError(project, "", TaskErrorCategory.Error, 1, 1, errorMessage);
                        hasErrors = true;
                        break;
                    }
                    errorMessage = strer.ReadLine();
                    continue;
                }
                Util.write(project, Util.msgLevel.msgError, errorMessage + "\n");

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
                catch(OverflowException)
                {
                    l = 0;
                }
                catch(FormatException)
                {
                    l = 0;
                }
                catch(ArgumentException)
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
                    bool currentFile = Util.equalPath(f, file, Path.GetDirectoryName(project.FileName));
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
                            addError(project, file, category, l, 1, errorMessage);
                        }
                        else
                        {
                            Util.write(project, Util.msgLevel.msgError,
                                "from file: " + f + "\n" + errorMessage + "\n");
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
                    Util.unexpectedExceptionWarning(ex);
                    throw;
                }
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
            if(action == vsBuildAction.vsBuildActionBuild || action == vsBuildAction.vsBuildActionRebuildAll)
            {
                //
                // Ensure slice compiler is only run once for parallel builds;
                // no need to lock, this is always called from main thread.
                //
                if(!_sliceBuild)
                {
                    _sliceBuild = true;
                }
                else
                {
                    return;
                }
            }

            try
            {
                string projectName = null;
                if(commandLine)
                {
                    projectName = Util.getCommandLineArgument("/Project");
                }
                _building = true;
                _buildScope = scope;
                Project project = null;
                if(String.IsNullOrEmpty(projectName))
                {
                    project = getSelectedProject();
                }
                else
                {
                    project = Util.getProjectByNameOrFile(_applicationObject.Solution, projectName);
                }
                
                List<Project> projects = new List<Project>();
                if(scope.Equals(vsBuildScope.vsBuildScopeProject))
                {
                    projects.Add(project);
                }
                else if(scope.Equals(vsBuildScope.vsBuildScopeSolution))
                {
                    projects = Util.buildOrder(_applicationObject.Solution);
                }
                else if(project != null && project.Kind.Equals(EnvDTE80.ProjectKinds.vsProjectKindSolutionFolder))
                {
                     projects = Util.solutionFolderProjects(project);
                }

                if(action == vsBuildAction.vsBuildActionBuild || action == vsBuildAction.vsBuildActionRebuildAll)
                {
                    foreach(Project p in projects)
                    {
                        _buildProject = p;
                        if(p == null)
                        {
                            continue;
                        }

                        clearErrors(p);
                        if(action == vsBuildAction.vsBuildActionRebuildAll)
                        {
                            cleanProject(p, false);
                        }
                        buildProject(p, false, scope, true);

                        if(hasErrors(p))
                        {
                            bringErrorsToFront();
                            Util.write(project, Util.msgLevel.msgError,
                                "------ Slice compilation contains errors. Build canceled. ------\n");
                            if(_connectMode == ext_ConnectMode.ext_cm_CommandLine)
                            {
                                // Is this the best we can do? Is there a clean way to exit?
                                Environment.Exit(-1);
                            }
                            _applicationObject.ExecuteCommand("Build.Cancel", "");
                        }
                    }
                }
                else if(action == vsBuildAction.vsBuildActionClean)
                {
                    foreach(Project p in projects)
                    {
                        if(p == null)
                        {
                            continue;
                        }
                        cleanProject(p, false);
                    }
                }
            }
            catch(Exception ex)
            {
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
            return;
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
                        Util.write(project, Util.msgLevel.msgError, ex.ToString() + "\n");
                        Util.unexpectedExceptionWarning(ex);
                        throw;
                    }
                }
            }
            return hierarchy;
        }
        
        //
        // Add an error to slice builder error list provider.
        //
        public void addError(Project project, string file, TaskErrorCategory category, int line, int column,
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
        // True if there were any errors in the last slice compilation.
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

        private const string buildOutputPaneGuid = "{1BD8A850-02D1-11d1-BEE7-00A0C913D1F8}";
        public OutputWindowPane buildOutput()
        {
            if(_output == null)
            {
                OutputWindow window = (OutputWindow)_applicationObject.Windows.Item(
                    EnvDTE.Constants.vsWindowKindOutput).Object;
                foreach(OutputWindowPane w in window.OutputWindowPanes)
                {
                    if(w.Guid.Equals(buildOutputPaneGuid, StringComparison.CurrentCultureIgnoreCase))
                    {
                        _output = w;
                        break;
                    }
                }
            }
            return _output;
        }

        public ext_ConnectMode connectMode()
        {
            return _connectMode;
        }

        //
        // Force the error list to show.
        //
        private void bringErrorsToFront()
        {
            if(commandLine || _errorListProvider == null)
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
            try
            {
                ErrorTask task = (ErrorTask)sender;
                task.Line += 1;
                _errorListProvider.Navigate(task, new Guid(EnvDTE.Constants.vsViewKindTextView));
                task.Line -= 1;
            }
            catch(Exception ex)
            {
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
        }

        private DependenciesMap
        getDependenciesMap()
        {
            if(_dependenciesMap == null)
            {
                _dependenciesMap = new DependenciesMap();
            }
            return _dependenciesMap;
        }

        private List<Project>
        getRebuildProjects()
        {
            if(_rebuildProjects == null)
            {
                _rebuildProjects = new List<Project>();
            }
            return _rebuildProjects;
        }

        private IVsTrackProjectDocuments2 GetTrackProjectDocuments()
        {
            // get IServiceProvider (OLE version, not .NET version) from DTE object
            IOleServiceProvider sp = (IOleServiceProvider)_applicationObject;

            // retrieve IVsTrackProjectDocuments2 interface via QueryService
            Guid guidSP = typeof(SVsTrackProjectDocuments).GUID;
            Guid guidIID = typeof(IVsTrackProjectDocuments2).GUID;
            object ptrUnknown;
            int rc = sp.QueryService(ref guidSP, ref guidIID, out ptrUnknown);
            if(ErrorHandler.Failed(rc))
            {
                try
                {
                    ErrorHandler.ThrowOnFailure(rc);
                }
                catch(Exception ex)
                {
                    Util.unexpectedExceptionWarning(ex);
                    throw;
                }
            }
            IVsTrackProjectDocuments2 vsTrackProjDocs = (IVsTrackProjectDocuments2)ptrUnknown;
            return vsTrackProjDocs;
        }

        public void beginTrackDocumentEvents()
        {
            int rc = GetTrackProjectDocuments().AdviseTrackProjectDocumentsEvents(this, out _dwCookie);
            if(ErrorHandler.Failed(rc))
            {
                try
                {
                    ErrorHandler.ThrowOnFailure(rc);
                }
                catch(Exception ex)
                {
                    Util.unexpectedExceptionWarning(ex);
                    throw;
                }
            }
        }

        public void endTrackDocumentEvents()
        {
            int rc = GetTrackProjectDocuments().UnadviseTrackProjectDocumentsEvents(_dwCookie);
            if(ErrorHandler.Failed(rc))
            {
                try
                {
                    ErrorHandler.ThrowOnFailure(rc);
                }
                catch(Exception ex)
                {
                    Util.unexpectedExceptionWarning(ex);
                    throw;
                }
            }
        }

#region IVsTrackProjectDocumentsEvents2 Members

        public int OnAfterAddDirectoriesEx(int cProjects, int cDirectories, IVsProject[] rgpProjects,
                                           int[] rgFirstIndices, string[] rgpszMkDocuments, 
                                           VSADDDIRECTORYFLAGS[] rgFlags)
        {
            return 0;
        }

        public int OnAfterAddFilesEx(int cProjects, int cFiles, IVsProject[] rgpProjects, int[] rgFirstIndices,
                                     string[] rgpszMkDocuments, VSADDFILEFLAGS[] rgFlags)
        {
            return 0;
        }

        public int OnAfterRemoveDirectories(int cProjects, int cDirectories, IVsProject[] rgpProjects,
                                            int[] rgFirstIndices, string[] rgpszMkDocuments,
                                            VSREMOVEDIRECTORYFLAGS[] rgFlags)
        {
            return 0;
        }

        public int OnAfterRemoveFiles(int cProjects, int cFiles, IVsProject[] rgpProjects, int[] rgFirstIndices,
                                      string[] rgpszMkDocuments, VSREMOVEFILEFLAGS[] rgFlags)
        {
            return 0;
        }

        public int OnAfterRenameDirectories(int cProjects, int cDirs, IVsProject[] rgpProjects, int[] rgFirstIndices,
                                            string[] rgszMkOldNames, string[] rgszMkNewNames,
                                            VSRENAMEDIRECTORYFLAGS[] rgFlags)
        {
            return 0;
        }

        public int OnAfterRenameFiles(int cProjects, int cFiles, IVsProject[] rgpProjects, int[] rgFirstIndices,
                                      string[] oldNames, string[] newNames, VSRENAMEFILEFLAGS[] rgFlags)
        {
            foreach(string newName in newNames)
            {
                if(!Util.isSliceFilename(newName))
                {
                    continue;
                }
                ProjectItem item = Util.findItem(newName);
                if(item == null)
                {
                    continue;
                }
                Project project = item.ContainingProject;
                if(project == null)
                {
                    continue;
                }
                if(!Util.isSliceBuilderEnabled(project))
                {
                    continue;
                }
                buildProject(project, false, vsBuildScope.vsBuildScopeProject, false);
            }
            return 0;
        }

        public int OnAfterSccStatusChanged(int cProjects, int cFiles, IVsProject[] rgpProjects, int[] rgFirstIndices,
                                           string[] rgpszMkDocuments, uint[] rgdwSccStatus)
        {
            return 0;
        }

        public int OnQueryAddDirectories(IVsProject pProject, int cDirectories, string[] rgpszMkDocuments,
                                         VSQUERYADDDIRECTORYFLAGS[] rgFlags, 
                                         VSQUERYADDDIRECTORYRESULTS[] pSummaryResult,
                                         VSQUERYADDDIRECTORYRESULTS[] rgResults)
        {
            return 0;
        }

        public int OnQueryAddFiles(IVsProject pProject, int cFiles, string[] rgpszMkDocuments, 
                                   VSQUERYADDFILEFLAGS[] rgFlags, VSQUERYADDFILERESULTS[] pSummaryResult, 
                                   VSQUERYADDFILERESULTS[] rgResults)
        {
            return 0;
        }

        public int OnQueryRemoveDirectories(IVsProject pProject, int cDirectories, string[] rgpszMkDocuments, 
                                            VSQUERYREMOVEDIRECTORYFLAGS[] rgFlags, 
                                            VSQUERYREMOVEDIRECTORYRESULTS[] pSummaryResult,
                                            VSQUERYREMOVEDIRECTORYRESULTS[] rgResults)
        {
            return 0;
        }

        public int OnQueryRemoveFiles(IVsProject pProject, int cFiles, string[] rgpszMkDocuments, 
                                      VSQUERYREMOVEFILEFLAGS[] rgFlags, 
                                      VSQUERYREMOVEFILERESULTS[] pSummaryResult,
                                      VSQUERYREMOVEFILERESULTS[] rgResults)
        {
            return 0;
        }

        public int OnQueryRenameDirectories(IVsProject pProject, int cDirs, string[] rgszMkOldNames, 
                                            string[] rgszMkNewNames, VSQUERYRENAMEDIRECTORYFLAGS[] rgFlags,
                                            VSQUERYRENAMEDIRECTORYRESULTS[] pSummaryResult, 
                                            VSQUERYRENAMEDIRECTORYRESULTS[] rgResults)
        {
            return 0;
        }

        public int OnQueryRenameFiles(IVsProject ivsProject, int cFiles, string[] oldNames, string[] newNames,
                                      VSQUERYRENAMEFILEFLAGS[] rgFlags, VSQUERYRENAMEFILERESULTS[] pSummaryResult,
                                      VSQUERYRENAMEFILERESULTS[] rgResults)
        {
            for(int i = 0; i < oldNames.Length; ++i)
            {
                string oldName = oldNames[i];
                string newName = newNames[i];
                
                if(String.IsNullOrEmpty(oldName) || String.IsNullOrEmpty(newName))
                {
                    continue;
                }

                if(!Util.isSliceFilename(oldName) )
                {
                    continue;
                }
                ProjectItem item = Util.findItem(oldName);
                if(item == null)
                {
                    continue;
                }
                if(!Util.isProjectItemFile(item))
                {
                    continue;
                }
                Project project = item.ContainingProject;
                if(project == null)
                {
                    continue;
                }
                try
                {
                    if(Util.isCSharpProject(project))
                    {
                        String csPath = getCSharpGeneratedFileName(project, newName, "cs");
                        if(File.Exists(csPath) || Util.findItem(csPath, project.ProjectItems) != null)
                        {
                            MessageBox.Show("A file named '" + Path.GetFileName(csPath) +
                                            "' already exists.\n" + oldName +
                                            " could not be renamed to '" + item.Name + "'.",
                                            "Ice Visual Studio Add-in",
                                            MessageBoxButtons.OK,
                                            MessageBoxIcon.Error,
                                            MessageBoxDefaultButton.Button1,
                                            (MessageBoxOptions)0);
                            return -1;
                        }

                        //
                        // Get rid of generated files, for the removed .ice file.
                        //
                        fileTracker().reap(project);

                        ProjectItem generatedItem = Util.findItem(getCSharpGeneratedFileName(project, item, "cs"), 
                                                                  project.ProjectItems);
                        if(generatedItem == null)
                        {
                            continue;
                        }
                        generatedItem.Delete();
                    }
                    else if(Util.isCppProject(project))
                    {
                        string cppPath = getCppGeneratedFileName(project, newName, "." + Util.getSourceExt(project));
                        string hPath = Path.ChangeExtension(cppPath, "." + Util.getHeaderExt(project));
                        if(File.Exists(cppPath) || Util.findItem(cppPath, project.ProjectItems) != null)
                        {
                            MessageBox.Show("A file named '" + Path.GetFileName(cppPath) + 
                                            "' already exists.\n" + "If you want to add '" + 
                                            Path.GetFileName(newName) + "' first remove " + " '" +
                                            Path.GetFileName(cppPath) + "' and '" +
                                            Path.GetFileName(hPath) + "' from your project.",
                                            "Ice Visual Studio Add-in",
                                            MessageBoxButtons.OK, MessageBoxIcon.Error, 
                                            MessageBoxDefaultButton.Button1, (MessageBoxOptions)0);
                            return -1;
                        }

                        if(File.Exists(hPath) || Util.findItem(hPath, project.ProjectItems) != null)
                        {
                            MessageBox.Show("A file named '" + Path.GetFileName(hPath) +
                                            "' already exists.\n" + "If you want to add '" +
                                            Path.GetFileName(newName) + "' first remove " +
                                            " '" + Path.GetFileName(cppPath) + "' and '" +
                                            Path.GetFileName(hPath) + "' from your project.",
                                            "Ice Visual Studio Add-in",
                                            MessageBoxButtons.OK, MessageBoxIcon.Error,
                                            MessageBoxDefaultButton.Button1, (MessageBoxOptions)0);
                            return -1;
                        }

                        //
                        // Get rid of generated files, for the removed .ice file.
                        //
                        fileTracker().reap(project);

                        string cppGeneratedPath = getCppGeneratedFileName(project, oldName, Util.getSourceExt(project));
                        string hGeneratedPath = Path.ChangeExtension(cppGeneratedPath, Util.getHeaderExt(project));

                        ProjectItem generatedItem = Util.findItem(cppGeneratedPath, project.ProjectItems);
                        if(generatedItem != null)
                        {
                            generatedItem.Delete();
                        }

                        generatedItem = Util.findItem(hGeneratedPath, project.ProjectItems);
                        if(generatedItem != null)
                        {
                            generatedItem.Delete();
                        }
                    }
                }
                catch(Exception ex)
                {
                    Util.write(null, Util.msgLevel.msgError, ex.ToString() + "\n");
                    Util.unexpectedExceptionWarning(ex);
                    throw;
                }
            }
            return 0;
        }

        #endregion

        private FileTracker fileTracker()
        {
            if(_fileTracker == null)
            {
                _fileTracker = new FileTracker();
            }
            return _fileTracker;
        }

        private DTE2 _applicationObject;
        private AddIn _addInInstance;
        private ext_ConnectMode _connectMode;
        private SolutionEvents _solutionEvents;
        private SelectionEvents _selectionEvents;
        private BuildEvents _buildEvents;
        private DocumentEvents _docEvents;
        private ProjectItemsEvents _csProjectItemsEvents;
        private VCProjectEngineEvents _vcProjectItemsEvents;
        private ServiceProvider _serviceProvider;

        private ErrorListProvider _errorListProvider;
        private List<ErrorTask> _errors;
        private int _errorCount;
        private FileTracker _fileTracker;
        private DependenciesMap _dependenciesMap;

        private string _deletedFile;
        private OutputWindowPane _output;

        private CommandEvents _addNewItemEvent;
        private CommandEvents _addExistingItemEvent;
        private CommandEvents _editRemoveEvent;
        private CommandEvents _excludeFromProjectEvent;
        private CommandEvents _editDeleteEvent;
        private CommandEvents _buildCancelEvent;
        private CommandEvents _debugStartEvent;
        private CommandEvents _debugStepIntoEvent;
        private CommandEvents _debugStepIntoNewInstance;
        private CommandEvents _debugStartWithoutDebuggingEvent;
        private CommandEvents _debugStartNewInstance;
        private List<String> _deleted = new List<String>();
        private List<Project> _rebuildProjects;
        private Command _iceConfigurationCmd;

        private string _excludedItem = null;

        //
        // The first of several parallel builds sets it to true in the "buildBegin" 
        // event handler, to ensure the slice compiler is run only once, then it's reset
        // to false when the build ends or is canceled.
        //
        private bool _sliceBuild = false;

        //
        // True if build is in process, false otherwise.
        //
        private bool _building = false;

        //
        // This contains the build scope of the last build command.
        //
        private vsBuildScope _buildScope = vsBuildScope.vsBuildScopeSolution;

        //
        // If build is in process and the build scope is "vsBuildScopeProject",
        // this contains a reference to the project, otherwise it's null.
        //
        private Project _buildProject;

        private uint _dwCookie;
        private bool _opening = false;
        private bool _opened = false; // True after solutionOpened has been executed.
        static public bool commandLine;
    }
}
