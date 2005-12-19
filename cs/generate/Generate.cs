// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;
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
            //
            // VS 7.1 can't deal with paths that contain spaces, and double quoting the $(SolutionDir) macro
            // in the pre-build event command line doesn't work. So, sadly, to deal with path names that contain
            // spaces, we have to pass an explicit single quote and then post-process it here.
            //
            int index = 0;
foreach(string a in args)
    Console.WriteLine("arg: " + a);
            string solDir;
            nextArg(args, ref index, out solDir);
            string projDir;
            nextArg(args, ref index, out projDir);
     Console.WriteLine("solDir: " + solDir);
            Console.WriteLine("projDir: " + projDir);
            string projName;
            nextArg(args, ref index, out projName);
	    
            try
	    {
		const string slice2csName = "slice2cs";
		string iceHome = Environment.GetEnvironmentVariable("ICE_HOME");
		if(iceHome == null)
		{
		    iceHome = Path.Combine(Path.Combine(solDir, ".."), "ice");
		    if(!Directory.Exists(iceHome))
		    {
		        iceHome = Path.Combine(Path.Combine(Path.Combine(solDir, ".."), ".."), "ice");
			if(!Directory.Exists(iceHome))
			{
			    iceHome = Path.Combine(solDir, "..");
			}
		    }
		}

		Directory.SetCurrentDirectory(projDir);

		string sliceDir = projDir;
		ArrayList sliceFiles = getSliceFiles(projDir);

		string includes = "";
		if(Directory.Exists(Path.Combine(solDir, "slice")))
		{
		    includes = "-I\"" + Path.Combine(solDir, "slice") + "\"";
		}
		if(Directory.Exists(Path.Combine(iceHome, "slice")))
		{
		    includes += " -I\"" + Path.Combine(iceHome, "slice") + "\"";
		}

		if(sliceFiles.Count == 0)
		{
		    sliceDir = Path.Combine(Path.Combine(solDir, "slice"), projName);
		    if(Directory.Exists(sliceDir))
		    {
			sliceFiles = getSliceFiles(sliceDir);
		    }
		}
		if(sliceFiles.Count == 0)
		{
		    sliceDir = Path.Combine(Path.Combine(iceHome, "slice"), projName);
		    if(Directory.Exists(sliceDir))
		    {
			sliceFiles = getSliceFiles(sliceDir);
		    }
		}
		if(sliceFiles.Count == 0)
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
		string cmdArgs = "--ice -I. " + includes + " --output-dir \"" + outputDir + "\"";

                while(index < args.Length)
                {
                    string arg;
                    nextArg(args, ref index, out arg);
		    if(arg.IndexOf(' ') != -1)
		    {
			cmdArgs += " \"" + arg + "\"";
		    }
		    else
		    {
			cmdArgs += " " + arg;
		    }
		}

		bool needCompile = false;
		foreach(string sliceFile in sliceFiles)
		{
		    DateTime sliceTime = File.GetLastWriteTime(sliceFile);
		    string csFile = Path.Combine(outputDir, Path.ChangeExtension(Path.GetFileName(sliceFile), ".cs"));
		    if(!File.Exists(csFile) || sliceTime > File.GetLastWriteTime(csFile))
		    {
			cmdArgs += " \"" + sliceFile + "\"";
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

        private static string progName = AppDomain.CurrentDomain.FriendlyName;

        private static void usage()
        {
            Console.Error.WriteLine("usage: {0} solution_dir project_dir project_name [args]", progName);
            Environment.Exit(1);
        }

        private static void nextArg(string[] args, ref int index, out string arg)
        {
            if(index >= args.Length)
            {
                usage();
            }

            if(args[index][0] != '\'')
            {
                arg = args[index++];
                return;
            }

            int startIndex = index;
            bool found = false;
            arg = "";
            while(index < args.Length && !found)
            {
                if(index == startIndex)
                {
                    if(args[index][args[index].Length - 1] == '\'')
                    {
                        arg = args[index].Substring(1, args[index].Length - 2);
                        found = true;
                    }
                    else
                    {
                        arg = args[index].Substring(1, args[index].Length - 1);
                    }
                }
                else
                {
                    if(args[index][args[index].Length - 1] == '\'')
                    {
                        arg = arg + " " + args[index].Substring(0, args[index].Length - 1);
                        found = true;
                    }
                    else
                    {
                        arg = arg + " " + args[index];
                    }
                }
                ++index;
            }
        }

        //
        // Return all the files ending in ".ice" in the specified dir.
        // Unfortunately, we can't use Directory.GetFiles(dir, ".ice") for
        // this because, for three-letter file extensions, it returns files
        // with extensions that have three *or more* letters :-(
        //
	private static ArrayList getSliceFiles(string dir)
	{
	    ArrayList result = new ArrayList();
	    string[] files = Directory.GetFiles(dir);
	    foreach(string file in files)
	    {
	        if(file.EndsWith(".ice"))
		{
		    result.Add(file);
		}
	    }
	    return result;
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
