// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
 *   casesensitive - The value for the --case-sensitive translator option.
 *
 * Nested elements:
 *
 *   includepath - The directories in which to search for Slice files.
 *   These are converted into -I directives for the translator.
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
	_caseSensitive = false;
        _includePath = null;
	_fileSets = new java.util.LinkedList();
    }

    public void
    setTagdir(File dir)
    {
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
    }

    public void
    setCaseSensitive(boolean c)
    {
        _caseSensitive = c;
    }

    public Path
    createIncludePath()
    {
        if(_includePath == null) 
        {
            _includePath = new Path(project);
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

		    String[] deps = depline.toString().split("[ \t\n\r]+");
		    assert(deps.length > 1);

		    //
		    // The first element is the target (<slice file>.cpp or .o dependending on the
		    // pre-processor), the second element is the compiled slice file and the other 
		    // elements are the dependency of the compiled slice file.
		    //

		    SliceDependency depend = new SliceDependency();

		    depend._dependencies = new String[deps.length - 1];
		    for(int i = 1; i < deps.length; ++i)
		    {
			depend._dependencies[i - 1] = deps[i];
		    }	
		    depend._timeStamp = new java.util.Date().getTime();

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
    };

    protected File _dependencyFile;
    protected File _outputDir;
    protected boolean _caseSensitive;
    protected Path _includePath;
    protected java.util.List _fileSets = new java.util.LinkedList();
}
