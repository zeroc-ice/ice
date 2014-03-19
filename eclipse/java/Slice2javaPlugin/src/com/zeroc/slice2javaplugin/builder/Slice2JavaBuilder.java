// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This plug-in is provided to you under the terms and conditions
// of the Eclipse Public License Version 1.0 ("EPL"). A copy of
// the EPL is available at http://www.eclipse.org/legal/epl-v10.html.
//
// **********************************************************************

package com.zeroc.slice2javaplugin.builder;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.StringTokenizer;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IFolder;
import org.eclipse.core.resources.IMarker;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.IResourceDelta;
import org.eclipse.core.resources.IResourceDeltaVisitor;
import org.eclipse.core.resources.IResourceVisitor;
import org.eclipse.core.resources.IncrementalProjectBuilder;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IPath;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Path;
import org.eclipse.core.runtime.Status;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Display;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.console.ConsolePlugin;
import org.eclipse.ui.console.IConsole;
import org.eclipse.ui.console.IConsoleManager;
import org.eclipse.ui.console.MessageConsole;
import org.eclipse.ui.console.MessageConsoleStream;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

import com.zeroc.slice2javaplugin.Activator;
import com.zeroc.slice2javaplugin.internal.Configuration;
import com.zeroc.slice2javaplugin.internal.Dependencies;

public class Slice2JavaBuilder extends IncrementalProjectBuilder
{
    public static final String BUILDER_ID = "com.zeroc.Slice2JavaPlugin.Slice2JavaBuilder";
    
    /*
     * (non-Javadoc)
     * 
     * @see org.eclipse.core.internal.events.InternalBuilder#build(int,
     * java.util.Map, org.eclipse.core.runtime.IProgressMonitor)
     */
    protected IProject[] build(int kind, @SuppressWarnings("rawtypes")Map args, IProgressMonitor monitor)
        throws CoreException
    {
        long start = System.currentTimeMillis();
        
        IResourceDelta delta = getDelta(getProject());
        BuildState state = new BuildState(getProject(), delta, monitor);
        state.dependencies.read();

        try
        {
            if(kind == FULL_BUILD)
            {
                fullBuild(state, monitor);
            }
            else
            {
                if(delta == null)
                {
                    fullBuild(state, monitor);
                }
                else
                {
                    incrementalBuild(state, monitor);
                }
            }
        }
        finally
        {
            long end = System.currentTimeMillis();
            if(state.out != null)
            {
                state.out.println("Build complete. Elapsed time: " + (end - start) / 1000 + "s.");
            }
            state.dependencies.write();
        }
        return null;
    }

    protected void clean(IProgressMonitor monitor)
        throws CoreException
    {
        BuildState state = new BuildState(getProject(), null, monitor);
        
        // Don't read the existing dependencies. That will have the
        // effect of trashing them.
        
        try
        {
            // Now, clean the generated sub-directory.
            Set<IFile> files = new HashSet<IFile>();
            getResources(files, state.generated.members());
            
            for(IFile file : files)
            {
                // Don't delete "." files (such as .gitignore).
                if(!file.getName().startsWith("."))
                {
                    file.delete(true, false, monitor);
                }
            }
        }
        finally
        {
            state.dependencies.write();
        }
    }
    
    static class StreamReaderThread extends Thread
    {
        public StreamReaderThread(InputStream in, StringBuffer out)
        {
            _in = new BufferedReader(new InputStreamReader(in), 1024);
            _out = out;
        }

        public void run()
        {
            try
            {
                char[] buf = new char[1024];
                while(true)
                {
                    int read = _in.read(buf);
                    if(read == -1)
                    {
                        break;
                    }
                    _out.append(buf, 0, read);
                }
            }
            catch(Exception e)
            {
            }
            finally
            {
                try
                {
                    _in.close();
                }
                catch(IOException e1)
                {
                    e1.printStackTrace();
                }
            }
        }

        private StringBuffer _out;
        private BufferedReader _in;
    }
    
