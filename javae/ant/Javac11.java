// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

import org.apache.tools.ant.BuildException;
import org.apache.tools.ant.Project;
import org.apache.tools.ant.types.Commandline;
import org.apache.tools.ant.types.Path;
import org.apache.tools.ant.taskdefs.Execute;
import org.apache.tools.ant.taskdefs.LogStreamHandler;

//
// This is a "compiler adapter" that allows us to execute a Java 1.1 compiler
// to build the JDK portions of IceEJ.
//
// The ant property java11.home must be defined and specify the installation
// directory of JDK 1.1.
//
public class Javac11 extends org.apache.tools.ant.taskdefs.compilers.DefaultCompilerAdapter
{
    protected boolean
    assumeJava11()
    {
	return true;
    }

    protected Path
    getCompileClasspath()
    {
	Path classpath = new Path(attributes.getProject());

	java.io.File destDir = attributes.getDestdir();
	if(destDir != null)
	{
	    classpath.setLocation(destDir);
	}

	Path cp = attributes.getClasspath();
	if(cp != null)
	{
	    classpath.addExisting(cp);
	}

	Path runtime = new Path(attributes.getProject(), _javacHome + "/lib/classes.zip");
	classpath.append(runtime);

	return classpath;
    }

    public boolean
    execute()
	throws BuildException
    {
	Project project = attributes.getProject();
	_javacHome = project.getProperty("java11.home");
	if(_javacHome == null)
	{
	    throw new BuildException("The property java11.home is not defined");
	}

	attributes.log("Using javac 1.1 compiler", Project.MSG_VERBOSE);

	Commandline cmd = new Commandline();
	cmd.setExecutable(_javacHome + "/bin/javac");
	setupJavacCommandlineSwitches(cmd, true);
	logAndAddFilesToCompile(cmd);

        String[] commandArray = cmd.getCommandline();

	try
	{
	    Execute exe = new Execute(new LogStreamHandler(attributes, Project.MSG_INFO, Project.MSG_WARN));

	    //
	    // Overwrite JAVA_HOME so that javac runs correctly.
	    //
	    final String[] env = { "JAVA_HOME=" + _javacHome };
	    exe.setEnvironment(env);

	    exe.setAntRun(project);
	    exe.setWorkingDirectory(project.getBaseDir());
	    exe.setCommandline(commandArray);
	    exe.execute();

	    return exe.getExitValue() == 0;
	}
	catch(java.io.IOException ex)
	{
	    throw new BuildException("Error running " + commandArray[0] + " compiler", ex, location);
	}
    }

    private String _javacHome;
}
