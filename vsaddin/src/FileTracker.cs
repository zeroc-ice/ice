// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.IO;
using System.Collections.Generic;
using EnvDTE;
using System.Runtime.InteropServices;

namespace Ice.VisualStudio
{
    public class FileTracker
    {
        public FileTracker()
        {
            _files = new Dictionary<string, Dictionary<string, List<string>>>();
        }

        public void clear()
        {
            _files.Clear();
        }

        public bool hasGeneratedFile(Project project, String generated)
        {
            if(project == null)
            {
                return false;
            }

            if(String.IsNullOrEmpty(project.FullName))
            {
                return false;
            }

            if (!_files.ContainsKey(project.FullName))
            {
                return false;
            }

            bool found = false;
            Dictionary<String, List<String>> projectFiles = _files[project.FullName];

            foreach(KeyValuePair<String, List<String>> k in projectFiles)
            {
                if(Path.GetFileName(k.Key).Equals(Path.GetFileName(Path.ChangeExtension(generated, ".ice")),
                                                  StringComparison.CurrentCultureIgnoreCase))
                {
                    foreach(String path in k.Value)
                    {
                        if(Util.equalPath(path, generated, Path.GetDirectoryName(project.FileName)))
                        {
                            found = true;
                            break;
                        }
                    }
                    if(found)
                    {
                        break;
                    }
                }
            }

            return found;
        }

        public void trackFile(Project project, String slice, String generated)
        {
            if(!_files.ContainsKey(project.FullName))
            {
                _files[project.FullName] = new Dictionary<string, List<string>>();
            }
            Dictionary<String, List<String>> _projectFiles = _files[project.FullName];
            if(!_projectFiles.ContainsKey(slice))
            {
                _projectFiles[slice] = new List<string>();
                _projectFiles[slice].Add(generated);
            }
            else if(!_projectFiles[slice].Contains(generated))
            {
                _projectFiles[slice].Add(generated);
            }
        }

        public void reap(Project project)
        {
            lock(this)
            {
                if(_reaping)
                {
                    return;
                }
                _reaping = true;
            }

            try
            {
                if(project == null)
                {
                    return;
                }

                if(!_files.ContainsKey(project.FullName))
                {
                    return;
                }

                Dictionary<string, List<string>> projectFiles = _files[project.FullName];
                List<String> removedSlice = new List<String>();
                foreach(KeyValuePair<string, List<string>> i in projectFiles)
                {
                    ProjectItem item = Util.findItem(i.Key, project.ProjectItems);
                    if(item == null) // Slice file no longer in the project.
                    {
                        removedSlice.Add(i.Key);

                        //
                        // Remove generated files corresponding to the Slice file.
                        //
                        List<String> removedFiles = new List<string>();
                        List<ProjectItem> generated = new List<ProjectItem>();
                        foreach(string f in i.Value)
                        {
                            removedFiles.Add(f);

                            ProjectItem generatedItem = Util.findItem(f, project.ProjectItems);
                            if(generatedItem == null)
                            {
                                continue;
                            }
                            generated.Add(generatedItem);
                        }

                        foreach(ProjectItem generatedItem in generated)
                        {
                            if(generatedItem == null)
                            {
                                continue;
                            }
                            generatedItem.Delete();
                        }
                        foreach(String f in removedFiles)
                        {
                            if(File.Exists(f))
                            {
                                try
                                {
                                    File.Delete(f);
                                }
                                catch(IOException)
                                {
                                }
                            }
                        }
                    }
                }

                foreach(String slice in removedSlice)
                {
                    if(String.IsNullOrEmpty(slice))
                    {
                        continue;
                    }
                    projectFiles.Remove(slice);
                }
            }
            catch(Exception ex)
            {
                Util.unexpectedExceptionWarning(ex);
                throw;
            }
            finally
            {
                lock(this)
                {
                    _reaping = false;
                }
            }
        }

        private Dictionary<string, Dictionary<string, List<string>>> _files;
        private bool _reaping;
    }
}
