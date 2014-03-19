// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This plug-in is provided to you under the terms and conditions
// of the Eclipse Public License Version 1.0 ("EPL"). A copy of
// the EPL is available at http://www.eclipse.org/legal/epl-v10.html.
//
// **********************************************************************

package com.zeroc.slice2javaplugin.internal;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.StringTokenizer;

import org.eclipse.core.resources.IFolder;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.ProjectScope;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Path;
import org.eclipse.core.runtime.Status;
import org.eclipse.core.runtime.preferences.InstanceScope;
import org.eclipse.jdt.core.IClasspathEntry;
import org.eclipse.jdt.core.IJavaProject;
import org.eclipse.jdt.core.JavaCore;
import org.eclipse.jdt.core.JavaModelException;
import org.eclipse.ui.preferences.ScopedPreferenceStore;

import com.zeroc.slice2javaplugin.Activator;
import com.zeroc.slice2javaplugin.preferences.PluginPreferencePage;

public class Configuration
{
    public Configuration(IProject project)
    {
        _project = project;

        _instanceStore = new ScopedPreferenceStore(new InstanceScope(), Activator.PLUGIN_ID + "." + _project.getName());

        _store = new ScopedPreferenceStore(new ProjectScope(project), Activator.PLUGIN_ID);

        _androidProject = false;
        try
        {
            _androidProject = project.hasNature("com.android.ide.eclipse.adt.AndroidNature");
        }
        catch(CoreException e)
        {
        }

        _store.setDefault(GENERATED_KEY, GENERATED_KEY);
        _store.setDefault(DEFINES_KEY, "");
        _store.setDefault(TIE_KEY, false);
        _store.setDefault(ICE_KEY, false);
        _store.setDefault(STREAM_KEY, false);
        _store.setDefault(ICE_INCLUDE_KEY, false);
        _store.setDefault(META_KEY, "");
        _store.setDefault(CONSOLE_KEY, false);
        _store.setDefault(SLICE_SOURCE_DIRS_KEY, "slice");
        _store.setDefault(INCLUDES_KEY, "");
        _store.setDefault(ADD_JARS_KEY, true);
        _store.setDefault(UNDERSCORE_KEY, false);

        _store.setDefault(JARS_KEY, "Ice.jar");
    }

    /**
     * Turns list of strings into a single ';' delimited string. ';' in the
     * string values are escaped with a leading '\'. '\' are turned into '\\'.
     *
     * @param l
     *            List of strings.
     * @return Semicolon delimited string.
     */
    static public String fromList(List<String> l)
    {
        StringBuffer sb = new StringBuffer();
        for(Iterator<String> p = l.iterator(); p.hasNext();)
        {
            if(sb.length() > 0)
            {
                sb.append(";");
            }
            sb.append(escape(p.next()));
        }
        return sb.toString();
    }

    /**
     * Turn a semicolon delimited string into a list of strings. Escaped values
     * are preserved (characters prefixed with a '\').
     *
     * @param s
     *            Semicolon delimited string.
     * @return List of strings.
     */
    static public List<String> toList(String s)
    {
        java.util.List<String> l = new ArrayList<String>();
        int curr = 0;
        int end = s.length();
        boolean escape = false;
        StringBuffer sb = new StringBuffer();
        for(curr = 0; curr < end; ++curr)
        {
            char ch = s.charAt(curr);
            if(escape)
            {
                sb.append(ch);
                escape = false;
            }
            else
            {
                if(ch == ';')
                {
                    String tok = sb.toString().trim();
                    sb.setLength(0);
                    if(tok.length() > 0)
                    {
                        l.add(tok);
                    }
                }
                else if(ch == '\\')
                {
                    escape = true;
                }
                else
                {
                    sb.append(ch);
                }
            }
        }
        String tok = sb.toString().trim();
        if(tok.length() > 0)
        {
            l.add(tok);
        }
        return l;
    }

    public boolean write()
        throws CoreException, IOException
    {
        boolean rc = false;
        if(_store.needsSaving())
        {
            _store.save();
            rc = true;
        }
        if(_instanceStore.needsSaving())
        {
            _instanceStore.save();
            rc = true;
        }

        if(rc)
        {
            IJavaProject javaProject = JavaCore.create(_project);
            if(getAddJars())
            {
                if(isAndroidProject())
                {
                    for(String jar : getJars())
                    {
                        addLibrary(javaProject, jar);
                    }
                }
                else
                {
                    addLibrary(javaProject);
                }
            }
            else
            {
                removeLibrary(javaProject);
            }
        }

        return rc;
    }