    static class BuildState
    {
        BuildState(IProject project, IResourceDelta delta, IProgressMonitor monitor) throws CoreException
        {
            config = new Configuration(project);
            
            if(config.getConsole())
            {
                initializeConsole();
                out = _consoleout;
                err = _consoleerr;
            }
            
            generated = project.getFolder(config.getGeneratedDir());
            if(!generated.exists())
            {
                generated.create(false, true, monitor);
            }

            _sourceLocations = new HashSet<IFolder>();
            for(Iterator<String> p = config.getSliceSourceDirs().iterator(); p.hasNext();)
            {
                _sourceLocations.add(project.getFolder(p.next()));
            }
            
            project.accept(new IResourceVisitor()
            {
                public boolean visit(IResource resource)
                    throws CoreException
                {
                    if(resource instanceof IFile)
                    {
                        IFile file = (IFile) resource;
                        if(filter(file))
                        {
                            _resources.add((IFile) resource);
                        }
                    }
                    return true;
                }
            });

            if(delta != null)
            {
                delta.accept(new IResourceDeltaVisitor()
                {
                    public boolean visit(IResourceDelta delta)
                        throws CoreException
                    {
                        IResource resource = delta.getResource();
                        if(resource instanceof IFile)
                        {
                            IFile file = (IFile) resource;
                            if(filter(file))
                            {
                                switch (delta.getKind())
                                {
                                case IResourceDelta.ADDED:
                                case IResourceDelta.CHANGED:
                                    _deltaCandidates.add(file);
                                    break;
                                case IResourceDelta.REMOVED:
                                    _removed.add(file);
                                    break;
                                }
                            }
                        }
                        return true;
                    }
                });
            }

            dependencies = new Dependencies(project, _resources, err);
        }
        
        public Set<IFile> deltas() 
        {
            return _deltaCandidates;
        }
        
        public List<IFile> removed()
        {
            return _removed;
        }
        
        public Set<IFile> resources()
        {
            return _resources;
        }

        public boolean filter(IFile file)
        {
            String ext = file.getFileExtension();
            if(ext != null && ext.equals("ice"))
            {
                //
                // The parent may not be an IFolder (e.g., it could be a Project).
                //
                if(file.getParent() instanceof IFolder)
                {
                    IFolder folder = (IFolder)file.getParent();
                    if(_sourceLocations.contains(folder))
                    {
                        return true;
                    }
                }
            }
            return false;
        }
        
        synchronized static private void initializeConsole()
        {
            if(_consoleout == null)
            {
                MessageConsole console = new MessageConsole("slice2java", null);
                IConsole[] ics = new IConsole[1];
                ics[0] = console;
                IConsoleManager csmg = ConsolePlugin.getDefault().getConsoleManager();
                csmg.addConsoles(ics);
                csmg.showConsoleView(console);
    
                _consoleout = console.newMessageStream();
                _consoleerr = console.newMessageStream();
    
                final Display display = PlatformUI.getWorkbench().getDisplay();
                display.syncExec(new Runnable() {
                    public void run() {
                        _consoleerr.setColor(display.getSystemColor(SWT.COLOR_RED));
                    }
                });
            }
        }

        Configuration config;
        Dependencies dependencies;
        IFolder generated;
        private Set<IFolder> _sourceLocations;
        
        private Set<IFile> _resources = new HashSet<IFile>();
        private Set<IFile> _deltaCandidates = new HashSet<IFile>();
        private List<IFile> _removed = new ArrayList<IFile>();
        
        private MessageConsoleStream out = null;
        private MessageConsoleStream err = null;
 
        static private MessageConsoleStream _consoleout = null;
        static private MessageConsoleStream _consoleerr = null;
    }
    
