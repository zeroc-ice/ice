// Copyright (c) ZeroC, Inc.

using Microsoft.Build.Framework;
using Microsoft.Build.Utilities;

namespace ZeroC.Ice.Slice.Tools.CSharp;

public class Slice2CSharpDependTask : SliceDependTask
{
    protected override ITaskItem[] GeneratedItems(ITaskItem source) =>
        new ITaskItem[]
        {
            new TaskItem(GetGeneratedPath(source, source.GetMetadata("OutputDir"), ".cs")),
        };

    // Same as generated items but only returns the generated items that need to be compiled
    // for example it excludes C++ headers
    protected override ITaskItem[] GeneratedCompiledItems(ITaskItem source) => GeneratedItems(source);

    protected override string ToolName => TaskUtil.isWindows ? "slice2cs.exe" : "slice2cs";
}