    public void initialize()
        throws CoreException
    {
        // Create the slice source directories, if necessary.
        for(Iterator<String> p = getSliceSourceDirs().iterator(); p.hasNext();)
        {
            IFolder slice = _project.getFolder(p.next());
            if(!slice.exists())
            {
                slice.create(false, true, null);
            }
        }

        // Create the generated directory, if necessary.
        IFolder generated = _project.getFolder(getGeneratedDir());
        if(!generated.exists())
        {
            generated.create(false, true, null);
        }

        fixGeneratedCP(null, getGeneratedDir());

        IJavaProject javaProject = JavaCore.create(_project);
        if(isAndroidProject())
        {
            for(String jar : getJars())
            {
                addLibrary(javaProject, jar);
            }
        }
        else
        {
            addLibrary(javaProject);
        }
    }

    public void deinstall()
        throws CoreException
    {
        IJavaProject javaProject = JavaCore.create(_project);
        if(isAndroidProject())
        {
            removeLibrary(javaProject, "Ice.jar");
            removeLibrary(javaProject, "Glacier2.jar");
            removeLibrary(javaProject, "IceBox.jar");
            removeLibrary(javaProject, "IceGrid.jar");
            removeLibrary(javaProject, "IcePatch2.jar");
            removeLibrary(javaProject, "IceStorm.jar");
        }
        else
        {
            removeLibrary(javaProject);
        }
        removedGeneratedCP();
        IFolder generatedFolder = _project.getFolder(getGeneratedDir());
        if(generatedFolder != null && generatedFolder.exists())
        {
            generatedFolder.delete(true, null);
        }
    }

    public boolean isAndroidProject()
    {
        return _androidProject;
    }

    public List<String> getSliceSourceDirs()
    {
        return toList(_store.getString(SLICE_SOURCE_DIRS_KEY));
    }

    public void setSliceSourceDirs(List<String> sliceSourceDirs)
    {
        setValue(SLICE_SOURCE_DIRS_KEY, fromList(sliceSourceDirs));
    }

    public String getGeneratedDir()
    {
        return _store.getString(GENERATED_KEY);
    }

    public void fixGeneratedCP(String oldG, String newG)
            throws CoreException
    {
        IJavaProject javaProject = JavaCore.create(_project);

        IFolder newGenerated = _project.getFolder(newG);

        IClasspathEntry[] entries = javaProject.getRawClasspath();
        IClasspathEntry newEntry = JavaCore.newSourceEntry(newGenerated.getFullPath());

        if(oldG != null)
        {
            IFolder oldGenerated = _project.getFolder(oldG);
            IClasspathEntry oldEntry = JavaCore.newSourceEntry(oldGenerated.getFullPath());
            for(int i = 0; i < entries.length; ++i)
            {
                if(entries[i].equals(oldEntry))
                {
                    entries[i] = newEntry;
                    javaProject.setRawClasspath(entries, null);
                    oldGenerated.delete(true, null);
                    return;
                }
            }
        }

        IClasspathEntry[] newEntries = new IClasspathEntry[entries.length + 1];
        System.arraycopy(entries, 0, newEntries, 1, entries.length);
        newEntries[0] = newEntry;
        
        newGenerated.setDerived(true, null);

        try
        {
            javaProject.setRawClasspath(newEntries, null);
        }
        catch(JavaModelException e)
        {
            // This can occur if a duplicate CLASSPATH entry is made.
            //
            // throw new CoreException(new Status(IStatus.ERROR,
            // Activator.PLUGIN_ID, e.toString(), null));
        }
    }
    
    public void removedGeneratedCP()
            throws CoreException
    {
        IJavaProject javaProject = JavaCore.create(_project);

        IFolder generated = _project.getFolder(getGeneratedDir());

        IClasspathEntry generatedEntry = JavaCore.newSourceEntry(generated.getFullPath());
        
        IClasspathEntry[] entries = javaProject.getRawClasspath();
        IClasspathEntry[] newEntries = new IClasspathEntry[entries.length - 1];

        for(int i = 0, j = 0; i < entries.length; i++)
        {
            if(entries[i].equals(generatedEntry))
            {
                continue;
            }
            newEntries[j] = entries[i];
            j++;
        }

        try
        {
            javaProject.setRawClasspath(newEntries, null);
        }
        catch(JavaModelException e)
        {
            // This can occur if a duplicate CLASSPATH entry is made.
            //
            // throw new CoreException(new Status(IStatus.ERROR,
            // Activator.PLUGIN_ID, e.toString(), null));
        }
    }

