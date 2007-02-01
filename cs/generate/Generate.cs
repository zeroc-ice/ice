// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
            try
            {

                string progName = AppDomain.CurrentDomain.FriendlyName;
                if(args.Length < 3)
                {
                    Console.Error.WriteLine("usage: {0} solution_dir project_dir slice_name [args]",
                        progName);
                    Environment.Exit(1);
                }

                const string slice2csName = "slice2cs";
                string solDir = args[0];
                string projDir = args[1];
                string sliceName = args[2];
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
                ArrayList sliceFiles = getSliceFiles(sliceDir);

                string includes = "";
                if(Directory.Exists(Path.Combine(solDir, "slice")))
                {
                    includes = "-I" + handlePathSpaces(Path.Combine(solDir, "slice"));
                }
                if(Directory.Exists(Path.Combine(iceHome, "slice")))
                {
                    includes += " -I" + handlePathSpaces(Path.Combine(iceHome, "slice"));
                }

                if(sliceFiles.Count == 0)
                {
                    sliceDir = Path.Combine(Path.Combine(solDir, "slice"), sliceName);
                    if(Directory.Exists(sliceDir))
                    {
                        sliceFiles = getSliceFiles(sliceDir);
                    }
                }
                if(sliceFiles.Count == 0)
                {
                    sliceDir = Path.Combine(Path.Combine(iceHome, "slice"), sliceName);
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
                        slice2cs = handlePathSpaces(Path.Combine(Path.Combine(iceHome, "bin"), slice2csName));
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
                    cmdArgs += " " + handlePathSpaces(args[i]);
                }

                bool needCompile = false;
                foreach(string sliceFile in sliceFiles)
                {
                    DateTime sliceTime = File.GetLastWriteTime(sliceFile);
                    string csFile = Path.Combine(outputDir, Path.ChangeExtension(Path.GetFileName(sliceFile), ".cs"));
                    if(!File.Exists(csFile) || sliceTime > File.GetLastWriteTime(csFile))
                    {
                        cmdArgs += " " + handlePathSpaces(sliceFile);
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
            catch(System.ComponentModel.Win32Exception ex)
            {
                if(ex.NativeErrorCode == 2)
                {
                    Console.Error.WriteLine("Cannot find slice2cs.exe: set ICE_HOME or add slice2cs.exe to your PATH");
                    Environment.Exit(1);
                }
                else
                {
                    Console.Error.WriteLine(ex);
                }
            }
            catch(System.Exception ex)
            {
                Console.Error.WriteLine(ex);
                Environment.Exit(1);
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

        private static string handlePathSpaces(string dir)
        {
            if(dir.IndexOf(' ') != -1)
            {
                return "\"" + dir + "\"";
            }
            return dir;
        }
    }
}           