    private int build(BuildState state, Set<IFile> files, boolean depend, StringBuffer out, StringBuffer err)
        throws CoreException
    {
        // Clear the output buffer.
        out.setLength(0);
        if(err != null)
        {
            err.setLength(0);
        }

        List<String> cmd = new LinkedList<String>();
        String translator = state.config.getTranslator();
        if(translator == null)
        {
            throw new CoreException(new Status(IStatus.ERROR, Activator.PLUGIN_ID, "Cannot locate slice2java translator: please fix Ice install location", null));
        }
        
        cmd.add(translator);
        if(depend)
        {
            cmd.add("--depend-xml");
        }
        else
        {
            cmd.add("--output-dir=" + state.generated.getProjectRelativePath().toString());
            cmd.add("--list-generated");
        }
        List<String> cmdBase = new LinkedList<String>();
        cmdBase.addAll(cmd);
        
        cmd.addAll(state.config.getCommandLine());
        
        Set<IFile> resourcesWithArguments = new  HashSet<IFile>();
        
        boolean allHasOptions = true;
        for(Iterator<IFile> p = files.iterator(); p.hasNext();)
        {
            IFile f = p.next();
            if(!Configuration.resourceHasOptions(f))
            {
                allHasOptions = false;
                cmd.add(f.getLocation().toOSString());
            }
            else
            {
                resourcesWithArguments.add(f);
            }
        }
       
        ProcessBuilder builder;
        IPath rootLocation = getProject().getLocation();
        Map<String, String> env;
        int status = 0;
        
        if(!allHasOptions)
        {
            builder = new ProcessBuilder(cmd);
            if(err == null)
            {
                builder.redirectErrorStream(true);
            }
                
            builder.directory(rootLocation.toFile());
            env = builder.environment();
            Configuration.setupSharedLibraryPath(env);
    
            status = runSliceCompiler(builder, state, out, err);
        }
        
        for(Iterator<IFile> p = resourcesWithArguments.iterator(); p.hasNext();)
        {
            IFile f = p.next();
            cmd = new LinkedList<String>();
            cmd.addAll(cmdBase);
            cmd.addAll(state.config.getCommandLine(f));
            
            cmd.add(f.getLocation().toOSString());
            
            builder = new ProcessBuilder(cmd);
            if(err == null)
            {
                builder.redirectErrorStream(true);
            }
            builder.directory(rootLocation.toFile());
            env = builder.environment();
            Configuration.setupSharedLibraryPath(env);
            
            status = runSliceCompiler(builder, state, out, err);
        }
        
        return status;
    }
    
    private int
    runSliceCompiler(ProcessBuilder builder, BuildState state, StringBuffer out, StringBuffer err)
        throws CoreException
    {
        try
        {    
            if(state.out != null)
            {
                for(Iterator<String> p = builder.command().iterator(); p.hasNext();)
                {
                    state.out.print(p.next());
                    state.out.print(" ");
                }
                state.out.println("");
            }
            
            Process proc = builder.start();

            StreamReaderThread outThread = new StreamReaderThread(proc.getInputStream(), out);
            outThread.start();
            StreamReaderThread errThread = null;
            if(err != null)
            {
                errThread = new StreamReaderThread(proc.getErrorStream(), err);
                errThread.start();
            }
            
            int status = proc.waitFor();

            outThread.join();
            if(errThread != null)
            {
                errThread.join();
            }
            
            if(status != 0)
            {
                throw new RuntimeException(err == null ? out.toString() : err.toString());
            }

            return status;
        }
        catch(Exception e)
        {
            throw new CoreException(new Status(IStatus.ERROR, Activator.PLUGIN_ID, e.toString(), null));
        }
    }

    private void
    createMarker(BuildState state, IFile source, IPath filename, int line, String msg)
        throws CoreException
    {
        // Process the error.
        IPath dir = getProject().getLocation();
        
        IFile file = null;
        if(filename != null && dir.isPrefixOf(filename))
        {
            // Locate the file within the project.
            file = getProject().getFile(filename.removeFirstSegments(dir.segmentCount()));

            // If the file is not the current source file, and the file exists in the project
            // then it must already contain a marker, so don't place another.
            if(!file.equals(source) && state.filter(file))
            {
                return;
            }
        }
        
        // If the message isn't contained in the source file, then identify the
        // file:line in the message itself.
        if(file == null)
        {
            if(line != -1)
            {
                msg = filename + ":" + line + ": " + msg;
            }
            else
            {
                msg = filename + ": " + msg;
            }
        }
        
        IMarker marker = source.createMarker(IMarker.PROBLEM);
        marker.setAttribute(IMarker.MESSAGE, msg);
        if(msg.toLowerCase().indexOf("warning:") >= 0)
        {
            marker.setAttribute(IMarker.SEVERITY, IMarker.SEVERITY_WARNING);
        }
        else
        {
            marker.setAttribute(IMarker.SEVERITY, IMarker.SEVERITY_ERROR);
        }
        if(line != -1)
        {
            if(file != null && file.equals(source))
            {
                marker.setAttribute(IMarker.LINE_NUMBER, line);
            }
            else
            {
                marker.setAttribute(IMarker.LINE_NUMBER, 1);
            }
        }
    }
    