    public void setGeneratedDir(String generated)
        throws CoreException
    {
        String oldGenerated = getGeneratedDir();
        if(setValue(GENERATED_KEY, generated))
        {
            fixGeneratedCP(oldGenerated, generated);
        }
    }

    public List<String> getCommandLine()
    {
        List<String> cmds = new ArrayList<String>();
        for(Iterator<String> p = getIncludes().iterator(); p.hasNext();)
        {
            cmds.add("-I" + p.next());
        }
        for(Iterator<String> p = getDefines().iterator(); p.hasNext();)
        {
            cmds.add("-D" + p.next());
        }
        for(Iterator<String> p = getMeta().iterator(); p.hasNext();)
        {
            cmds.add("--meta");
            cmds.add(p.next());
        }
        if(getStream())
        {
            cmds.add("--stream");
        }
        if(getTie())
        {
            cmds.add("--tie");
        }
        if(getIce())
        {
            cmds.add("--ice");
        }
        if(getUnderscore())
        {
            cmds.add("--underscore");
        }
        
        StringTokenizer tokens = new StringTokenizer(getExtraArguments());
        while(tokens.hasMoreTokens())
        {  
            cmds.add(tokens.nextToken());  
        }

        return cmds;
    }
    
    public List<String> getCommandLine(IResource resource)
    {
        List<String> cmds = getCommandLine();
        for(Iterator<String> p = getBareIncludes(resource).iterator(); p.hasNext();)
        {
            cmds.add("-I" + p.next());
        }
        for(Iterator<String> p = getDefines(resource).iterator(); p.hasNext();)
        {
            cmds.add("-D" + p.next());
        }
        for(Iterator<String> p = getMeta(resource).iterator(); p.hasNext();)
        {
            cmds.add("--meta");
            cmds.add(p.next());
        }
        if(!getStream() && getStream(resource))
        {
            cmds.add("--stream");
        }
        if(!getTie() && getTie(resource))
        {
            cmds.add("--tie");
        }
        if(!getIce() && getIce(resource))
        {
            cmds.add("--ice");
        }
        if(!getUnderscore() && getUnderscore(resource))
        {
            cmds.add("--underscore");
        }
        
        StringTokenizer tokens = new StringTokenizer(getExtraArguments(resource));
        while(tokens.hasMoreTokens())
        {  
            cmds.add(tokens.nextToken());  
        }

        return cmds;
    }

    public List<String> getIncludes()
    {
        List<String> s = toList(_store.getString(INCLUDES_KEY));
        
        String iceHome = getIceHome();
        String os = System.getProperty("os.name");
        String path = null;
        if(os.equals("Linux") && iceHome.equals("/usr"))
        {
            String version = getIceVersion();
            if(version != null)
            {
                File f = new File("/usr/share/Ice-" + version + "/slice");
                if(f.exists())
                {
                    path = f.toString();
                }
            }
        }

        if(path == null)
        {
            path = new File(iceHome + File.separator + "slice").toString();
        }

        s.add(path);
        return s;
    }

    // The bare include list.
    public List<String> getBareIncludes()
    {
        return toList(_store.getString(INCLUDES_KEY));
    }
    
    public List<String> getBareIncludes(IResource resource)
    {
        return toList(_store.getString(resourceKey(resource, INCLUDES_KEY)));
    }

    public void setIncludes(List<String> includes)
    {
        setValue(INCLUDES_KEY, fromList(includes));
    }
    
    public void setIncludes(IResource resource, List<String> includes)
    {
        setValue(resourceKey(resource, INCLUDES_KEY), fromList(includes));
    }

    public boolean getAddJars()
    {
        return _store.getBoolean(ADD_JARS_KEY);
    }

    public void setAddJars(boolean b)
    {
        _store.setValue(ADD_JARS_KEY, b);
    }

