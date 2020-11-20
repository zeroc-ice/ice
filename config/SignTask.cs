//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.IO;
using System.Threading;
using Microsoft.Build.Framework;
using Microsoft.Build.Utilities;

public class SignTask : Task
{
    [Required]
    public string WorkingDirectory
    {
        get;
        set;
    }

    [Required]
    public ITaskItem[] Files
    {
        get;
        set;
    }

    public string AdditionalOptions
    {
        get;
        set;
    }

    protected string GenerateCommandLineCommands()
    {
        CommandLineBuilder builder = new(false);
        builder.AppendSwitch("sign");
        if (AdditionalOptions != null)
        {
            builder.AppendTextUnquoted(" ");
            builder.AppendTextUnquoted(AdditionalOptions);
        }
        builder.AppendFileNamesIfNotNull(Files, " ");
        return builder.ToString();
    }

    public override bool Execute()
    {
        string commandLineCommands = GenerateCommandLineCommands();
        int status = 0;
        string output = "";
        string error = "";
        int nRetries = 0;
        while (nRetries++ < 10)
        {
            output = "";
            error = "";
            status = RunCommand(WorkingDirectory, "signtool.exe", commandLineCommands, ref output, ref error);
            if (status != 0 && error.IndexOf("timestamp server") != -1)
            {
                Thread.Sleep(10);
                continue;
            }
            break;
        }

        if (status == 0)
        {
            Log.LogMessage(MessageImportance.High, output.Trim());
        }
        else
        {
            Log.LogError(error.Trim());
        }

        return status == 0;
    }

    public class StreamReader
    {
        public string Output { get; private set; }

        public string Error { get; private set; }

        public void ouput(object sendingProcess, DataReceivedEventArgs outLine)
        {
            if (outLine.Data != null)
            {
                Output += outLine.Data + "\n";
            }
        }

        public void error(object sendingProcess, DataReceivedEventArgs outLine)
        {
            if (outLine.Data != null)
            {
                Error += outLine.Data + "\n";
            }
        }
    }

    public static int RunCommand(string workingDir, string command, string args, ref string output, ref string error)
    {
        Process process = new();
        process.StartInfo.FileName = command;
        process.StartInfo.Arguments = args;
        process.StartInfo.CreateNoWindow = true;
        process.StartInfo.UseShellExecute = false;
        process.StartInfo.RedirectStandardError = true;
        process.StartInfo.RedirectStandardOutput = true;
        process.StartInfo.WorkingDirectory = workingDir;

        StreamReader streamReader = new();
        process.OutputDataReceived += new DataReceivedEventHandler(streamReader.ouput);
        process.ErrorDataReceived += new DataReceivedEventHandler(streamReader.error);

        try
        {
            process.Start();
            process.BeginOutputReadLine();
            process.BeginErrorReadLine();
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
