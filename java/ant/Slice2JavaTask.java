// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
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
 * An ant task for slice2java. The task minimizes regeneration by
 * creating a tag file whose timestamp represents the last time
 * the corresponding Slice file was generated.
 *
 * Attributes:
 *
 *   translator - The pathname of the translator (default: "slice2java").
 *   tagdir - The directory in which tag files are located (default: ".").
 *   outputdir - The value for the --output-dir translator option.
 *   package - The value for the --package translator option.
 *   tie - The value for the --tie translator option.
 *   casesensitive - The value for the --case-sensitive translator option.
 *
 * Nested elements:
 *
 *   includepath - The directories in which to search for Slice files.
 *       These are converted into -I directives for the translator.
 *   fileset - The set of Slice files to generate.
 *
 * Example:
 *
 *    <project ...>
 *        <taskdef name="slice2java" classname="Slice2JavaTask" />
 *        <property name="slice.dir" value="../include/slice"/>
 *        <target name="generate">
 *            <mkdir dir="tags" />
 *            <slice2java tagdir="tags" outputdir="out" package="com.foo">
 *                <includepath>
 *                    <pathelement path="${slice.dir}" />
 *                </includepath>
 *                <fileset dir="${slice.dir}">
 *                    <include name="*.ice" />
 *                </fileset>
 *            </slice2java>
 *        </target>
 *    </project>
 *
 * The <taskdef> element installs the slice2java task.
 */
public class Slice2JavaTask extends org.apache.tools.ant.Task
{
    public
    Slice2JavaTask()
    {
        _translator = new File("slice2java");
        _tagDir = new File(".");
        _outputDir = null;
        _package = null;
        _includePath = null;
        _tie = false;
	_caseSensitive = false;
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

    public void
    setPackage(String pkg)
    {
        _package = pkg;
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

    public void
    setTie(boolean tie)
    {
        _tie = tie;
    }

    public void
    setCaseSensitive(boolean c)
    {
        _caseSensitive = c;
    }

    public FileSet
    createFileset()
    {
        FileSet fileset = new FileSet();
        _fileSets.add(fileset);

        return fileset;
    }

    public void
    execute()
        throws BuildException
    {
        if(_fileSets.isEmpty())
        {
            throw new BuildException("No fileset specified");
        }

        //
        // Compose a list of the files that need to be translated
        //
        java.util.Vector buildList = new java.util.Vector();
        java.util.Iterator p = _fileSets.iterator();
        while(p.hasNext())
        {
            FileSet fileset = (FileSet)p.next();

            DirectoryScanner scanner = fileset.getDirectoryScanner(project);
            scanner.scan();
            String[] files = scanner.getIncludedFiles();
            for(int i = 0; i < files.length; i++)
            {
		boolean translate = false;

                File slice = new File(fileset.getDir(project), files[i]);
                File tag = new File(_tagDir, "." + slice.getName() + ".tag");

                if(tag.exists() && slice.lastModified() <= tag.lastModified())
                {
		    try
		    {
			BufferedReader reader = new BufferedReader(new FileReader(tag));
			String dependency;
			while((dependency = reader.readLine()) != null)
			{
			    File dependencyFile = new File(dependency);
			    if(!dependencyFile.exists() || tag.lastModified() < dependencyFile.lastModified())
			    {
				translate = true;
				break;
			    }
			}
			reader.close();
		    }
		    catch(java.io.FileNotFoundException ex)
		    {
			translate = true;
		    }
		    catch(java.io.IOException ex)
		    {
			translate = true;
		    }
                }
		else
		{
		    translate = true;
		}

		if(translate)
                {
                    buildList.addElement(slice);
                }
		else
		{
                    log("skipping " + files[i]);
		}
            }
        }

        //
        // Run the translator
        //
        if(!buildList.isEmpty())
        {
            StringBuffer cmd = new StringBuffer();

	    //
	    // Add --ice
	    //
	    cmd.append(" --ice");

            //
            // Add --output-dir
            //
            if(_outputDir != null)
            {
                cmd.append(" --output-dir ");
                cmd.append(_outputDir.toString());
            }

            //
            // Add --package
            //
            if(_package != null)
            {
                cmd.append(" --package ");
                cmd.append(_package);
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
            // Add --tie
            //
            if(_tie)
            {
                cmd.append(" --tie");
            }

            //
            // Add --case-sensitive
            //
            if(_caseSensitive)
            {
                cmd.append(" --case-sensitive");
            }

            //
            // Add files to be translated
            //
            for(int i = 0; i < buildList.size(); i++)
            {
                File f = (File)buildList.elementAt(i);
                cmd.append(" ");
                cmd.append(f.toString());
            }

            //
            // Execute
            //
            log(_translator.toString() + " " + cmd);
            ExecTask task = (ExecTask)project.createTask("exec");
            task.setFailonerror(true);
            Argument arg = task.createArg();
            arg.setLine(cmd.toString());
            task.setExecutable(_translator.toString());
            task.execute();

            //
            // Create the tag files
            //
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
            // Add files for which we need to check dependencies.
            //
            for(int i = 0; i < buildList.size(); i++)
            {
                File f = (File)buildList.elementAt(i);
                cmd.append(" ");
                cmd.append(f.toString());
            }

	    task = (ExecTask)project.createTask("exec");
            task.setFailonerror(true);
	    arg = task.createArg();
            arg.setLine(cmd.toString() + " --depend");
            task.setExecutable("slice2java");
	    task.setOutputproperty("slice2java.depend");
            task.execute();

	    try
	    {
		BufferedReader in = new BufferedReader(new StringReader(project.getProperty("slice2java.depend")));
		StringBuffer depline = new StringBuffer();
		String line;
		while((line = in.readLine()) != null)
		{
		    depline.append(line);

		    if(!line.endsWith("\\"))
		    {
			String[] deps = depline.toString().split("[\\s\\\\]");
			if(deps.length > 0)
			{
			    int pos = deps[0].indexOf('.');
			    if(pos != -1)
			    {
				String sliceFile = deps[0].substring(0, pos);
				File tag = new File(_tagDir, "." + sliceFile + ".ice.tag");
				
				try
				{
				    BufferedWriter out = new BufferedWriter(new FileWriter(tag));
				    for(int i = 2; i < deps.length; ++i)
				    {
					if(deps[i].length() > 0)
					{
					    out.write(deps[i]);
					    out.newLine();
					}
				    }
				    out.close();
				}
				catch(java.io.IOException ex)
				{
				    throw new BuildException("Unable to create tag file " + tag + ": " + ex);
				}
			    }
			}
			
			depline = new StringBuffer();
		    }
		}
	    }
	    catch(java.io.IOException ex)
	    {
		throw new BuildException("Unable to read dependencies to create tag files: " + ex);
	    }
        }
    }

    private File _translator;
    private File _tagDir;
    private File _outputDir;
    private String _package;
    private Path _includePath;
    private boolean _tie;
    private boolean _caseSensitive;
    private java.util.List _fileSets = new java.util.LinkedList();
}