    public List<String> getJars()
    {
        return toList(_store.getString(JARS_KEY));
    }

    public void setJars(List<String> jars) throws CoreException
    {
        if(setValue(JARS_KEY, fromList(jars)))
        {
            if(isAndroidProject())
            {
                IJavaProject javaProject = JavaCore.create(_project);
                ArrayList<String> removeJars = new ArrayList<String>();
                removeJars.add("Glacier2.jar");
                removeJars.add("IceBox.jar");
                removeJars.add("IceGrid.jar");
                removeJars.add("IcePatch2.jar");
                removeJars.add("IceStorm.jar");
                
                for(String jar : jars)
                {
                    removeJars.remove(jar);
                    addLibrary(javaProject, jar);
                }
                
                for(String jar : removeJars)
                {
                    removeLibrary(javaProject, jar);
                }
            }
            else
            {
                IceClasspathContainerIntializer.reinitialize(_project, this);
            }
        }
    }

    public List<String> getDefines()
    {
        return toList(_store.getString(DEFINES_KEY));
    }
    
    public List<String> getDefines(IResource resource)
    {
        return toList(_store.getString(resourceKey(resource, DEFINES_KEY)));
    }

    public void setDefines(List<String> defines)
    {
        setValue(DEFINES_KEY, fromList(defines));
    }
    
    public void setDefines(IResource resource, List<String> defines)
    {
        setValue(resourceKey(resource, DEFINES_KEY), fromList(defines));
    }

    public boolean getStream()
    {
        return _store.getBoolean(STREAM_KEY);
    }
    
    public boolean getStream(IResource resource)
    {
        return _store.getBoolean(resourceKey(resource, STREAM_KEY));
    }

    public void setStream(boolean stream)
    {
        _store.setValue(STREAM_KEY, stream);
    }
    
    public void setStream(IResource resource, boolean stream)
    {
        _store.setValue(resourceKey(resource, STREAM_KEY), stream);
    }

    public boolean getTie()
    {
        return _store.getBoolean(TIE_KEY);
    }
    
    public boolean getTie(IResource resource)
    {
        return _store.getBoolean(resourceKey(resource, TIE_KEY));
    }

    public void setTie(boolean tie)
    {
        _store.setValue(TIE_KEY, tie);
    }
    
    public void setTie(IResource resource, boolean tie)
    {
        _store.setValue(resourceKey(resource, TIE_KEY), tie);
    }

    public boolean getIce()
    {
        return _store.getBoolean(ICE_KEY);
    }
    
    public boolean getIce(IResource resource)
    {
        return _store.getBoolean(resourceKey(resource, ICE_KEY));
    }

    public void setIce(boolean ice)
    {
        _store.setValue(ICE_KEY, ice);
    }
    
    public void setIce(IResource resource, boolean ice)
    {
        _store.setValue(resourceKey(resource, ICE_KEY), ice);
    }

    public boolean getUnderscore()
    {
        return _store.getBoolean(UNDERSCORE_KEY);
    }
    
    public boolean getUnderscore(IResource resource)
    {
        return _store.getBoolean(resourceKey(resource, UNDERSCORE_KEY));
    }

    public void setUnderscore(boolean underscore)
    {
        _store.setValue(UNDERSCORE_KEY, underscore);
    }
    
    public void setUnderscore(IResource resource, boolean underscore)
    {
        _store.setValue(resourceKey(resource, UNDERSCORE_KEY), underscore);
    }

    public boolean getConsole()
    {
        return _store.getBoolean(CONSOLE_KEY);
    }

    public void setConsole(boolean console)
    {
        _store.setValue(CONSOLE_KEY, console);
    }

    public List<String> getMeta()
    {
        return toList(_store.getString(META_KEY));
    }
    
    public List<String> getMeta(IResource resource)
    {
        return toList(_store.getString(resourceKey(resource, META_KEY)));
    }

    public void setMeta(List<String> meta)
    {
        setValue(META_KEY, fromList(meta));
    }
    
    public void setMeta(IResource resource, List<String> meta)
    {
        setValue(resourceKey(resource, META_KEY), fromList(meta));
    }
    
    public String getExtraArguments()
    {
        return _store.getString(EXTRA_ARGUMENTS_KEY);
    }
    
    public String getExtraArguments(IResource resource)
    {
        return _store.getString(resourceKey(resource, EXTRA_ARGUMENTS_KEY));
    }