    private void createMarkers(BuildState state, IFile source, String output)
        throws CoreException
    {
        output = output.trim();
        if(output.length() == 0)
        {
            return;
        }
        
        String[] lines = output.split("\n");

        IPath filename = null;
        int line = -1;
        StringBuffer msg = new StringBuffer();
        
        boolean continuation = false;

        for(int i = 0; i < lines.length; ++i)
        {
            if(continuation)
            {
                if(lines[i].startsWith(" "))
                {
                    // Continuation of the previous message.
                    msg.append(lines[i]);
                    continue;
                }
                else
                {
                    // Process the message.
                    createMarker(state, source, filename, line, msg.toString());
                }
            }
            
            // We're on a new message.
            msg.setLength(0);
            continuation = false;
            
            // Ignore.
            if(lines[i].contains("errors in preprocessor") || lines[i].contains("error in preprocessor"))
            {
                continue;
            }
            
            //
            // Parse a line of the form:
            //
            // file:[line:] message
            //
            int start = 0;
            int end;
            // Handle drive letters.
            if(lines[i].length() > 2 && lines[i].charAt(1) == ':')
            {
                end = lines[i].indexOf(':', 2);
            }
            else
            {
                end = lines[i].indexOf(':');
            }
            if(end != -1)
            {
                filename = new Path(lines[i].substring(start, end));
                start = end + 1;
                end = lines[i].indexOf(':', start);
                if(end != -1)
                {
                    try
                    {
                        line = Integer.parseInt(lines[i].substring(start, end));
                        start = end + 1;
                    }
                    catch(NumberFormatException e)
                    {
                        // The message may not have a line number.
                        line = -1;
                    }
                    msg.append(lines[i].substring(start, lines[i].length()));

                    continuation = true;
                    continue;
                }
            }
            // Unknown format.
            createMarker(state, source, null, -1, lines[i]);
        }

        if(continuation)
        {
            createMarker(state, source, filename, line, msg.toString());
        }
    }
    
    private void getResources(Set<IFile> files, IResource[] members)
        throws CoreException
    {
        for(int i = 0; i < members.length; ++i)
        {
            if(members[i] instanceof IFile)
            {
                files.add((IFile) members[i]);
            }
            else if(members[i] instanceof IFolder)
            {
                getResources(files, ((IFolder) members[i]).members());
            }
        }
    }

    private void fullBuild(BuildState state, final IProgressMonitor monitor)
        throws CoreException
    {
        clean(monitor);
        Set<IFile> candidates = state.resources();
        if(candidates.isEmpty())
        {
            return;
        }

        if(state.out != null)
        {
            java.util.Date date = new java.util.Date();
            state.out.println("Start full build at " + new SimpleDateFormat("HH:mm:ss").format(date));

            state.out.println("Candidate list:");
            // This is a complete list of Slice files.
            for(Iterator<IFile> p = candidates.iterator(); p.hasNext();)
            {
                state.out.println("    " + p.next().getProjectRelativePath().toString());
            }
            state.out.println("Regenerating java source files.");
        }
        
        StringBuffer out = new StringBuffer();

        Set<IFile> depends = new HashSet<IFile>();
        
        // Delete each marker.
        for(Iterator<IFile> p = candidates.iterator(); p.hasNext();)
        {
            IFile file = p.next();
            file.deleteMarkers(IMarker.PROBLEM, true, IResource.DEPTH_INFINITE);
        }
            
        // Do the build.
        build(state, candidates, false, out, null);

        out = mergeXmls(out, false);
        
        // Refresh the generated subdirectory prior to processing the
        // generated files list.
        state.generated.refreshLocal(IResource.DEPTH_INFINITE, monitor);

        // Parse the output.
        Slice2JavaGeneratedParser parser = getGeneratedFiles(state, candidates, out);
        for(Map.Entry<IFile, Slice2JavaGeneratedParser.Entry> entry : parser.output.entrySet())
        {
            IFile source = entry.getKey();

            Slice2JavaGeneratedParser.Entry outputEntry = entry.getValue();
            Set<IFile> newGeneratedJavaFiles = outputEntry.files;

            for(IFile f : newGeneratedJavaFiles)
            {
                // Mark the resource as derived.
                f.setDerived(true, null);
            }

            if(!outputEntry.error)
            {
                depends.add(source);
                if(state.out != null)
                {
                    if(newGeneratedJavaFiles.isEmpty())
                    {
                        state.out.println(source.getProjectRelativePath().toString() + ": No java files emitted.");
                    }
                    else
                    {
                        state.out.println(source.getProjectRelativePath().toString() + ": Emitted:");
                        for(Iterator<IFile> q = newGeneratedJavaFiles.iterator(); q.hasNext();)
                        {
                            state.out.println("    " + q.next().getProjectRelativePath().toString());
                        }
                    }
                }
            }
            else
            {
                state.dependencies.errorSliceFiles.add(source);
                if(state.out != null)
                {
                    state.out.println(source.getProjectRelativePath().toString() + ": Error.");
                }
            }

            // Update the set of slice -> java dependencies.
            state.dependencies.sliceJavaDependencies.put(source, newGeneratedJavaFiles);
            
            // Create markers for each warning/error.
            createMarkers(state, source, outputEntry.output);
        }
        
        // Update the slice->slice dependencies.
        // Only update the dependencies for those files with no build problems.
        if(!depends.isEmpty())
        {
            if(state.out != null)
            {
                state.out.println("Updating dependencies.");
            }

            StringBuffer err = new StringBuffer();
            if(build(state, depends, true, out, err) == 0)
            {
                out = mergeXmls(out, true);
                // Parse the new dependency set.
                state.dependencies.updateDependencies(out.toString());
            }
            else if(state.err != null)
            {
                state.err.println("Dependencies not updated due to error.");
                state.err.println(err.toString());    
            }
        }
    }

