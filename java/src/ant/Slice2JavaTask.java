// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
 * An ant task for slice2java. This task extends the abstract
 * SliceTask class which takes care of attributes common to all slice
 * translators (see SliceTask.java for details on these attributes).
 *
 * Attributes specific to slice2java:
 *
 *   translator - The pathname of the translator (default: "slice2java").
 *   tie - The value for the --tie translator option.
 *   checksum - The value for the --checksum translator option.
 *   stream - The value for the --stream translator option.
 *
 * Example:
 *
 *    <project ...>
 *        <taskdef name="slice2java" classname="Slice2JavaTask" />
 *        <property name="slice.dir" value="../include/slice"/>
 *        <target name="generate">
 *            <mkdir dir="tags" />
 *            <slice2java tagdir="tags" outputdir="out">
 *                <define name="SYMBOL" value="VALUE"/>
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
public class Slice2JavaTask extends SliceTask
{
    public
    Slice2JavaTask()
    {
        _translator = null;
        _tie = false;
        _checksum = null;
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
    setChecksum(String checksum)
    {
        _checksum = checksum;
    }

    public void
    setStream(boolean stream)
    {
        _stream = stream;
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
        java.util.HashMap<String, SliceDependency> dependencies = readDependencies();

        //
        // Compose a list of the files that need to be translated. A
        // file needs to translated if we can't find a dependency in
        // the dependency table or if its dependency is not up-to-date
        // anymore (the slice file changed since the dependency was
        // last updated or a slice file it depends on changed).
        //
        java.util.HashSet<File> buildList = new java.util.HashSet<File>();
        java.util.HashSet<File> skipList = new java.util.HashSet<File>();
        for(FileSet fileset : _fileSets)
        {
            DirectoryScanner scanner = fileset.getDirectoryScanner(getProject());
            scanner.scan();
            String[] files = scanner.getIncludedFiles();
            for(String file : files)
            {
                File slice = new File(fileset.getDir(getProject()), file);

                SliceDependency depend = dependencies.get(getTargetKey(slice.toString()));
                if(depend == null || !depend.isUpToDate())
                {
                    buildList.add(slice);
                }
                else
                {
                    skipList.add(slice);
                }
            }
        }

        if(_checksum != null && _checksum.length() > 0 && !buildList.isEmpty())
        {
            //
            // Recompile all Slice files when checksums are used.
            //
            buildList.addAll(skipList);
        }
        else
        {
            for(File file : skipList)
            {
                log("skipping " + file.getName());
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
                translator = getDefaultTranslator("slice2java");
            }
            else
            {
                translator = _translator.toString();
            }

            StringBuilder cmd = new StringBuilder(256);

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
                for(String dir : dirs)
                {
                    cmd.append(" -I");
                    if(dir.indexOf(' ') != -1)
                    {
                        cmd.append('"');
                        cmd.append(dir);
                        cmd.append('"');
                    }
                    else
                    {
                        cmd.append(dir);
                    }
                }
            }

            //
            // Add defines
            //
            if(!_defines.isEmpty())
            {
                for(SliceDefine define : _defines)
                {
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
            // Add --checksum
            //
            if(_checksum != null && _checksum.length() > 0)
            {
                cmd.append(" --checksum ");
                cmd.append(_checksum);
            }

            //
            // Add --stream
            //
            if(_stream)
            {
                cmd.append(" --stream");
            }

            //
            // Add --meta
            //
            if(!_meta.isEmpty())
            {
                for(SliceMeta m : _meta)
                {
                    cmd.append(" --meta ");
                    cmd.append(m.getValue());
                }
            }

            //
            // Add --ice
            //
            if(_ice)
            {
                cmd.append(" --ice");
            }

            //
            // Add --underscore
            //
            if(_underscore)
            {
                cmd.append(" --underscore");
            }

            //
            // Add files to be translated
            //
            for(File f : buildList)
            {
                cmd.append(" ");
                String s = f.toString();
                if(s.indexOf(' ') != -1)
                {
                    cmd.append('"');
                    cmd.append(s);
                    cmd.append('"');
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
            addLdLibraryPath(task);
            task.setFailonerror(true);
            Argument arg = task.createArg();
            arg.setLine(cmd.toString());
            task.setExecutable(translator);
            task.execute();

            //
            // Update the dependencies.
            //
            cmd = new StringBuilder(256);
            cmd.append("--depend-xml");

            //
            // Add --ice
            //
            if(_ice)
            {
                cmd.append(" --ice");
            }

            //
            // Add --underscore
            //
            if(_underscore)
            {
                cmd.append(" --underscore");
            }

            //
            // Add include directives
            //
            if(_includePath != null)
            {
                String[] dirs = _includePath.list();
                for(String dir : dirs)
                {
                    cmd.append(" -I");
                    if(dir.indexOf(' ') != -1)
                    {
                        cmd.append('"');
                        cmd.append(dir);
                        cmd.append('"');
                    }
                    else
                    {
                        cmd.append(dir);
                    }
                }
            }

            //
            // Add files for which we need to check dependencies.
            //
            for(File f : buildList)
            {
                cmd.append(" ");
                String s = f.toString();
                if(s.indexOf(' ') != -1)
                {
                    cmd.append('"');
                    cmd.append(s);
                    cmd.append('"');
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
            final String outputProperty = "slice2java.depend." + System.currentTimeMillis();
            final String errorProperty = "slice2java.error." + System.currentTimeMillis();

            task = (ExecTask)getProject().createTask("exec");
            addLdLibraryPath(task);
            task.setFailonerror(true);
            arg = task.createArg();
            arg.setLine(cmd.toString());
            task.setExecutable(translator);
            task.setOutputproperty(outputProperty);
            task.setErrorProperty(errorProperty);
            task.execute();

            //
            // Update dependency file.
            //
            java.util.List<SliceDependency> newDependencies =
                parseDependencies(getProject().getProperty(outputProperty));
            for(SliceDependency dep : newDependencies)
            {
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
        // If there's two slice2java tasks using the same dependency
        // file, with the same output dir and which compiles the same
        // slice file they'll use the same dependency.
        //
        return "slice2java " + _outputDir.toString() + " " + slice;
    }

    private File _translator;
    private boolean _tie;
    private String _checksum;
    private boolean _stream;
}