    public void setExtraArguments(String arguments)
    {
        setValue(EXTRA_ARGUMENTS_KEY, arguments);
    }
    
    public void setExtraArguments(IResource resource, String arguments)
    {
        setValue(resourceKey(resource, EXTRA_ARGUMENTS_KEY), arguments);
    }

    public static void setupSharedLibraryPath(Map<String, String> env)
    {
        String iceHome = getIceHome();

        String libPath;
        boolean srcdist = false;
        if(new File(iceHome + File.separator + "cpp" + File.separator + "bin").exists())
        {
            // iceHome points at a source distribution.
            libPath = new File(iceHome + File.separator + "cpp" + File.separator + "lib").toString();
            srcdist = true;
        }
        else
        {
            libPath = new File(iceHome + File.separator + "lib").toString();
        }

        String ldLibPathEnv = null;
        String ldLib64PathEnv = null;
        String lib64Path = null;

        String os = System.getProperty("os.name");
        if(os.equals("Mac OS X"))
        {
            ldLibPathEnv = "DYLD_LIBRARY_PATH";
        }
        else if(os.equals("AIX"))
        {
            ldLibPathEnv = "LIBPATH";
        }
        else if(os.equals("HP-UX"))
        {
            ldLibPathEnv = "SHLIB_PATH";
            ldLib64PathEnv = "LD_LIBRARY_PATH";
            if(srcdist)
            {
                lib64Path = libPath;
            }
            else
            {
                lib64Path = new File(iceHome + File.separator + "lib" + File.separator + "pa20_64").toString();
            }
        }
        else if(os.startsWith("Windows"))
        {
            //
            // No need to change the PATH environment variable on Windows, the
            // DLLs should be found
            // in the translator local directory.
            //
            // ldLibPathEnv = "PATH";
        }
        else if(os.equals("SunOS"))
        {
            ldLibPathEnv = "LD_LIBRARY_PATH";
            ldLib64PathEnv = "LD_LIBRARY_PATH_64";
            String arch = System.getProperty("os.arch");
            if(srcdist)
            {
                lib64Path = libPath;
            }
            else if(arch.equals("x86"))
            {
                lib64Path = new File(iceHome + File.separator + "lib" + File.separator + "amd64").toString();
            }
            else
            // Sparc
            {
                lib64Path = new File(iceHome + File.separator + "lib" + File.separator + "sparcv9").toString();
            }
        }
        else
        {
            ldLibPathEnv = "LD_LIBRARY_PATH";
            ldLib64PathEnv = "LD_LIBRARY_PATH";
            if(srcdist)
            {
                lib64Path = libPath;
            }
            else
            {
                lib64Path = new File(iceHome + File.separator + "lib64").toString();
            }
        }

        if(ldLibPathEnv != null)
        {
            if(ldLibPathEnv.equals(ldLib64PathEnv))
            {
                libPath = libPath + File.pathSeparator + lib64Path;
            }

            String envLibPath = env.get(ldLibPathEnv);
            if(envLibPath != null)
            {
                libPath = libPath + File.pathSeparator + envLibPath;
            }

            env.put(ldLibPathEnv, libPath);
        }

        if(ldLib64PathEnv != null && !ldLib64PathEnv.equals(ldLibPathEnv))
        {
            String envLib64Path = env.get(ldLib64PathEnv);
            if(envLib64Path != null)
            {
                lib64Path = lib64Path + File.pathSeparator + envLib64Path;
            }
            env.put(ldLib64PathEnv, lib64Path);
        }
    }

    public String getTranslator()
    {
        return getTranslatorForHome(getIceHome());
    }

    static public boolean verifyIceHome(String dir)
    {
        return getTranslatorForHome(dir) != null;
    }

    public static String getJarDir()
    {
        String iceHome = getIceHome();
        String os = System.getProperty("os.name");
        if(os.equals("Linux") && iceHome.equals("/usr"))
        {
            File f = new File(iceHome + File.separator + "share" + File.separator + "java");
            if(f.exists())
            {
                return f.toString();
            }
        }

        File f = new File(iceHome + File.separator + "lib");
        if(!f.exists())
        {
            File f2 = new File(iceHome + File.separator + "java" + File.separator + "lib");
            if(f2.exists())
            {
                return f2.toString();
            }
        }
        // Add the platform default even if it cannot be found.
        return f.toString();
    }

