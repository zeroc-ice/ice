// Copyright (c) ZeroC, Inc.

using Microsoft.Build.Framework;
using Microsoft.Build.Utilities;
using System.Collections.Generic;
using System.IO;
using ZeroC.Ice.Slice.Tools.Common;

namespace ZeroC.Ice.Slice.Tools.Cpp;

public class Slice2CppTask : SliceCompilerTask
{
    protected override string ToolName => "slice2cpp.exe";

    public string HeaderOutputDir { get; set; } = "";

    [Required]
    public string HeaderExt { get; set; } = "h";

    [Required]
    public string SourceExt { get; set; } = "cpp";

    public string BaseDirectoryForGeneratedInclude { get; set; } = "";

    protected override string GeneratedExtensions => string.Format("{0},{1}", HeaderExt, SourceExt);

    protected override ITaskItem[] GeneratedItems(ITaskItem source) =>
        new ITaskItem[]
        {
            new TaskItem(GetGeneratedPath(source, OutputDir, SourceExt)),
            new TaskItem(GetGeneratedPath(
                source,
                string.IsNullOrEmpty(HeaderOutputDir) ? OutputDir : HeaderOutputDir,
                HeaderExt)),
        };

    protected override string GenerateCommandLineCommands()
    {
        var builder = new CommandLineBuilder(false);

        builder.AppendSwitch("--header-ext");
        builder.AppendFileNameIfNotNull(HeaderExt);

        builder.AppendSwitch("--source-ext");
        builder.AppendFileNameIfNotNull(SourceExt);

        if (!string.IsNullOrEmpty(BaseDirectoryForGeneratedInclude))
        {
            builder.AppendSwitch("--include-dir");
            builder.AppendFileNameIfNotNull(BaseDirectoryForGeneratedInclude);
        }
        builder.AppendTextUnquoted(" ");
        builder.AppendTextUnquoted(base.GenerateCommandLineCommands());

        return builder.ToString();
    }

    protected override Dictionary<string, string> GetOptions(ITaskItem source)
    {
        var options = base.GetOptions(source);
        if (!string.IsNullOrEmpty(HeaderOutputDir))
        {
            options["HeaderOutputDir"] = HeaderOutputDir;
        }
        if (!string.IsNullOrEmpty(BaseDirectoryForGeneratedInclude))
        {
            options["BaseDirectoryForGeneratedInclude"] = BaseDirectoryForGeneratedInclude;
        }
        options["SourceExt"] = SourceExt;
        options["HeaderExt"] = HeaderExt;
        return options;
    }

    protected override void TraceGenerated()
    {
        var headerOutputDir = string.IsNullOrEmpty(HeaderOutputDir) ? OutputDir : HeaderOutputDir;
        foreach (ITaskItem source in Sources)
        {
            var cppSource = GetGeneratedPath(source, OutputDir, SourceExt);
            var cppHeader = GetGeneratedPath(source, headerOutputDir, HeaderExt);
            Log.LogMessage(MessageImportance.High,
                            string.Format("Compiling {0} Generating -> {1} and {2}",
                                            source.GetMetadata("Identity"),
                                            TaskUtil.MakeRelative(WorkingDirectory, cppSource),
                                            TaskUtil.MakeRelative(WorkingDirectory, cppHeader)));
        }
    }

    protected override int ExecuteTool(string pathToTool, string responseFileCommands, string commandLineCommands)
    {
        int status = base.ExecuteTool(pathToTool, responseFileCommands, commandLineCommands);
        if (status == 0)
        {
            //
            // If HeaderOutputDir is set move the generated header to its final location
            //
            if (!string.IsNullOrEmpty(HeaderOutputDir))
            {
                foreach (ITaskItem source in Sources)
                {
                    string sourceH = GetGeneratedPath(source, OutputDir, HeaderExt);
                    string targetH = GetGeneratedPath(source, HeaderOutputDir, HeaderExt);
                    if (File.Exists(targetH))
                    {
                        File.Delete(targetH);
                    }
                    File.Move(sourceH, targetH);
                }
            }
        }
        return status;
    }
}
