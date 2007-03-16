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
 * An ant task for slice2javae. This task extends the abstract
 * SliceTask class which takes care of attributes common to all slice
 * translators (see SliceTask.java for details on these attributes).
 *
 * Attributes specific to slice2javae:
 *
 *   translator - The pathname of the translator (default: "slice2javae").
 *   tie - The value for the --tie translator option.
 *
 * Example:
 *
 *    <project ...>
 *        <taskdef name="slice2javae" classname="Slice2JavaETask" />
 *        <property name="slice.dir" value="../include/slice"/>
 *        <target name="generate">
 *            <mkdir dir="tags" />
 *            <slice2javae tagdir="tags" outputdir="out">
 *                <define name="SYMBOL" value="VALUE"/>
 *                <includepath>
 *                    <pathelement path="${slice.dir}" />
 *                </includepath>
 *                <fileset dir="${slice.dir}">
 *                    <include name="*.ice" />
 *                </fileset>
 *            </slice2javae>
 *        </target>
 *    </project>
 *
 * The <taskdef> element installs the slice2javae task.
 */
public class Slice2JavaETask extends SliceTask
{
    public
    Slice2JavaETask()
    {
        _translator = null;
        _tie = false;
    }

    public void
    setTranslator(File prog)
    {
        _translator = prog;
    }

    public void
    setTie(boolean tie)
    {
        _tie = tie;
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
	// Read the set of dependencies for this task.
	//
	java.util.HashMap dependencies = readDependencies();

        //
        // Compose a list of the files that need to be translated. A
        // file needs to translated if we can't find a dependency in
        // the dependency table or if its dependency is not up-to-date
        // anymore (the slice file changed since the dependency was
        // last updated or a slice file it depends on changed).
        //
        java.util.Vector buildList = new java.util.Vector();
        java.util.Iterator p = _fileSets.iterator();
        while(p.hasNext())
        {
            FileSet fileset = (FileSet)p.next();

            DirectoryScanner scanner = fileset.getDirectoryScanner(getProject());
            scanner.scan();
            String[] files = scanner.getIncludedFiles();
            for(int i = 0; i < files.length; i++)
            {
                File slice = new File(fileset.getDir(getProject()), files[i]);

		SliceDependency depend = (SliceDependency)dependencies.get(getTargetKey(slice.toString()));
		if(depend == null || !depend.isUpToDate())
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
            String translator;
            if(_translator == null)
            {
                if(getIceHome() == null)
                {
                    translator = "slice2javae";
                }
                else
                {
                    translator = new File(getIceHome() + File.separator + "bin" + File.separator + "slice2javae").toString();
                }
            }
            else
            {
                translator = _translator.toString();
            }

            StringBuffer cmd = new StringBuffer();

            //
            // Add --output-dir
            //
            if(_outputDir != null)
            {
                cmd.append(" --output-dir ");
                cmd.append(_outputDirString);
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
                    if(dirs[i].indexOf(' ') != -1)
                    {
                        cmd.append('"' + dirs[i] + '"');
                    }
                    else
                    {
                        cmd.append(dirs[i]);
                    }
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
            // Add --tie
            //
            if(_tie)
            {
                cmd.append(" --tie");
            }

	    //
	    // Add --ice
	    //
            if(_ice)
            {
                cmd.append(" --ice");
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
                String s = f.toString();
                if(s.indexOf(' ') != -1)
                {
                    cmd.append('"' + s + '"');
                }
                else
                {
                    cmd.append(s);
                }
            }

            //
            // Execute
            //
            log(translator + " " + cmd);
            ExecTask task = (ExecTask)getProject().createTask("exec");
            task.setFailonerror(true);
            Argument arg = task.createArg();
            arg.setLine(cmd.toString());
            task.setExecutable(translator);
            task.execute();

            //
            // Update the dependencies.
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
                    if(dirs[i].indexOf(' ') != -1)
                    {
                        cmd.append('"' + dirs[i] + '"');
                    }
                    else
                    {
                        cmd.append(dirs[i]);
                    }
                }
            }

            //
            // Add files for which we need to check dependencies.
            //
            for(int i = 0; i < buildList.size(); i++)
            {
                File f = (File)buildList.elementAt(i);
                cmd.append(" ");
                String s = f.toString();
                if(s.indexOf(' ') != -1)
                {
                    cmd.append('"' + s + '"');
                }
                else
                {
                    cmd.append(s);
                }
            }

	    //
	    // It's not possible anymore to re-use the same output property since Ant 1.5.x. so we use a 
	    // unique property name here. Perhaps we should output the dependencies to a file instead.
	    //
	    final String outputProperty = "slice2javae.depend." + System.currentTimeMillis();

	    task = (ExecTask)getProject().createTask("exec");
            task.setFailonerror(true);
	    arg = task.createArg();
            arg.setLine(cmd.toString());
            task.setExecutable(translator);
	    task.setOutputproperty(outputProperty);
            task.execute();

	    //
	    // Update dependency file.
	    //
	    java.util.List newDependencies = parseDependencies(getProject().getProperty(outputProperty));
	    p = newDependencies.iterator();
	    while(p.hasNext())
	    {
		SliceDependency dep = (SliceDependency)p.next();
		dependencies.put(getTargetKey(dep._dependencies[0]), dep);
	    }
		
	    writeDependencies(dependencies);
        }
    }

    private String
    getTargetKey(String slice)
    {
	//
	// Since the dependency file can be shared by several slice
	// tasks we need to make sure that each dependency has a
	// unique key. We use the name of the task, the output
	// directory and the name of the slice file to be compiled. 
	//
	// If there's two slice2javae tasks using the same dependency
	// file, with the same output dir and which compiles the same
	// slice file they'll use the same dependency.
	//
	return "slice2javae " + _outputDir.toString() + " " + slice;
    }

    private File _translator;
    private boolean _tie;
}
