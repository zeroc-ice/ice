// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//package Ice.Ant;

import org.apache.tools.ant.Task;
import org.apache.tools.ant.BuildException;
import org.apache.tools.ant.DirectoryScanner;
import org.apache.tools.ant.types.FileSet;
import org.apache.tools.ant.taskdefs.ExecTask;
import org.apache.tools.ant.taskdefs.Execute;
import org.apache.tools.ant.taskdefs.PumpStreamHandler;
import org.apache.tools.ant.types.Commandline;
import org.apache.tools.ant.types.Environment;
import org.apache.tools.ant.types.Commandline.Argument;
import org.apache.tools.ant.types.Path;
import org.apache.tools.ant.types.Reference;

import java.io.File;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.StringReader;
import java.io.BufferedReader;
import java.io.BufferedWriter;

/**
 * An abstract ant task for slice translators. The task minimizes
 * regeneration by checking the dependencies between slice files.
 *
 * Attributes:
 *
 *   dependencyfile - The file in which dependencies are stored (default: ".depend").
 *   outputdir - The value for the --output-dir translator option.
 *   ice - Enables the --ice translator option.
 *
 * Nested elements:
 *
 *   includepath - The directories in which to search for Slice files.
 *     These are converted into -I directives for the translator.
 *   define - Defines a preprocessor symbol. The "name" attribute
 *     specifies the symbol name, and the optional "value" attribute
 *     specifies the symbol's value.
 *   fileset - The set of Slice files which contain relevent types.
 *
 */
public class SliceTask extends org.apache.tools.ant.Task
{
    public
    SliceTask()
    {
        _dependencyFile = null;
        _outputDir = null;
        _outputDirString = null;
        _ice = false;
        _includePath = null;
    }

    public void
    setDependencyFile(File file)
    {
        _dependencyFile = file;
    }

    public void
    setOutputdir(File dir)
    {
        _outputDir = dir;
        _outputDirString = _outputDir.toString();
        if(_outputDirString.indexOf(' ') != -1)
        {
            _outputDirString = '"' + _outputDirString + '"';
        }
    }

    public void
    setIce(boolean ice)
    {
        _ice = ice;
    }

    public Path
    createIncludePath()
    {
        if(_includePath == null)
        {
            _includePath = new Path(getProject());
        }
        return _includePath.createPath();
    }

    public void
    setIncludePathRef(Reference ref)
    {
        createIncludePath().setRefid(ref);
    }

    public void
    setIncludePath(Path includePath)
    {
        if(_includePath == null)
        {
            _includePath = includePath;
        }
        else
        {
            _includePath.append(includePath);
        }
    }

    public FileSet
    createFileset()
    {
        FileSet fileset = new FileSet();
        _fileSets.add(fileset);

        return fileset;
    }

    public void
    addConfiguredDefine(SliceDefine define)
        throws BuildException
    {
        if(define.getName() == null)
        {
            throw new BuildException("The name attribute must be supplied in a <define> element");
        }

        _defines.add(define);
    }

    public void
    addConfiguredMeta(SliceMeta meta)
    {
        if(meta.getValue().length() > 0)
        {
            _meta.add(meta);
        }
    }

    //
    // Read the dependency file.
    //
    @SuppressWarnings("unchecked")
    protected java.util.HashMap<String, SliceDependency>
    readDependencies()
    {
        if(_dependencyFile == null)
        {
            if(_outputDir != null)
            {
                _dependencyFile = new File(_outputDir, ".depend");
            }
            else
            {
                _dependencyFile = new File(".depend");
            }
        }

        try
        {
            java.io.ObjectInputStream in = new java.io.ObjectInputStream(new java.io.FileInputStream(_dependencyFile));
            java.util.HashMap<String, SliceDependency> dependencies =
                (java.util.HashMap<String, SliceDependency>)in.readObject();
            in.close();
            return dependencies;
        }
        catch(java.io.IOException ex)
        {
        }
        catch(java.lang.ClassNotFoundException ex)
        {
        }

        return new java.util.HashMap<String, SliceDependency>();
    }

    protected void
    writeDependencies(java.util.HashMap<String, SliceDependency> dependencies)
    {
        try
        {
            java.io.ObjectOutputStream out = new java.io.ObjectOutputStream(new FileOutputStream(_dependencyFile));
            out.writeObject(dependencies);
            out.close();
        }
        catch(java.io.IOException ex)
        {
            throw new BuildException("Unable to write dependencies in file " + _dependencyFile.getPath() + ": " + ex);
        }
    }

