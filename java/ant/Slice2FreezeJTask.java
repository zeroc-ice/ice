// **********************************************************************
//
// Copyright (c) 2003
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
import org.apache.tools.ant.types.Commandline.Argument;
import org.apache.tools.ant.types.Path;
import org.apache.tools.ant.types.Reference;

import java.io.File;
import java.io.FileOutputStream;

/**
 * An ant task for slice2freezej. This task extends the abstract
 * SliceTask class which takes care of attributes common to all slice
 * translators (see SliceTask.java for details on these attributes).
 *
 * Attributes:
 *
 *   binary - Enables --binary option.
 *
 * Nested elements:
 *
 *   define - defines a preprocessor symbol
 *   dict - contains the NAME, KEY TYPE, and VALUE TYPE of a Freeze map.
 *
 * Example:
 *
 *    <project ...>
 *        <taskdef name="slice2freezej" classname="Slice2FreezeJTask" />
 *        <property name="slice.dir" value="../include/slice"/>
 *        <target name="generate">
 *            <mkdir dir="tags" />
 *            <slice2freezej tagdir="tags" outputdir="out" output="CharIntMap">
 *                <define name="SYMBOL" value="VALUE"/>
 *                <includepath>
 *                    <pathelement path="${slice.dir}" />
 *                </includepath>
 *                <fileset dir="${slice.dir}">
 *                    <include name="*.ice" />
 *                </fileset>
 *                <dict name="CharIntMap" key="char" value="int"/>
 *            </slice2freezej>
 *        </target>
 *    </project>
 *
 * The <taskdef> element installs the slice2freezej task.
 */
public class Slice2FreezeJTask extends SliceTask
{
    public
    Slice2FreezeJTask()
    {
    }

    public void
    setTranslator(File prog)
    {
        _translator = prog;
    }

    public void
    setBinary(boolean binary)
    {
        _binary = binary;
    }

    public Dict
    createDict()
    {
	Dict d = new Dict();
	_dicts.add(d);
	return d;
    }

