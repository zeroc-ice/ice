// Copyright (c) ZeroC, Inc.

using Microsoft.Build.Framework;
using Microsoft.Build.Utilities;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Xml;

namespace Ice.Slice.Tools;

public abstract class SliceDependTask : Task
{
    [Required]
    public ITaskItem[] Sources { get; set; } = Array.Empty<ITaskItem>();

    [Required]
    public string IceHome { get; set; } = "";

    [Required]
    public string IceToolsPath { get; set; } = "";

    [Required]
    public string WorkingDirectory { get; set; } = "";

    [Output]
    public ITaskItem[] ComputedSources { get; private set; } = Array.Empty<ITaskItem>();

    [Output]
    public string[] GeneratedCompiledPaths { get; private set; } = Array.Empty<string>();

    protected abstract string ToolName { get; }

    abstract protected ITaskItem[] GeneratedItems(ITaskItem source);

    // Same as generated items but only returns the generated items that need to be compiled
    // for example it excludes C++ headers
    protected abstract ITaskItem[] GeneratedCompiledItems(ITaskItem source);

    protected virtual string GetGeneratedPath(ITaskItem item, string outputDir, string ext) =>
        Path.Combine(
            outputDir,
            Path.GetFileName(Path.ChangeExtension(item.GetMetadata("Identity"), ext)));

    public virtual Dictionary<string, string> GetOptions(ITaskItem item)
    {
        var options = new Dictionary<string, string>
        {
            ["IceHome"] = IceHome,
            ["IceToolsPath"] = IceToolsPath,
            ["OutputDir"] = item.GetMetadata("OutputDir").TrimEnd('\\')
        };

        var value = item.GetMetadata("IncludeDirectories");
        if (!string.IsNullOrEmpty(value))
        {
            value = value.Trim(';');
            if (!string.IsNullOrEmpty(value))
            {
                options["IncludeDirectories"] = value;
            }
        }
        value = item.GetMetadata("AdditionalOptions");
        if (!string.IsNullOrEmpty(value))
        {
            value = value.Trim(';');
            if (!string.IsNullOrEmpty(value))
            {
                options["AdditionalOptions"] = value;
            }
        }
        return options;
    }

