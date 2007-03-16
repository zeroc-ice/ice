// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
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
 *   casesensitive - Enables the --case-sensitive translator option.
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
	_caseSensitive = false;
        _ice = false;
        _includePath = null;

        //
        // Check for the presence of the ICE_HOME environment variable.
        //
        java.util.Vector env = Execute.getProcEnvironment();
        java.util.Enumeration e = env.elements();
        while(e.hasMoreElements())
        {
            String entry = (String)e.nextElement();
            if(entry.startsWith("ICE_HOME="))
            {
                _iceHome = entry.substring(entry.indexOf('=') + 1);
                break;
            }
        }
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
    setCaseSensitive(boolean c)
    {
        _caseSensitive = c;
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

    //
    // Read the dependency file.
    //
    protected java.util.HashMap
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
	    java.util.HashMap dependencies = (java.util.HashMap)in.readObject();
	    in.close();
	    return dependencies;
	}
	catch(java.io.IOException ex)
	{
	}
	catch(java.lang.ClassNotFoundException ex)
	{
	}
	
	return new java.util.HashMap();
    }

    protected void
    writeDependencies(java.util.HashMap dependencies)
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
    protected java.util.List
    parseDependencies(String allDependencies)
    {
	java.util.List dependencies = new java.util.LinkedList();
	try
	{
	    BufferedReader in = new BufferedReader(new StringReader(allDependencies));
	    StringBuffer depline = new StringBuffer();
	    String line;

	    while((line = in.readLine()) != null)
	    {
		if(line.endsWith("\\"))
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
		    java.util.ArrayList l = new java.util.ArrayList();
                    StringBuffer file = new StringBuffer();
                    pos = 0;
		    while(pos < chars.length)
		    {
			if(Character.isWhitespace(chars[pos]))
			{
			    if(file.length() > 0)
			    {
				l.add(file.toString());
				file = new StringBuffer();
			    }
			}
                        else if(chars[pos] != '\\') // Skip backslash of an escaped character.
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
                    //assert(pos == depend._dependencies[0].length() - 1);
                    depend._dependencies[0] = depend._dependencies[0].substring(0, pos);
                    for(int i = 0; i < depend._dependencies.length; ++i)
                    {
                        depend._dependencies[i] = new File(depend._dependencies[i]).toString();
                    }
		    dependencies.add(depend);

		    depline = new StringBuffer();
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
    getIceHome()
    {
        //
        // _iceHome used to be set in the constructor. It appears that the
        // current project isn't available at that point and consequently, the
        // properties that allow us to find the ice translators based on the
        // contents the 'ice.dir' property in the ant projects aren't available
        // yet.
        //
        if(_iceHome == null)
        {
            //
            // Check for the presence of the ICE_HOME environment variable.
            //
            java.util.Vector env = Execute.getProcEnvironment();
            java.util.Enumeration e = env.elements();
            while(e.hasMoreElements())
            {
                String entry = (String)e.nextElement();
                if(entry.startsWith("ICE_HOME="))
                {
                    _iceHome = entry.substring(entry.indexOf('=') + 1);
                    break;
                }
            }

            if(_iceHome == null)
            {
                if(getProject().getProperties().containsKey("ice.bin.dir"))
                {
                    _iceHome = (String)getProject().getProperties().get("ice.bin.dir");
                }
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
	    for(int i = 0; i < _dependencies.length; ++i)
	    {
		File dep = new File(_dependencies[i]);
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

    protected File _dependencyFile;
    protected File _outputDir;
    protected String _outputDirString;
    protected boolean _caseSensitive;
    protected boolean _ice;
    protected Path _includePath;
    protected java.util.List _fileSets = new java.util.LinkedList();
    protected java.util.List _defines = new java.util.LinkedList();
    protected String _iceHome;
}