    private static String getIceHome()
    {
        return Activator.getDefault().getPreferenceStore().getString(PluginPreferencePage.SDK_PATH);
    }

    // For some reason ScopedPreferenceStore.setValue(String, String)
    // doesn't check to see whether the stored value is the same as
    // the new value.
    private boolean setValue(String key, String value)
    {
        return setValue(_store, key, value);
    }

    private boolean setValue(ScopedPreferenceStore store, String key, String value)
    {
        if(!store.getString(key).equals(value))
        {
            store.setValue(key, value);
            return true;
        }
        return false;
    }

    static private String escape(String s)
    {
        int curr = 0;
        int end = s.length();
        StringBuffer sb = new StringBuffer();
        for(curr = 0; curr < end; ++curr)
        {
            char ch = s.charAt(curr);
            if(ch == '\\' || ch == ';')
            {
                sb.append('\\');
            }
            sb.append(ch);
        }
        return sb.toString();
    }

    // Obtain the Ice version by executing the translator with the -v option.
    private String getIceVersion()
    {
        String version = null;
        String exec = getTranslatorForHome(getIceHome());
        if(exec != null)
        {
            try
            {
                ProcessBuilder b = new ProcessBuilder(exec, "-v");
                b.redirectErrorStream(true);
                Map<String, String> env = b.environment();
                setupSharedLibraryPath(env);
                Process p = b.start();
                int status = p.waitFor();
                if(status == 0)
                {
                    BufferedReader r = new BufferedReader(new InputStreamReader(p.getInputStream()));
                    String line = r.readLine();
                    version = line.trim();
                }
            }
            catch(Throwable ex)
            {
                // Ignore.
            }
        }
        return version;
    }

    private static String getTranslatorForHome(String dir)
    {
        String suffix = "";
        String os = System.getProperty("os.name");
        if(os.startsWith("Windows"))
        {
            suffix = ".exe";
        }
        File f = new File(dir + File.separator + "bin" + File.separator + "slice2java" + suffix);
        if(f.exists())
        {
            return f.toString();
        }
        f = new File(dir + File.separator + "cpp" + File.separator + "bin" + File.separator + "slice2java" + suffix);
        if(f.exists())
        {
            return f.toString();
        }
        return null;
    }

    private void addLibrary(IJavaProject project)
        throws CoreException
    {
        IClasspathEntry cpEntry = null;
        if(!isAndroidProject())
        {
            cpEntry = IceClasspathContainerIntializer.getContainerEntry();
        }
        else
        {
            cpEntry = JavaCore.newVariableEntry(new Path("ICE_JAR_HOME/Ice.jar"), 
                                                new Path("ICE_JAR_HOME/Ice.jar"), 
                                                new Path("ICE_JAR_HOME/lib/"), 
                                                true);
        }
        
        IClasspathEntry[] entries = project.getRawClasspath();
        boolean found = false;
        for(int i = 0; i < entries.length; ++i)
        {
            if(entries[i].equals(cpEntry))
            {
                found = true;
                break;
            }
        }

        if(!found)
        {
            IClasspathEntry[] newEntries = new IClasspathEntry[entries.length + 1];
            System.arraycopy(entries, 0, newEntries, 0, entries.length);
            newEntries[entries.length] = cpEntry;

            try
            {
                project.setRawClasspath(newEntries, null);
            }
            catch(JavaModelException e)
            {
                throw new CoreException(new Status(IStatus.ERROR, Activator.PLUGIN_ID, e.toString(), null));
            }
        }
    }
    
    private void addLibrary(IJavaProject project, String jar)
        throws CoreException
    {
        IClasspathEntry cpEntry = JavaCore.newVariableEntry(new Path("ICE_JAR_HOME/" + jar), 
                                                            new Path("ICE_JAR_HOME/" + jar), 
                                                            new Path("ICE_JAR_HOME/"), 
                                                            true);
        
        IClasspathEntry[] entries = project.getRawClasspath();
        boolean found = false;
        for(int i = 0; i < entries.length; ++i)
        {
            if(entries[i].equals(cpEntry))
            {
                found = true;
                break;
            }
        }
    
        if(!found)
        {
            IClasspathEntry[] newEntries = new IClasspathEntry[entries.length + 1];
            System.arraycopy(entries, 0, newEntries, 0, entries.length);
            newEntries[entries.length] = cpEntry;
    
            try
            {
                project.setRawClasspath(newEntries, null);
            }
            catch(JavaModelException e)
            {
                throw new CoreException(new Status(IStatus.ERROR, Activator.PLUGIN_ID, e.toString(), null));
            }
        }
    }