    public override bool Execute()
    {
        var computed = new List<ITaskItem>();
        var generatedCompiledPaths = new List<string>();
        foreach (ITaskItem source in Sources)
        {
            bool skip = true;
            Log.LogMessage(MessageImportance.Low,
                            string.Format("Computing dependencies for {0}", source.GetMetadata("Identity")));
            var sourceInfo = new FileInfo(source.GetMetadata("FullPath"));
            if (!sourceInfo.Exists)
            {
                Log.LogMessage(MessageImportance.Low,
                                string.Format("Build required because source: {0} doesn't exists",
                                                source.GetMetadata("Identity")));
                skip = false;
            }

            var generatedItems = GeneratedItems(source);

            generatedCompiledPaths.AddRange(
                GeneratedCompiledItems(source).Select(item => item.GetMetadata("FullPath")));
            //
            // Check if the Slice compiler is older than the source file
            //
            var sliceCompiler = new FileInfo(Path.Combine(IceToolsPath, ToolName));
            FileInfo? generatedInfo = null;
            if (skip)
            {
                foreach (ITaskItem item in generatedItems)
                {
                    generatedInfo = new FileInfo(item.GetMetadata("FullPath"));

                    if (generatedInfo.Exists &&
                        sliceCompiler.LastWriteTime.ToFileTime() > generatedInfo.LastWriteTime.ToFileTime())
                    {
                        Log.LogMessage(MessageImportance.Low,
                            string.Format("Build required because target: {0} is older than Slice compiler: {1}",
                                            TaskUtil.MakeRelative(WorkingDirectory, generatedInfo.FullName),
                                            ToolName));
                        skip = false;
                        break;
                    }
                }
            }

            var dependsDoc = new XmlDocument();
            if (skip)
            {
                var dependInfo = new FileInfo(Path.Combine(WorkingDirectory, source.GetMetadata("OutputDir"),
                    string.Format("SliceCompile.{0}.d", Path.GetFileNameWithoutExtension(sourceInfo.Name))));
                //
                // Check that the depend file exists
                //
                if (!dependInfo.Exists)
                {
                    Log.LogMessage(MessageImportance.Low,
                        string.Format("Build required because depend file: {0} doesn't exists",
                                        TaskUtil.MakeRelative(WorkingDirectory, dependInfo.FullName)));
                    skip = false;
                }
                //
                // Check that the depend file is older than the corresponding Slice source
                //
                else if (sourceInfo.LastWriteTime.ToFileTime() > dependInfo.LastWriteTime.ToFileTime())
                {
                    Log.LogMessage(MessageImportance.Low,
                        string.Format("Build required because source: {0} is older than depend file {1}",
                                        source.GetMetadata("Identity"),
                                        TaskUtil.MakeRelative(WorkingDirectory, dependInfo.FullName)));
                    skip = false;
                }
                else
                {
                    try
                    {
                        dependsDoc.Load(dependInfo.FullName);
                    }
                    catch (XmlException)
                    {
                        try
                        {
                            File.Delete(dependInfo.FullName);
                        }
                        catch (IOException)
                        {
                        }
                        Log.LogMessage(MessageImportance.Low,
                            string.Format("Build required because depend file: {0} has some invalid data",
                                            TaskUtil.MakeRelative(WorkingDirectory, dependInfo.FullName)));
                        skip = false;
                    }
                }
            }

            if (skip)
            {
                foreach (ITaskItem item in generatedItems)
                {
                    generatedInfo = new FileInfo(item.GetMetadata("FullPath"));
                    //
                    // Check that the generated file exists
                    //
                    if (!generatedInfo.Exists)
                    {
                        Log.LogMessage(MessageImportance.Low,
                            string.Format("Build required because generated: {0} doesn't exists",
                                            TaskUtil.MakeRelative(WorkingDirectory, generatedInfo.FullName)));
                        skip = false;
                        break;
                    }
                    //
                    // Check that the generated file is older than the corresponding Slice source
                    //
                    else if (sourceInfo.LastWriteTime.ToFileTime() > generatedInfo.LastWriteTime.ToFileTime())
                    {
                        Log.LogMessage(MessageImportance.Low,
                            string.Format("Build required because source: {0} is older than target {1}",
                                            source.GetMetadata("Identity"),
                                            TaskUtil.MakeRelative(WorkingDirectory, generatedInfo.FullName)));
                        skip = false;
                        break;
                    }
                }
            }

            if (skip)
            {
                XmlNodeList options = dependsDoc.DocumentElement.SelectNodes(
                    string.Format("/dependencies/source[@name='{0}']/options/child::node()",
                                    source.GetMetadata("Identity")));
                if (options != null)
                {
                    var newOptions = GetOptions(source);
                    var oldOptions = options.Cast<XmlNode>().Select(node => new
                    {
                        node.Name,
                        node.InnerXml
                    }).ToDictionary(t => t.Name, t => t.InnerXml);

                    if (newOptions.Except(oldOptions).Any() || oldOptions.Except(newOptions).Any())
                    {
                        Log.LogMessage(MessageImportance.Low,
                                        string.Format("Build required because source: {0} build options change",
                                                        source.GetMetadata("Identity")));
                        skip = false;
                    }
                }
            }

            if (skip)
            {
                XmlNodeList depends = dependsDoc.DocumentElement.SelectNodes(
                    string.Format("/dependencies/source[@name='{0}']/dependsOn", source.GetMetadata("Identity")));

                if (depends != null)
                {
                    var inputs = new List<string>();
                    foreach (XmlNode depend in depends)
                    {
                        string path = depend.Attributes["name"].Value;
                        var dependencyInfo = new FileInfo(path);
                        if (!dependencyInfo.Exists)
                        {
                            Log.LogMessage(MessageImportance.Low,
                                string.Format("Build required because dependency: {0} doesn't exists",
                                                TaskUtil.MakeRelative(WorkingDirectory, dependencyInfo.FullName)));
                            skip = false;
                            break;
                        }
                        else if (dependencyInfo.LastWriteTime > generatedInfo!.LastWriteTime)
                        {
                            Log.LogMessage(MessageImportance.Low,
                                string.Format("Build required because source: {0} is older than target: {1}",
                                                source.GetMetadata("Identity"),
                                                TaskUtil.MakeRelative(WorkingDirectory, dependencyInfo.FullName)));
                            skip = false;
                            break;
                        }

                        inputs.Add(Path.GetFullPath(depend.Attributes["name"].Value).ToUpper());
                    }
                    inputs.Add(source.GetMetadata("FullPath").ToUpper());
                    inputs.Add(sliceCompiler.FullName.ToUpper());

                    var outputs = GeneratedItems(source).Select(item => item.GetMetadata("FullPath").ToUpper());
                    source.SetMetadata("Outputs", string.Join(";", outputs));
                    source.SetMetadata("Inputs", string.Join(";", inputs));
                }
            }

            if (skip)
            {
                string message = string.Format("Skipping {0} -> ", source.GetMetadata("Identity"));
                message += generatedItems[0].GetMetadata("Identity");
                if (generatedItems.Length > 1)
                {
                    message += " and ";
                    message += generatedItems[1].GetMetadata("Identity");
                    message += " are ";
                }
                else
                {
                    message += " is ";
                }
                message += "up to date";

                Log.LogMessage(MessageImportance.Normal, message);
            }

            ITaskItem computedSource = new TaskItem(source.ItemSpec);
            source.CopyMetadataTo(computedSource);
            computedSource.SetMetadata("BuildRequired", skip ? "False" : "True");
            computedSource.SetMetadata("OutputDir", computedSource.GetMetadata("OutputDir").TrimEnd('\\'));
            computed.Add(computedSource);
        }
        ComputedSources = computed.ToArray();
        GeneratedCompiledPaths = generatedCompiledPaths.ToArray();
        return true;
    }
}
