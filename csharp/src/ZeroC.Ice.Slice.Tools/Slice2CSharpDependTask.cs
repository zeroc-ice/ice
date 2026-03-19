// Copyright (c) ZeroC, Inc.

using Microsoft.Build.Framework;
using Microsoft.Build.Utilities;

namespace ZeroC.Ice.Slice.Tools;

public class Slice2CSharpDependTask : Common.SliceDependTask
{
    protected override ITaskItem[] GeneratedItems(ITaskItem source) =>
        IceRpc ?
            new ITaskItem[]
            {
                new TaskItem(GetGeneratedPath(source, source.GetMetadata("OutputDir"), ".cs")),
                new TaskItem(GetGeneratedPath(source, source.GetMetadata("OutputDir"), ".IceRpc.cs"))
            } :
            new ITaskItem[]
            {
                new TaskItem(GetGeneratedPath(source, source.GetMetadata("OutputDir"), ".cs"))
            };

    // Same as generated items but only returns the generated items that need to be compiled
    // for example it excludes C++ headers
    protected override ITaskItem[] GeneratedCompiledItems(ITaskItem source) => GeneratedItems(source);

    protected override string ToolName => Common.TaskUtil.isWindows ? "slice2cs.exe" : "slice2cs";
}
