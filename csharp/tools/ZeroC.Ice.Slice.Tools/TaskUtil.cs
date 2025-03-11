// Copyright (c) ZeroC, Inc.

using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;

namespace ZeroC.Ice.Slice.Tools;

public class TaskUtil
{
    public static readonly bool isWindows = RuntimeInformation.IsOSPlatform(OSPlatform.Windows);

    public static string MakeRelative(string from, string to)
    {
        if (!Path.IsPathRooted(from))
        {
            throw new ArgumentException(string.Format("from: `{0}' must be an absolute path", from));
        }
        else if (!Path.IsPathRooted(to))
        {
            return to;
        }

        string[] firstPathParts =
            Path.GetFullPath(from).Trim(Path.DirectorySeparatorChar).Split(Path.DirectorySeparatorChar);
        string[] secondPathParts =
            Path.GetFullPath(to).Trim(Path.DirectorySeparatorChar).Split(Path.DirectorySeparatorChar);

        int sameCounter = 0;
        while (sameCounter < Math.Min(firstPathParts.Length, secondPathParts.Length) &&
                string.Equals(firstPathParts[sameCounter], secondPathParts[sameCounter],
                            StringComparison.CurrentCultureIgnoreCase))
        {
            ++sameCounter;
        }

        // Different volumes, relative path not possible.
        if (sameCounter == 0)
        {
            return to;
        }

        // Pop back up to the common point.
        string newPath = "";
        for (int i = sameCounter; i < firstPathParts.Length; ++i)
        {
            newPath += ".." + Path.DirectorySeparatorChar;
        }
        // Descend to the target.
        for (int i = sameCounter; i < secondPathParts.Length; ++i)
        {
            newPath += secondPathParts[i] + Path.DirectorySeparatorChar;
        }
        return newPath.TrimEnd(Path.DirectorySeparatorChar);
    }

    public static int RunCommand(string workingDir, string command, string args, ref string output, ref string error)
    {
        var process = new Process();
        process.StartInfo.FileName = command;
        process.StartInfo.Arguments = args;
        process.StartInfo.CreateNoWindow = true;
        process.StartInfo.UseShellExecute = false;
        process.StartInfo.RedirectStandardError = true;
        process.StartInfo.RedirectStandardOutput = true;
        process.StartInfo.WorkingDirectory = workingDir;

        var streamReader = new StreamReader();
        process.OutputDataReceived += new DataReceivedEventHandler(streamReader.WriteOutput);
        process.ErrorDataReceived += new DataReceivedEventHandler(streamReader.WriteError);

        try
        {
            process.Start();

            //
            // When StandardError and StandardOutput are redirected, at least one
            // should use asynchronous reads to prevent deadlocks when calling
            // process.WaitForExit; the other can be read synchronously using ReadToEnd.
            //
            // See the Remarks section in the below link:
            //
            // http://msdn.microsoft.com/en-us/library/system.diagnostics.process.standarderror.aspx
            //

            // Start the asynchronous read of the standard output stream.
            process.BeginOutputReadLine();
            process.BeginErrorReadLine();
            // Read Standard error.
            process.WaitForExit();
            error = streamReader.Error;
            output = streamReader.Output;
            return process.ExitCode;
        }
        catch (Exception ex)
        {
            error = ex.ToString();
            return 1;
        }
    }
}
