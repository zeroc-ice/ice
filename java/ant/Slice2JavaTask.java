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
        _fileSet = null;
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
        if (_includePath == null) 
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
        if (_includePath == null)
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
        if (_fileSet == null) 
        {
            _fileSet = new FileSet();
        }
        return _fileSet;
    }

    public void
    execute()
        throws BuildException
    {
        if (_fileSet == null)
        {
            throw new BuildException("No fileset specified");
        }

        //
        // Compose a list of the files that need to be translated
        //
        java.util.Vector buildList = new java.util.Vector();
        java.util.Vector tagList = new java.util.Vector();
        DirectoryScanner scanner = _fileSet.getDirectoryScanner(project);
        scanner.scan();
        String[] files = scanner.getIncludedFiles();
        for (int i = 0; i < files.length; i++)
        {
            File slice = new File(_fileSet.getDir(project), files[i]);
            File tag = new File(_tagDir, "." + slice.getName() + ".tag");

            if (tag.exists() && slice.lastModified() <= tag.lastModified())
            {
                log("skipping " + files[i]);
            }
            else
            {
                buildList.addElement(slice);
                tagList.addElement(tag);
            }
        }

        //
        // Run the translator
        //
        if (!buildList.isEmpty())
        {
            StringBuffer cmd = new StringBuffer();

            //
            // Add --output-dir
            //
            if (_outputDir != null)
            {
                cmd.append(" --output-dir ");
                cmd.append(_outputDir.toString());
            }

            //
            // Add --package
            //
            if (_package != null)
            {
                cmd.append(" --package ");
                cmd.append(_package);
            }

            //
            // Add include directives
            //
            if (_includePath != null)
            {
                String[] dirs = _includePath.list();
                for (int i = 0; i < dirs.length; i++)
                {
                    cmd.append(" -I");
                    cmd.append(dirs[i]);
                }
            }

            //
            // Add files to be translated
            //
            for (int i = 0; i < buildList.size(); i++)
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
            // Touch the tag files
            //
            for (int i = 0; i < tagList.size(); i++)
            {
                File f = (File)tagList.elementAt(i);
                try
                {
                    FileOutputStream out = new FileOutputStream(f);
                    out.close();
                }
                catch(java.io.IOException ex)
                {
                    throw new BuildException("Unable to create tag file " + f + ": " + ex);
                }
            }
        }
    }

    private File _translator;
    private File _tagDir;
    private File _outputDir;
    private String _package;
    private Path _includePath;
    private FileSet _fileSet;
}