    public void removeLibrary(IJavaProject project)
        throws CoreException
    {
        IClasspathEntry cpEntry = IceClasspathContainerIntializer.getContainerEntry();

        IClasspathEntry[] entries = project.getRawClasspath();
    
        for(int i = 0; i < entries.length; ++i)
        {
            if(entries[i].equals(cpEntry))
            {
                IClasspathEntry[] newEntries = new IClasspathEntry[entries.length - 1];
                System.arraycopy(entries, 0, newEntries, 0, i);
                System.arraycopy(entries, i + 1, newEntries, i, entries.length - i - 1);
    
                try
                {
                    project.setRawClasspath(newEntries, null);
                }
                catch(JavaModelException e)
                {
                    throw new CoreException(new Status(IStatus.ERROR, Activator.PLUGIN_ID, e.toString(), null));
                }
                break;
            }
        }
    }
    
    public void removeLibrary(IJavaProject project, String lib)
        throws CoreException
    {
        Path path = new Path("ICE_JAR_HOME/" + lib);
        
        IClasspathEntry[] entries = project.getRawClasspath();
    
        for(int i = 0; i < entries.length; ++i)
        {
            if(entries[i].getPath().equals(path))
            {
                IClasspathEntry[] newEntries = new IClasspathEntry[entries.length - 1];
                System.arraycopy(entries, 0, newEntries, 0, i);
                System.arraycopy(entries, i + 1, newEntries, i, entries.length - i - 1);
    
                try
                {
                    project.setRawClasspath(newEntries, null);
                }
                catch(JavaModelException e)
                {
                    throw new CoreException(new Status(IStatus.ERROR, Activator.PLUGIN_ID, e.toString(), null));
                }
                break;
            }
        }
    }
    
    //
    // Check if the given resource has any Slice compiler options set.
    //
    public static boolean resourceHasOptions(IResource resource)
    {
        Configuration configuration = new Configuration(resource.getProject());
        if(configuration.getDefines(resource) != null && configuration.getDefines(resource).size() > 0)
        {
            return true;
        }
        if(configuration.getMeta(resource) != null && configuration.getMeta(resource).size() > 0)
        {
            return true;
        }
        if(!configuration.getStream() && configuration.getStream(resource))
        {
            return true;
        }
        if(!configuration.getTie() && configuration.getTie(resource))
        {
            return true;
        }
        if(!configuration.getIce() && configuration.getIce(resource))
        {
            return true;
        }
        if(!configuration.getUnderscore() && configuration.getUnderscore(resource))
        {
            return true;
        }
        if(configuration.getExtraArguments(resource) != null && !configuration.getExtraArguments(resource).isEmpty())
        {
            return true;
        }
        return false;
    }
    
    public static String resourceKey(IResource resource, String key)
    {
        return resource.getFullPath().toString() + "." + key;
    }

    private static final String JARS_KEY = "jars";
    private static final String INCLUDES_KEY = "includes";
    private static final String SLICE_SOURCE_DIRS_KEY = "sliceSourceDirs";
    private static final String CONSOLE_KEY = "console";
    private static final String META_KEY = "meta";
    private static final String STREAM_KEY = "stream";
    private static final String ICE_INCLUDE_KEY = "iceIncludes";
    private static final String ICE_KEY = "ice";
    private static final String TIE_KEY = "tie";
    private static final String DEFINES_KEY = "defines";
    private static final String GENERATED_KEY = "generated";
    private static final String ADD_JARS_KEY = "addJars";
    private static final String UNDERSCORE_KEY = "underscore";
    
    private static final String EXTRA_ARGUMENTS_KEY = "extraArguments";

    // Preferences store for items which should go in SCM. This includes things
    // like build flags.
    private ScopedPreferenceStore _store;

    // Preferences store per project items which should not go in SCM, such as
    // the location of the Ice installation.
    private ScopedPreferenceStore _instanceStore;

    private IProject _project;

    private boolean _androidProject;
}