    //
    // Parse dependencies returned by the slice translator (Makefile
    // dependencies).
    //
    protected java.util.List<SliceDependency>
    parseDependencies(String allDependencies)
    {
        java.util.List<SliceDependency> dependencies = new java.util.LinkedList<SliceDependency>();
        try
        {
            BufferedReader in = new BufferedReader(new StringReader(allDependencies));
            StringBuilder depline = new StringBuilder(1024);
            String line;

            while((line = in.readLine()) != null)
            {
                if(line.length() == 0)
                {
                    continue;
                }
                else if(line.endsWith("\\"))
                {
                    depline.append(line.substring(0, line.length() - 1));
                }
                else
                {
                    depline.append(line);

                    //
                    // It's easier to split up the filenames if we first convert Windows
                    // path separators into Unix path separators.
                    //
                    char[] chars = depline.toString().toCharArray();
                    int pos = 0;
                    while(pos < chars.length)
                    {
                        if(chars[pos] == '\\')
                        {
                            if(pos + 1 < chars.length)
                            {
                                //
                                // Only convert the backslash if it's not an escape.
                                //
                                if(chars[pos + 1] != ' ' && chars[pos + 1] != ':' && chars[pos + 1] != '\r' &&
                                   chars[pos + 1] != '\n')
                                {
                                    chars[pos] = '/';
                                }
                            }
                        }
                        ++pos;
                    }

                    //
                    // Split the dependencies up into filenames. Note that filenames containing
                    // spaces are escaped and the initial file may have escaped colons
                    // (e.g., "C\:/Program\ Files/...").
                    //
                    java.util.ArrayList<String> l = new java.util.ArrayList<String>();
                    StringBuilder file = new StringBuilder(128);
                    pos = 0;
                    while(pos < chars.length)
                    {
                        if(chars[pos] == '\\') // Skip backslash of an escaped character.
                        {
                            if(pos + 1 >= chars.length)
                            {
                                throw new BuildException("dependency parse failure");
                            }
                            file.append(chars[++pos]);
                        }
                        else if(Character.isWhitespace(chars[pos]))
                        {
                            if(file.length() > 0)
                            {
                                l.add(file.toString());
                                file = new StringBuilder(128);
                            }
                        }
                        else
                        {
                            file.append(chars[pos]);
                        }
                        ++pos;
                    }
                    if(file.length() > 0)
                    {
                        l.add(file.toString());
                    }

                    //
                    // Create SliceDependency. We need to remove the trailing colon from the first file.
                    // We also normalize the pathname for this platform.
                    //
                    SliceDependency depend = new SliceDependency();
                    depend._dependencies = new String[l.size()];
                    l.toArray(depend._dependencies);
                    depend._timeStamp = new java.util.Date().getTime();
                    pos = depend._dependencies[0].lastIndexOf(':');
                    if(pos != depend._dependencies[0].length() - 1)
                    {
                        throw new BuildException("dependency parse failure");
                    }
                    depend._dependencies[0] = depend._dependencies[0].substring(0, pos);
                    for(int i = 0; i < depend._dependencies.length; ++i)
                    {
                        depend._dependencies[i] = new File(depend._dependencies[i]).toString();
                    }
                    dependencies.add(depend);

                    depline = new StringBuilder(1024);
                }
            }
        }
        catch(java.io.IOException ex)
        {
            throw new BuildException("Unable to read dependencies from slice translator: " + ex);
        }

        return dependencies;
    }

    protected String
    getDefaultTranslator(String name)
    {
        String iceInstall = getIceHome();
        if(iceInstall != null)
        {
            return new File(iceInstall + File.separator + "bin" + File.separator + name).toString();
        }
        else
        {
            //
            // If the location of the Ice install is not known, we
            // rely on a path search to find the translator.
            //
            return name;
        }
    }

