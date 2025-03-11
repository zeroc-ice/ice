// Copyright (c) ZeroC, Inc.

using Microsoft.Build.Framework;
using Microsoft.Build.Utilities;
using System.Collections.Generic;
using ZeroC.Ice.Slice.Tools.Common;

namespace ZeroC.Ice.Slice.Tools.Cpp;

public class Slice2CppDependTask : SliceDependTask
{
    protected override string ToolName => "slice2cpp.exe";

    public override Dictionary<string, string> GetOptions(ITaskItem item)
    {
        var options = base.GetOptions(item);
        var value = item.GetMetadata("HeaderOutputDir");
        if (!string.IsNullOrEmpty(value))
        {
            options["HeaderOutputDir"] = value;
        }
        value = item.GetMetadata("BaseDirectoryForGeneratedInclude");
        if (!string.IsNullOrEmpty(value))
        {
            options["BaseDirectoryForGeneratedInclude"] = value;
        }
        value = item.GetMetadata("HeaderExt");
        if (!string.IsNullOrEmpty(value))
        {
            options["HeaderExt"] = value;
        }
        value = item.GetMetadata("SourceExt");
        if (!string.IsNullOrEmpty(value))
        {
            options["SourceExt"] = value;
        }
        return options;
    }

    protected override ITaskItem[] GeneratedItems(ITaskItem source)
    {
        var outputDir = source.GetMetadata("OutputDir");
        var headerOutputDir = source.GetMetadata("HeaderOutputDir");
        if (string.IsNullOrEmpty(headerOutputDir))
        {
            headerOutputDir = outputDir;
        }
        var sourceExt = source.GetMetadata("SourceExt");
        var headerExt = source.GetMetadata("HeaderExt");

        return new ITaskItem[]
        {
            new TaskItem(GetGeneratedPath(source, outputDir, sourceExt)),
            new TaskItem(GetGeneratedPath(source, headerOutputDir, headerExt))
        };
    }

    // The generated items that need to be compiled for example the .cpp sources
    protected override ITaskItem[] GeneratedCompiledItems(ITaskItem source)
    {
        var generatedItems = GeneratedItems(source);
        return new ITaskItem[]
        {
            generatedItems[0]
        };
    }
}
