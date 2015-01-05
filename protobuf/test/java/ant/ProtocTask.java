// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Protobuf is licensed to you under the terms
// described in the ICE_PROTOBUF_LICENSE file included in this
// distribution.
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
 * An ant task for protoc.
 *
 * Attributes specific to protoc:
 *
 *   translator - The pathname of the translator (default: "protoc").
 *   protocpath - The value for the --proto_path translator option.
 *   outputdir - The value for the --java_out translator option.
 *   dependencyfile - The file in which dependencies are stored (default: ".pbdepend").
 *
 * Example:
 *
 *    <project ...>
 *        <taskdef name="protoc" classname="ProtocTask" />
 *        <property name="protoc.dir" value="../include/protoc"/>
 *        <target name="generate">
 *            <mkdir dir="tags" />
 *            <protoc tagdir="tags" outputdir="out">
 *                <fileset dir="${protoc.dir}">
 *                    <include name="*.ice" />
 *                </fileset>
 *            </protoc>
 *        </target>
 *    </project>
 *
 * The <taskdef> element installs the protoctask task.
 */
public class ProtocTask extends org.apache.tools.ant.Task
{
    public
    ProtocTask()
    {
        _translator = null;
        _outputDir = null;
        _protocPath = null;
        _outputDirString = null;
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
    setProtocpath(File dir)
    {
        _protocPath = dir.toString();
    }

    public void
    setTranslator(File prog)
    {
        _translator = prog;
    }

    public FileSet
    createFileset()
    {
        FileSet fileset = new FileSet();
        _fileSets.add(fileset);

        return fileset;
    }

    @SuppressWarnings("unchecked")
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
        // anymore (the proto file changed since the dependency was
        // last updated)
        //
        java.util.Vector<File> buildList = new java.util.Vector<File>();
        java.util.Vector<File> skipList = new java.util.Vector<File>();
        java.util.Iterator p = _fileSets.iterator();
        while(p.hasNext())
        {
            FileSet fileset = (FileSet)p.next();

            DirectoryScanner scanner = fileset.getDirectoryScanner(getProject());
            scanner.scan();
            String[] files = scanner.getIncludedFiles();
            for(int i = 0; i < files.length; i++)
            {
                File proto = new File(fileset.getDir(getProject()), files[i]);

                ProtoDependency depend = (ProtoDependency)dependencies.get(getTargetKey(proto.toString()));
                if(depend == null || !depend.isUpToDate())
                {
                    buildList.addElement(proto);
                }
                else
                {
                    skipList.addElement(proto);
                }
            }

            java.util.Iterator i = skipList.iterator();
            while(i.hasNext())
            {
                File file = (File)i.next();
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
                translator = "protoc";
            }
            else
            {
                translator = _translator.toString();
            }

            StringBuilder cmd = new StringBuilder(128);

            //
            // Add --java_out.
            //
            if(_outputDir != null)
            {
                cmd.append(" --java_out=");
                cmd.append(stripDriveLetter(_outputDirString));
            }

            //
            // Add --proto_path
            //
            if(_protocPath != null)
            {
                cmd.append(" --proto_path=");
                cmd.append(stripDriveLetter(_protocPath));
            }

            //
            // Add files to be translated
            //
            for(int i = 0; i < buildList.size(); i++)
            {
                File f = (File)buildList.elementAt(i);
                cmd.append(" ");
                String s = stripDriveLetter(f.toString());
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
            task.setFailonerror(true);
            Argument arg = task.createArg();
            arg.setLine(cmd.toString());
            task.setExecutable(translator);
            task.execute();

            //
            // Update dependency file.
            //
            for(int i = 0; i < buildList.size(); i++)
            {
                ProtoDependency depend = new ProtoDependency();
                depend._timeStamp = new java.util.Date().getTime();
                depend._dependency = ((File)buildList.elementAt(i)).toString();
                dependencies.put(getTargetKey(depend._dependency), depend);
            }

            writeDependencies(dependencies);
        }
    }

    private String
    getTargetKey(String proto)
    {
        //
        // Since the dependency file can be shared by several proto
        // tasks we need to make sure that each dependency has a
        // unique key. We use the name of the task, the output
        // directory and the name of the proto file to be compiled.
        //
        // If there's two protoc tasks using the same dependency
        // file, with the same output dir and which compiles the same
        // protoc file they'll use the same dependency.
        //
        return "protoc " + _outputDir.toString() + " " + proto;
    }

    // This is to work around a bug with protoc, where it does not
    // accept drive letters in path names. See
    // http://bugzilla/bugzilla/show_bug.cgi?id=3349
    //
    private String
    stripDriveLetter(String s)
    {
        if(s.length() > 1 && s.charAt(1) == ':')
        {
            return s.substring(2);
        }
        return s;
    }

    //
    // Read the dependency file.
    //
    private java.util.HashMap
    readDependencies()
    {
        if(_dependencyFile == null)
        {
            if(_outputDir != null)
            {
                _dependencyFile = new File(_outputDir, ".pbdepend");
            }
            else
            {
                _dependencyFile = new File(".pbdepend");
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

    private void
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
    // A proto dependency.
    //
    // * the _timeStamp attribute contains the last time the proto
    //   file was compiled.
    //
    // * the _dependency attribute contains the .proto file.
    //
    private class ProtoDependency implements java.io.Serializable
    {
        private void writeObject(java.io.ObjectOutputStream out)
            throws java.io.IOException
        {
            out.writeObject(_dependency);
            out.writeLong(_timeStamp);
        }

        private void readObject(java.io.ObjectInputStream in)
            throws java.io.IOException, java.lang.ClassNotFoundException
        {
            _dependency = (String)in.readObject();
            _timeStamp = in.readLong();
        }

        public boolean
        isUpToDate()
        {
            File dep = new File(_dependency);
            if(!dep.exists() || _timeStamp < dep.lastModified())
            {
                return false;
            }

            return true;
        }

        public String _dependency;
        public long _timeStamp;
    }

    private File _translator;
    private File _dependencyFile;
    private File _outputDir;
    private String _outputDirString;
    private String _protocPath;
    private java.util.List<FileSet> _fileSets = new java.util.LinkedList<FileSet>();
}
