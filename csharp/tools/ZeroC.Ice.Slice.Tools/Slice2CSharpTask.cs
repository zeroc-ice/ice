// Copyright (c) ZeroC, Inc.

using Microsoft.Build.Framework;
using Microsoft.Build.Utilities;

namespace ZeroC.Ice.Slice.Tools;

public class Slice2CSharpTask : Common.SliceCompilerTask
{
    protected override string ToolName => Common.TaskUtil.isWindows ? "slice2cs.exe" : "slice2cs";

    protected override string GeneratedExtensions => "cs";

    protected override void TraceGenerated()
    {
        foreach (ITaskItem source in Sources)
        {
            string message = string.Format("Compiling {0} Generating -> ", source.GetMetadata("Identity"));
            message += Common.TaskUtil.MakeRelative(WorkingDirectory, GetGeneratedPath(source, OutputDir, ".cs"));
            Log.LogMessage(MessageImportance.High, message);
        }
    }

    protected override ITaskItem[] GeneratedItems(ITaskItem source) =>
        new ITaskItem[]
        {
            new TaskItem(GetGeneratedPath(source, OutputDir, ".cs"))
        };
}