    protected void
    addLdLibraryPath(ExecTask task)
    {
        String iceInstall = getIceHome();
        boolean srcdist = getProject().getProperties().containsKey("ice.src.dist");
        if(iceInstall != null)
        {
            String ldLibPathEnv = null;
            String ldLib64PathEnv = null;
            String libPath = new File(iceInstall + File.separator + "lib").toString();
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
                    lib64Path = new File(iceInstall + File.separator + "lib" + File.separator + "pa20_64").toString();
                }
            }
            else if(os.startsWith("Windows"))
            {
                //
                // No need to change the PATH environment variable on Windows, the DLLs should be found
                // in the translator local directory.
                //
                //ldLibPathEnv = "PATH";
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
                    lib64Path = new File(iceInstall + File.separator + "lib" + File.separator + "amd64").toString();
                }
                else // Sparc
                {
                    lib64Path = new File(iceInstall + File.separator + "lib" + File.separator + "sparcv9").toString();
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
                    lib64Path = new File(iceInstall + File.separator + "lib64").toString();
                }
            }

            if(ldLibPathEnv != null)
            {
                if(ldLibPathEnv.equals(ldLib64PathEnv))
                {
                    libPath = libPath + File.pathSeparator + lib64Path;
                }

                String envLibPath = getEnvironment(ldLibPathEnv);
                if(envLibPath != null)
                {
                    libPath = libPath + File.pathSeparator + envLibPath;
                }

                Environment.Variable v = new Environment.Variable();
                v.setKey(ldLibPathEnv);
                v.setValue(libPath);
                task.addEnv(v);
            }

            if(ldLib64PathEnv != null && !ldLib64PathEnv.equals(ldLibPathEnv))
            {
                String envLib64Path = getEnvironment(ldLib64PathEnv);
                if(envLib64Path != null)
                {
                    lib64Path = lib64Path + File.pathSeparator + envLib64Path;
                }

                Environment.Variable v = new Environment.Variable();
                v.setKey(ldLib64PathEnv);
                v.setValue(lib64Path);
                task.addEnv(v);
            }
        }
    }

    //
    // Query for the location of the Ice install. The Ice install
    // location may be indicated in one of two ways:
    //
    //  1. Through the ice.home property
    //  2. Through the ICE_HOME environment variable.
    //
    //  If both the property and environment variable is specified, the
    //  property takes precedence. If neither is available, getIceHome()
    //  returns null.
    //
    protected String
    getIceHome()
    {
        if(_iceHome == null)
        {
            if(getProject().getProperties().containsKey("ice.home"))
            {
                _iceHome = (String)getProject().getProperties().get("ice.home");
            }
            else
            {
                _iceHome = getEnvironment("ICE_HOME");
            }
        }
        return _iceHome;
    }

    //
    // A slice dependency.
    //
    // * the _timeStamp attribute contains the last time the slice
    //   file was compiled.
    //
    // * the _dependencies attribute contains an array with all the
    //   files this slice file depends on.
    //
    // This dependency represents the dependencies for the slice file
    // _dependencies[0].
    //
    protected class SliceDependency implements java.io.Serializable
    {
        private void writeObject(java.io.ObjectOutputStream out)
            throws java.io.IOException
        {
            out.writeObject(_dependencies);
            out.writeLong(_timeStamp);
        }

        private void readObject(java.io.ObjectInputStream in)
            throws java.io.IOException, java.lang.ClassNotFoundException
        {
            _dependencies = (String[])in.readObject();
            _timeStamp = in.readLong();
        }

        public boolean
        isUpToDate()
        {
            for(String d : _dependencies)
            {
                File dep = new File(d);
                if(!dep.exists() || _timeStamp < dep.lastModified())
                {
                    return false;
                }
            }

            return true;
        }

        public String[] _dependencies;
        public long _timeStamp;
    }

    private String
    getEnvironment(String key)
    {
        java.util.Vector env = Execute.getProcEnvironment();
        java.util.Enumeration e = env.elements();
        while(e.hasMoreElements())
        {
            String entry = (String)e.nextElement();
            if(entry.startsWith(key + "="))
            {
                return entry.substring(entry.indexOf('=') + 1);
            }
        }
        return null;
    }

    protected File _dependencyFile;
    protected File _outputDir;
    protected String _outputDirString;
    protected boolean _ice;
    protected Path _includePath;
    protected java.util.List<FileSet> _fileSets = new java.util.LinkedList<FileSet>();
    protected java.util.List<SliceDefine> _defines = new java.util.LinkedList<SliceDefine>();
    protected java.util.List<SliceMeta> _meta = new java.util.LinkedList<SliceMeta>();
    private String _iceHome;
}