    private void incrementalBuild(BuildState state, IProgressMonitor monitor)
        throws CoreException
    {
        Set<IFile> candidates = state.deltas();
        List<IFile> removed = state.removed();
        
        if(state.out != null)
        {
            java.util.Date date = new java.util.Date();
            state.out.println("Start incremental build at " + new SimpleDateFormat("HH:mm:ss").format(date));
            
            state.out.println("Candidate list:");
            // This is a complete list of slice files.
            for(Iterator<IFile> p = candidates.iterator(); p.hasNext();)
            {
                state.out.println("   + " + p.next().getProjectRelativePath().toString());
            }
            for(Iterator<IFile> p = removed.iterator(); p.hasNext();)
            {
                state.out.println("   - " + p.next().getProjectRelativePath().toString());
            }
        }
        
        // The orphan candidate set.
        Set<IFile> orphanCandidateSet = new HashSet<IFile>();
        
        // Go through the removed list, removing the dependencies.
        for(Iterator<IFile> p = removed.iterator(); p.hasNext();)
        {
            IFile f = p.next();
            
            // Remove the file from the error list, if necessary.
            if(state.dependencies.errorSliceFiles.contains(f))
            {
                state.dependencies.errorSliceFiles.remove(f);
            }
            
            Set<IFile> dependents = state.dependencies.sliceSliceDependencies.remove(f);
            if(dependents != null)
            {
                Iterator<IFile> dependentsIterator = dependents.iterator();
                while(dependentsIterator.hasNext())
                {
                    IFile dependent = dependentsIterator.next();
                    Set<IFile> files = state.dependencies.reverseSliceSliceDependencies.get(dependent);
                    if(files != null)
                    {
                        files.remove(f);
                    }
                }
            }
            
            Set<IFile> oldJavaFiles = state.dependencies.sliceJavaDependencies.remove(f);
            if(state.out != null)
            {
                if(oldJavaFiles == null || oldJavaFiles.isEmpty())
                {
                    state.out.println(f.getProjectRelativePath().toString() + ": No orphans.");
                }
                else
                {
                    state.out.println(f.getProjectRelativePath().toString() + ": Orphans:");
                    for(Iterator<IFile> q = oldJavaFiles.iterator(); q.hasNext();)
                    {
                        state.out.println("    " + q.next().getProjectRelativePath().toString());
                    }
                }
            }

            if(oldJavaFiles != null)
            {
                orphanCandidateSet.addAll(oldJavaFiles);
            }
        }

        // Add the removed files to the candidates set
        // prior to determining additional candidates.
        candidates.addAll(removed);
        
        // Add to the candidate set any slice files that are in error. Clear the
        // error list.
        candidates.addAll(state.dependencies.errorSliceFiles);
        state.dependencies.errorSliceFiles.clear();

        Set<IFile> candidatesTmp = new HashSet<IFile>();

        for(Iterator<IFile> p = candidates.iterator(); p.hasNext();)
        {
            IFile f = p.next();
            
            Set<IFile> files = state.dependencies.reverseSliceSliceDependencies.get(f);
            if(files != null)
            {
                for(Iterator<IFile> q = files.iterator(); q.hasNext();)
                {
                    IFile potentialCandidate = q.next();
                    if(potentialCandidate.exists())
                    {
                        candidatesTmp.add(potentialCandidate);
                    }
                }
            }

            // If this is a file in the contained list, then remove the
            // dependency entry.
            if(removed.contains(f))
            {
                state.dependencies.reverseSliceSliceDependencies.remove(f);
            }
        }
        candidates.addAll(candidatesTmp);

        // Remove all the removed files from the candidates list.
        candidates.removeAll(removed);

        if(state.out != null)
        {
            if(candidates.isEmpty())
            {
                state.out.println("No remaining candidates.");
            }
            else
            {
                state.out.println("Expanded candidate list:");
                // This is a complete list of slice files.
                for(Iterator<IFile> p = candidates.iterator(); p.hasNext();)
                {
                    state.out.println("    " + p.next().getProjectRelativePath().toString());
                }
            }
        }

        StringBuffer out = new StringBuffer();

        // The set of files that we'll generate dependencies for.
        Set<IFile> depends = new HashSet<IFile>();
        
        if(!candidates.isEmpty())
        {
            if(state.out != null)
            {
                state.out.println("Regenerating java source files.");
            }

            // The complete set of generated java files by this build.
            Set<IFile> generatedJavaFiles = new HashSet<IFile>();

            // Remove all markers for the candidate list.
            for(Iterator<IFile> p = candidates.iterator(); p.hasNext();)
            {
                IFile file = p.next();
                file.deleteMarkers(IMarker.PROBLEM, true, IResource.DEPTH_INFINITE);
            }
            
            // Do the build.
            build(state, candidates, false, out, null);
            out = mergeXmls(out, false);
            
            // Refresh the generated directory prior to processing the generated
            // files list.
            state.generated.refreshLocal(IResource.DEPTH_INFINITE, monitor);
    
            // Parse the emitted XML file that describes what was produced by
            // the build.
            Slice2JavaGeneratedParser parser = getGeneratedFiles(state, candidates, out);
            for(Map.Entry<IFile, Slice2JavaGeneratedParser.Entry> entry : parser.output.entrySet())
            {
                IFile source = entry.getKey();
    
                Slice2JavaGeneratedParser.Entry outputEntry = entry.getValue();
                
                Set<IFile> newGeneratedJavaFiles = outputEntry.files;
                for(IFile f : newGeneratedJavaFiles)
                {
                    // Mark the resource as derived.
                    f.setDerived(true, null);
                }

                // If the build of the file didn't result in an error, add to
                // the dependencies list. Otherwise, add to the error list.
                if(!outputEntry.error)
                {
                    depends.add(source);
                }
                else
                {
                    if(state.out != null)
                    {
                        state.out.println(source.getProjectRelativePath().toString() + ": Error.");
                    }
                    state.dependencies.errorSliceFiles.add(source);
                }
    
                // Compute the set difference between the old set and new set
                // of generated files. The difference should be added to the
                // orphan candidate set.
                Set<IFile> oldJavaFiles = state.dependencies.sliceJavaDependencies.get(source);
                if(oldJavaFiles != null)
                {
                    // Compute the set difference.
                    oldJavaFiles.removeAll(newGeneratedJavaFiles);
                    if(state.out != null)
                    {
                        if(oldJavaFiles.isEmpty())
                        {
                            state.out.println(source.getProjectRelativePath().toString() + ": No orphans.");
                        }
                        else
                        {
                            state.out.println(source.getProjectRelativePath().toString() + ": Orphans:");
                            for(Iterator<IFile> q = oldJavaFiles.iterator(); q.hasNext();)
                            {
                                state.out.println("    " + q.next().getProjectRelativePath().toString());
                            }
                        }
                    }
                    orphanCandidateSet.addAll(oldJavaFiles);
                }
    
                // Update the set of slice -> java dependencies.
                state.dependencies.sliceJavaDependencies.put(source, newGeneratedJavaFiles);
    
                // If the build resulted in an error, there will be no java source files.
                if(state.out != null && !outputEntry.error)
                {
                    if(newGeneratedJavaFiles.isEmpty())
                    {
                        state.out.println(source.getProjectRelativePath().toString() + ": No java files emitted.");
                    }
                    else
                    {
                        state.out.println(source.getProjectRelativePath().toString() + ": Emitted:");
                        for(Iterator<IFile> q = newGeneratedJavaFiles.iterator(); q.hasNext();)
                        {
                            state.out.println("    " + q.next().getProjectRelativePath().toString());
                        }
                    }
                }
    
                generatedJavaFiles.addAll(newGeneratedJavaFiles);
                
                // Create markers for each warning/error.
                createMarkers(state, source, outputEntry.output);
            }
            
            // Do a set difference between the orphan candidate set
            // and the complete set of generated java source files.
            // Any remaining are complete orphans and should
            // be removed.
            orphanCandidateSet.removeAll(generatedJavaFiles);
        }

        if(state.out != null)
        {
            if(orphanCandidateSet.isEmpty())
            {
                state.out.println("No orphans from this build.");
            }
            else
            {
                state.out.println("Orphans from this build:");
                for(Iterator<IFile> p = orphanCandidateSet.iterator(); p.hasNext();)
                {
                    state.out.println("    " + p.next().getProjectRelativePath().toString());
                }
            }
        }

        //
        // Remove orphans.
        //
        for(Iterator<IFile> p = orphanCandidateSet.iterator(); p.hasNext();)
        {
            p.next().delete(true, false, monitor);
        }

        // The dependencies of any files without build errors should be updated.
        if(!depends.isEmpty())
        {
            if(state.out != null)
            {
                state.out.println("Updating dependencies.");
            }

            StringBuffer err = new StringBuffer();

            // We've already added markers for any errors... Only update the
            // dependencies if no problems resulted in the build.
            if(build(state, depends, true, out, err) == 0)
            {
                out = mergeXmls(out, true);
                // Parse the new dependency set.
                state.dependencies.updateDependencies(out.toString());
            }
            else if(state.err != null)
            {
                state.err.println("Dependencies not updated due to error.");
                state.err.println(err.toString());    
            }
        }
    }
    
