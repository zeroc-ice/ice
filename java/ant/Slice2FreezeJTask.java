// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
 * An ant task for slice2freezej. The task minimizes regeneration by
 * the use of tag files.
 *
 * Attributes:
 *
 *   translator - The pathname of the translator (default: "slice2java").
 *   tagdir - The directory in which tag files are located (default: ".").
 *   outputdir - The value for the --output-dir translator option.
 *
 * Nested elements:
 *
 *   includepath - The directories in which to search for Slice files.
 *   These are converted into -I directives for the translator.
 *   fileset - The set of Slice files which contain relevent types.
 *   dict - contains the NAME KEY & VALUE of the freeze type.
 *
 * Example:
 *
 *    <project ...>
 *        <taskdef name="slice2freezej" classname="Slice2FreezeJTask" />
 *        <property name="slice.dir" value="../include/slice"/>
 *        <target name="generate">
 *            <mkdir dir="tags" />
 *            <slice2freezej tagdir="tags" outputdir="out" output="CharIntMap">
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
 * The <taskdef> element installs the slice2java task.
 */
public class Slice2FreezeJTask extends org.apache.tools.ant.Task
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
    setTagdir(File dir)
    {
        _tagDir = dir;
    }

    public void
    setOutputdir(File dir)
    {
        _outputDir = dir;
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
	// Determine if the output file needs to be created.
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
	    }
	}
	
	java.util.List tagFiles = new java.util.LinkedList();
	
	p = _dicts.iterator();
	while(p.hasNext())
	{
	    //
	    // Build the complete list of tag files - it's not
	    // possible to stop on the first missing or old file since
	    // if changed they all need to be touched.
	    //
	    File tag = new File(_tagDir, "." + ((Dict)p.next()).getName() + ".tag");
	    tagFiles.add(tag);
	    
	    if(!build)
	    {
		if(!tag.exists())
		{
		    build = true;
		    continue;
		}

		java.util.Iterator q = sliceFiles.iterator();
		while(q.hasNext())
		{
		    File slice = (File)q.next();
		    if(slice.lastModified() > tag.lastModified())
		    {
			build = true;
			break;
		    }
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
	// Add --output-dir
	//
	if(_outputDir != null)
	{
	    cmd.append(" --output-dir ");
	    cmd.append(_outputDir.toString());
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
	// Touch the tag files.
	//
	p = tagFiles.iterator();
	while(p.hasNext())
	{
	    File tag = (File)p.next();
	    try
	    {
		FileOutputStream out = new FileOutputStream(tag);
		out.close();
	    }
	    catch(java.io.IOException ex)
	    {
		throw new BuildException("Unable to create tag file " +
					 tag + ": " + ex);
	    }
	}
    }

    private File _translator = new File("slice2freezej");
    private File _tagDir = new File(".");
    private File _outputDir = null;
    private Path _includePath = null;
    private java.util.List _fileSets = new java.util.LinkedList();

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
