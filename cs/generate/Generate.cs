// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.IO;
using System.Text;
using System.Threading;

namespace Generate
{
    class Generate
    {
	static void Main(string[] args)
	{   
	    try
	    {
		const string slice2csName = "slice2cs";
		string iceHome = Environment.GetEnvironmentVariable("ICE_HOME");

		string progName = AppDomain.CurrentDomain.FriendlyName;
		if(args.Length < 3)
		{
		    Console.Error.WriteLine("usage: {0} solution_dir project_dir project_name [args]", progName);
		    Environment.Exit(1);
		}

		string solDir = args[0];
		string projDir = args[1];
		string projName = args[2];

		Directory.SetCurrentDirectory(projDir);

		const string slicePat = @"*.ice";
		string sliceDir = projDir;
		string[] sliceFiles = Directory.GetFiles(projDir, slicePat);

		string includes = "";
		if(Directory.Exists(Path.Combine(solDir, "slice")))
		{
		    includes = "-I" + Path.Combine(solDir, "slice");
		}
		if(iceHome != null)
		{
		    if(Directory.Exists(Path.Combine(iceHome, "slice")))
		    {
			includes += " -I" + Path.Combine(iceHome, "slice");
		    }
		}

		if(sliceFiles.Length == 0)
		{
		    sliceDir = Path.Combine(Path.Combine(solDir, "slice"), projName);
		    if(Directory.Exists(sliceDir))
		    {
			sliceFiles = Directory.GetFiles(sliceDir, slicePat);
		    }
		}
		if(sliceFiles.Length == 0)
		{
		    if(iceHome != null)
		    {
			sliceDir = Path.Combine(Path.Combine(iceHome, "slice"), projName);
			if(Directory.Exists(sliceDir))
			{
			    sliceFiles = Directory.GetFiles(sliceDir, slicePat);
			}
		    }
		}
		if(sliceFiles.Length == 0)
		{
		    Console.Error.WriteLine(progName + ": no Slice files found");
		    Environment.Exit(1);
		}

		string slice2cs = Path.Combine(Path.Combine(solDir, "bin"), slice2csName);
		if(!File.Exists(slice2cs) && !File.Exists(slice2cs + ".exe"))
		{
		    if(iceHome != null)
		    {
			slice2cs = Path.Combine(Path.Combine(iceHome, "bin"), slice2csName);
			if(!File.Exists(slice2cs) && !File.Exists(slice2cs + ".exe"))
			{
			    slice2cs = slice2csName;
			}
		    }
		    else
		    {
			slice2cs = slice2csName;
		    }
		}

		string outputDir = Path.Combine(projDir, "generated");
		string cmdArgs = "--ice -I. " + includes + " --output-dir " + outputDir;
		for(int i = 3; i < args.Length; ++i)
		{
		    if(args[i].IndexOf(' ') != -1)
		    {
			cmdArgs += " \"" + args[i] + "\"";
		    }
		    else
		    {
			cmdArgs += " " + args[i];
		    }
		}

		bool needCompile = false;
		foreach(string sliceFile in sliceFiles)
		{
		    DateTime sliceTime = File.GetLastWriteTime(sliceFile);
		    string csFile = Path.Combine(outputDir, Path.ChangeExtension(Path.GetFileName(sliceFile), ".cs"));
		    if(!File.Exists(csFile) || sliceTime > File.GetLastWriteTime(csFile))
		    {	  
			cmdArgs += " " + sliceFile;
			Console.Out.WriteLine(Path.GetFileName(sliceFile));
			needCompile = true;
		    }
		}

		if(needCompile)
		{
		    ProcessStartInfo info = new ProcessStartInfo(slice2cs, cmdArgs);
		    info.CreateNoWindow = true;
		    info.UseShellExecute = false;
		    info.RedirectStandardOutput = true;
		    info.RedirectStandardError = true;
		    p = Process.Start(info);
                    if(p == null)
                    {
                        Console.Error.WriteLine(progName + ": cannot start `" + slice2cs + " " + cmdArgs + "'");
                        Environment.Exit(1);
                    }
		    Thread t1 = new Thread (new ThreadStart(RedirectStandardOutput));
		    Thread t2 = new Thread(new ThreadStart(RedirectStandardError));
		    t1.Start();
		    t2.Start();
		    p.WaitForExit();
                    int rc = p.ExitCode;
                    p.Close();
		    t1.Join();
		    t2.Join();
		    Environment.Exit(rc);
		}
		Environment.Exit(0);
	    }
	    catch(System.Exception ex)
	    {
	        Console.Error.WriteLine(ex);
		Environment.Exit(1);
	    }
	}

	static volatile Process p;

	private static void RedirectStandardOutput()
	{
	    string output = p.StandardOutput.ReadToEnd();
	    Console.Out.Write(output);
	    Console.Out.Flush();
	}

	private static void RedirectStandardError()
	{
	    string output = p.StandardError.ReadToEnd();
	    Console.Error.Write(output);
	    Console.Error.Flush();
	}
    }
}	    