    //
    // This method merge the XML produced by multiple Slice translator
    // invocations in a single XML. If depend argument is true, the input
    // buffer is treated as a dependencies XML, otherwise is treated as
    // a generated list XML.
    //
    private StringBuffer
    mergeXmls(StringBuffer input, boolean depend)
    {
        //
        // Merge depend XMLs in a single XML
        //
        String v = input.toString();
        StringTokenizer lines = new StringTokenizer(v, System.getProperty("line.separator"));
        boolean firstLine = true;
        boolean firstGenerated = true;
        StringBuffer out = new StringBuffer();
        while(lines.hasMoreTokens())
        {
            String line = lines.nextToken();
            if(line.startsWith("<?xml version=\"1.0\" encoding=\"UTF-8\"?>") &&
            !firstLine)
            {
                continue;
            }

            if(depend)
            {
                if(line.equals("<dependencies>"))
                {
                    if(firstGenerated)
                    {
                        firstGenerated = false;
                    }
                    else
                    {
                        continue;
                    }
                }
                else if(line.equals("</dependencies>") && lines.hasMoreTokens())
                {
                    continue;
                }
            }
            else
            {
                if(line.equals("<generated>"))
                {
                    if(firstGenerated)
                    {
                        firstGenerated = false;
                    }
                    else
                    {
                        continue;
                    }
                }
                else if(line.equals("</generated>") && lines.hasMoreTokens())
                {
                    continue;
                }
            }

            out.append(line + "\n");
            firstLine = false;
        }
        return out;
    }