    public void
    execute()
        throws BuildException
    {
	if(_dicts.isEmpty())
	{
            throw new BuildException("No dictionaries specified");
	}

	//
	// Read the set of dependencies for this task.
	//
	java.util.HashMap dependencies = readDependencies();

	//
	// Check if the set of slice files changed. If it changed we
	// need to rebuild all the dictionnaries.
	//
	boolean build = false;
	java.util.List sliceFiles = new java.util.LinkedList();

	java.util.Iterator p = _fileSets.iterator();
	while(p.hasNext())
	{
	    FileSet fileset = (FileSet)p.next();

	    DirectoryScanner scanner = fileset.getDirectoryScanner(project);
	    String[] files = scanner.getIncludedFiles();
	    
	    for(int i = 0; i < files.length; i++)
	    {
		File slice = new File(fileset.getDir(project), files[i]);
		sliceFiles.add(slice);

		if(!build)
		{
		    //
		    // The dictionnaries need to be re-created since
		    // on dependency changed.
		    //
		    SliceDependency depend = (SliceDependency)dependencies.get(getSliceTargetKey(slice.toString()));
		    if(depend == null || !depend.isUpToDate())
		    {
			build = true;
		    }
		}
	    }
	}

	if(!build)
	{
	    //
	    // Check that each dictionnaries has been built at least
	    // once.
	    //
	    p = _dicts.iterator();
	    while(p.hasNext())
	    {
		SliceDependency depend = (SliceDependency)dependencies.get(getDictTargetKey((Dict)p.next()));
		if(depend == null)
		{
		    build = true;
		    break;
		}
	    }
	}
	
	//
	// Add the --dict options.
	//
	p = _dicts.iterator();
	StringBuffer dictString = new StringBuffer();
	while(p.hasNext())
	{
	    Dict d = (Dict)p.next();

	    dictString.append(" --dict ");
	    dictString.append(d.getName() + "," + d.getKey() + "," + d.getValue());
	}

	if(!build)
	{
	    log("skipping" + dictString);
	    return;
	}

        //
        // Run the translator
        //
	StringBuffer cmd = new StringBuffer();

	//
	// Add --ice
	//
        if(_ice)
        {
            cmd.append(" --ice");
        }

	//
	// Add --output-dir
	//
	if(_outputDir != null)
	{
	    cmd.append(" --output-dir ");
	    cmd.append(_outputDir.toString());
	}

        //
        // Add --binary
        //
        if(_binary)
        {
            cmd.append(" --binary");
        }

	//
	// Add --case-sensitive
	//
	if(_caseSensitive)
	{
	    cmd.append(" --case-sensitive");
	}

	//
	// Add include directives
	//
	if(_includePath != null)
	{
	    String[] dirs = _includePath.list();
	    for(int i = 0; i < dirs.length; i++)
	    {
		cmd.append(" -I");
		cmd.append(dirs[i]);
	    }
	}

        //
        // Add defines
        //
        if(!_defines.isEmpty())
        {
            java.util.Iterator i = _defines.iterator();
            while(i.hasNext())
            {
                SliceDefine define = (SliceDefine)i.next();
                cmd.append(" -D");
                cmd.append(define.getName());
                String value = define.getValue();
                if(value != null)
                {
                    cmd.append("=");
                    cmd.append(value);
                }
            }
        }

	//
	// Add the --dict options.
	//
	cmd.append(dictString);

	//
	// Add the slice files.
	//
	p = sliceFiles.iterator();
	while(p.hasNext())
	{
	    File f = (File)p.next();
	    cmd.append(" " + f);
	}
	
	//
	// Execute.
	//
	log(_translator.toString() + " " + cmd);
	ExecTask task = (ExecTask)project.createTask("exec");
	task.setFailonerror(true);
	Argument arg = task.createArg();
	arg.setLine(cmd.toString());
	task.setExecutable(_translator.toString());
	task.execute();
	
	//
	// Update the dependencies.
	//	
	if(!sliceFiles.isEmpty())
	{
	    cmd = new StringBuffer("--depend");
	    
	    //
	    // Add include directives
	    //
	    if(_includePath != null)
	    {
		String[] dirs = _includePath.list();
		for(int i = 0; i < dirs.length; i++)
		{
		    cmd.append(" -I");
		    cmd.append(dirs[i]);
		}
	    }

	    //
	    // Add the slice files.
	    //
	    p = sliceFiles.iterator();
	    while(p.hasNext())
	    {
		File f = (File)p.next();
		cmd.append(" " + f.toString());
	    }

	    //
	    // Add the --dict options.
	    //
	    cmd.append(dictString);

	    //
	    // It's not possible anymore to re-use the same output property since Ant 1.5.x. so we use a 
	    // unique property name here. Perhaps we should output the dependencies to a file instead.
	    //
	    final String outputProperty = "slice2freezej.depend." + System.currentTimeMillis();

	    task = (ExecTask)project.createTask("exec");
	    task.setFailonerror(true);
	    arg = task.createArg();
	    arg.setLine(cmd.toString());
	    task.setExecutable(_translator.toString());
	    task.setOutputproperty(outputProperty);
	    task.execute();

	    //
	    // Update dependency file.
	    //
	    java.util.List newDependencies = parseDependencies(project.getProperty(outputProperty));
	    p = newDependencies.iterator();
	    while(p.hasNext())
	    {
		SliceDependency dep = (SliceDependency)p.next();
		dependencies.put(getSliceTargetKey(dep._dependencies[0]), dep);
	    }
	}

	p = _dicts.iterator();
	while(p.hasNext())
	{
	    dependencies.put(getDictTargetKey((Dict)p.next()), new SliceDependency());
	}

	writeDependencies(dependencies);
    }

    private String
    getSliceTargetKey(String slice)
    {
	//
	// Since the dependency file can be shared by several slice
	// tasks we need to make sure that each dependency has a
	// unique key. We use the name of the task, the output
	// directory, the first dictionnary name and the name of the
	// slice file to be compiled.
	//
	return "slice2freezej " + _outputDir.toString() + ((Dict)_dicts.get(0)).getName() + slice;
    }

    private String
    getDictTargetKey(Dict d)
    {
	return "slice2freezej " + _outputDir.toString() + d.getName();
    }

    private File _translator = new File("slice2freezej");
    private boolean _binary = false;

    public class Dict
    {
	private String _name;
	private String _key;
	private String _value;

	public void
	setName(String name)
	{
	    _name = name;
	}

	public String
	getName()
	{
	    return _name;
	}

	public void
	setKey(String key)
	{
	    _key = key;
	}

	public String
	getKey()
	{
	    return _key;
	}

	public void
	setValue(String value)
	{
	    _value = value;
	}

	public String
	getValue()
	{
	    return _value;
	}
    };
    private java.util.List _dicts = new java.util.LinkedList();
}