    private static class Slice2JavaGeneratedParser
    {
        static class Entry
        {
            boolean error; // Did the build result in an error.
            String output; // Any warnings/errors from the build.
            Set<IFile> files; // The set of java source files associated with the source file.
        }
        Map<IFile, Entry> output = new HashMap<IFile, Entry>(); // Map of source files to build entry.

        private IFolder _generated;
        private IPath _generatedPath;
        // Map of absolute path to project location.
        private Map<IPath, IFile> _sources = new HashMap<IPath, IFile>();
        
        Slice2JavaGeneratedParser(IFolder generated, Set<IFile> candidates)
        {
            _generated = generated;
            _generatedPath = generated.getProjectRelativePath();
            for(IFile f : candidates)
            {
                _sources.put(f.getLocation(), f);
            }
        }

        private Node findNode(Node n, String qName)
            throws SAXException
        {
            NodeList children = n.getChildNodes();
            for(int i = 0; i < children.getLength(); ++i)
            {
                Node child = children.item(i);
                if(child.getNodeType() == Node.ELEMENT_NODE && child.getNodeName().equals(qName))
                {
                    return child;
                }
            }
            throw new SAXException("no such node: " + qName);
        }
        
        private IFile convert(String fname)
        {
            IPath p = new Path(fname); // fname contains "generated/...".
            int match = p.matchingFirstSegments(_generatedPath);
            return _generated.getFile(p.removeFirstSegments(match));
        }            

        public Set<IFile> visitSource(Node source) throws SAXException
        {
            Set<IFile> files = new HashSet<IFile>();
            NodeList sourceNodes = source.getChildNodes();
            for(int j = 0; j < sourceNodes.getLength(); ++j)
            {
                if(sourceNodes.item(j).getNodeType() == Node.ELEMENT_NODE && sourceNodes.item(j).getNodeName().equals("file"))
                {
                    Element file = (Element)sourceNodes.item(j);
                    String name = file.getAttribute("name");
                    if(name.length() == 0)
                    {
                        throw new SAXException("empty name attribute");
                    }
                    files.add(convert(name));
                }
            }
            return files;
        }
        
        private String getText(Node n) throws SAXException
        {
            NodeList children = n.getChildNodes();
            if(children.getLength() == 1 && children.item(0).getNodeType() == Node.TEXT_NODE)
            {
                return children.item(0).getNodeValue();
            }
            return "";
        }
        
        public void visit(Node doc) throws SAXException
        {
            Node n = findNode(doc, "generated");
            NodeList fileNodes = n.getChildNodes();
            for(int j = 0; j < fileNodes.getLength(); ++j)
            {
                if(fileNodes.item(j).getNodeType() == Node.ELEMENT_NODE && fileNodes.item(j).getNodeName().equals("source"))
                {
                    Element sourceElement = (Element)fileNodes.item(j);
                    String name = sourceElement.getAttribute("name");
                    if(name.length() == 0)
                    {
                        throw new SAXException("empty name attribute");
                    }
                    
                    // The source file 
                    IFile source = _sources.get(new Path(name));
                    if(source == null)
                    {
                        throw new SAXException("unknown source file: " + name);
                    }

                    Entry e = new Entry();
                    e.error = true;
                    e.output = getText(findNode(sourceElement, "output"));

                    String error = sourceElement.getAttribute("error");
                    if(error.equals("true"))
                    {
                        e.error = true;
                        e.files = new HashSet<IFile>();
                    }
                    else
                    {
                        e.error = false;
                        e.files = visitSource(sourceElement);
                    }
                    output.put(source, e);
                }
            }
        }
    }
    
    private Slice2JavaGeneratedParser getGeneratedFiles(BuildState state, Set<IFile> candidates, StringBuffer out)
        throws CoreException
    {
        Slice2JavaGeneratedParser parser = new Slice2JavaGeneratedParser(state.generated, candidates);
        try
        {
            InputStream in = new ByteArrayInputStream(out.toString().getBytes());
            Document doc = DocumentBuilderFactory.newInstance().newDocumentBuilder().parse(new BufferedInputStream(in));
            parser.visit(doc);
        }
        catch(SAXException e)
        {
            throw new CoreException(new Status(IStatus.ERROR, Activator.PLUGIN_ID,
                    "internal error reading the generated output list", e));
        }
        catch(ParserConfigurationException e)
        {
            throw new CoreException(new Status(IStatus.ERROR, Activator.PLUGIN_ID,
                    "internal error reading the generated output list", e));
        }
        catch(IOException e)
        {
            throw new CoreException(new Status(IStatus.ERROR, Activator.PLUGIN_ID,
                    "internal error reading the generated output list", e));
        }
        return parser;
    }
}
