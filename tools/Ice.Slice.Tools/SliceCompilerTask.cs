// Copyright (c) ZeroC, Inc.

using Microsoft.Build.Framework;
using Microsoft.Build.Utilities;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Xml;
using System.Xml.Linq;

namespace Ice.Slice.Tools;

public abstract class SliceCompilerTask : ToolTask
{
    [Required]
    public string WorkingDirectory { get; set; } = "";

    [Required]
    public string IceHome { get; set; } = "";

    [Required]
    public string IceToolsPath { get; set; } = "";

    [Required]
    public string OutputDir { get; set; } = "";

    [Required]
    public ITaskItem[] Sources { get; set; } = Array.Empty<ITaskItem>();

    public string[] IncludeDirectories { get; set; } = Array.Empty<string>();

    public string[] AdditionalOptions { get; set; } = Array.Empty<string>();

    [Output]
    public ITaskItem[] ComputedSources { get; private set; } = Array.Empty<ITaskItem>();

    protected override string GetWorkingDirectory() => WorkingDirectory;

    protected virtual string GetGeneratedPath(ITaskItem item, string outputDir, string ext) =>
        Path.Combine(
            outputDir,
            Path.GetFileName(Path.ChangeExtension(item.GetMetadata("Identity"), ext)));

    protected abstract string GeneratedExtensions { get; }

    protected virtual Dictionary<string, string> GetOptions()
    {
        var options = new Dictionary<string, string>
        {
            ["IceHome"] = IceHome,
            ["IceToolsPath"] = IceToolsPath,
            ["OutputDir"] = OutputDir.TrimEnd('\\')
        };

        if (IncludeDirectories != null && IncludeDirectories.Length > 0)
        {
            options["IncludeDirectories"] = string.Join(";", IncludeDirectories);
        }

        if (AdditionalOptions != null && AdditionalOptions.Length > 0)
        {
            options["AdditionalOptions"] = string.Join(";", AdditionalOptions);
        }
        return options;
    }

    protected abstract void TraceGenerated();

    protected override string GenerateCommandLineCommands()
    {
        UsageError = false;
        var builder = new CommandLineBuilder(false);

        if (!string.IsNullOrEmpty(OutputDir))
        {
            builder.AppendSwitch("--output-dir");
            builder.AppendFileNameIfNotNull(OutputDir);
        }

        if (IncludeDirectories != null)
        {
            foreach (string path in IncludeDirectories)
            {
                builder.AppendSwitchIfNotNull("-I", path);
            }
        }
        builder.AppendSwitchIfNotNull("-I", Path.Combine(IceHome, "slice"));

        if (AdditionalOptions != null)
        {
            foreach (var option in AdditionalOptions)
            {
                builder.AppendTextUnquoted(" ");
                builder.AppendTextUnquoted(option);
            }
        }

        builder.AppendFileNamesIfNotNull(Sources, " ");

        return builder.ToString();
    }

    protected abstract ITaskItem[] GeneratedItems(ITaskItem source);

    protected override int ExecuteTool(string pathToTool, string responseFileCommands, string commandLineCommands)
    {
        int status = base.ExecuteTool(pathToTool, responseFileCommands, commandLineCommands);

        if (status == 0)
        {
            TraceGenerated();

            // Run --depend-xml to compute the dependencies
            string output = "";
            string error = "";
            status = TaskUtil.RunCommand(WorkingDirectory, pathToTool, commandLineCommands + " --depend-xml",
                                            ref output, ref error);
            if (status == 0)
            {
                var computed = new List<ITaskItem>();
                var dependsDoc = new XmlDocument();
                dependsDoc.LoadXml(output);

                foreach (ITaskItem source in Sources)
                {
                    var inputs = new List<string>();
                    var depends = dependsDoc.DocumentElement.SelectNodes(
                        string.Format("/dependencies/source[@name='{0}']/dependsOn",
                                        source.GetMetadata("Identity")));
                    if (depends != null)
                    {
                        foreach (XmlNode depend in depends)
                        {
                            inputs.Add(depend.Attributes["name"].Value);
                        }
                    }

                    // Save the dependencies for each source to a dependency file
                    //
                    // Foo.ice -> $(OutputDir)/SliceCompile.Foo.d
                    var doc = new XDocument(
                        new XDeclaration("1.0", "utf-8", "yes"),
                        new XElement("dependencies",
                            new XElement("source", new XAttribute("name", source.GetMetadata("Identity")),
                                inputs.Select(path => new XElement("dependsOn", new XAttribute("name", path))),
                                new XElement("options",
                                    GetOptions().Select(e => new XElement(e.Key, e.Value))))));

                    doc.Save(Path.Combine(OutputDir,
                                            string.Format("SliceCompile.{0}.d", source.GetMetadata("Filename"))));
                    // Update the Inputs and Outputs metadata of the output sources, these info will be use to write
                    // the TLog files used by MSBuild to compute what has to be build.
                    inputs = inputs.Select(path => Path.GetFullPath(path)).ToList();
                    inputs.Add(source.GetMetadata("FullPath").ToUpper());
                    inputs.Add(Path.GetFullPath(pathToTool).ToUpper());

                    ITaskItem computedSource = new TaskItem(source.ItemSpec);
                    source.CopyMetadataTo(computedSource);
                    var outputs = GeneratedItems(source).Select((item) => item.GetMetadata("FullPath").ToUpper());
                    computedSource.SetMetadata("Outputs", string.Join(";", outputs));
                    computedSource.SetMetadata("Inputs", string.Join(";", inputs));
                    computed.Add(computedSource);
                }
                ComputedSources = computed.ToArray();
            }
        }
        return status;
    }

    protected override string GenerateFullPathToTool()
    {
        string path = Path.Combine(IceToolsPath, ToolName);
        if (!File.Exists(path))
        {
            Log.LogError(string.Format("Slice compiler `{0}' not found. Review Ice Home setting", path));
        }
        return path;
    }

    protected override void LogToolCommand(string message)
    {
        Log.LogMessage(MessageImportance.Low, message);
    }

    private bool UsageError { get; set; }

    protected override void LogEventsFromTextOutput(string singleLine, MessageImportance messageImportance)
    {
        if (UsageError)
        {
            return;
        }

        int i = singleLine.IndexOf(string.Format("{0}:", ToolName));
        if (i != -1)
        {
            i += ToolName.Length + 1;
            Log.LogError(
                "",
                "",
                "",
                "",
                0,
                0,
                0,
                0,
                string.Format("{0}: {1}", Path.GetFileName(ToolName), singleLine.Substring(i)));
            UsageError = true;
        }
        else
        {
            string s = singleLine.Trim();
            if (s.StartsWith(WorkingDirectory))
            {
                s = s.Substring(WorkingDirectory.Length);
            }

            string file = "";
            int line = 0;
            string description = "";

            // Skip the drive letter
            i = s.IndexOf(":");
            if (i <= 1 && s.Length > i + 1)
            {
                i = s.IndexOf(":", i + 1);
            }

            if (i != -1)
            {
                file = Path.GetFullPath(s.Substring(0, i).Trim().Trim('"'));
                if (file.IndexOf(WorkingDirectory) != -1)
                {
                    file = file.Substring(WorkingDirectory.Length)
                        .Trim(Path.DirectorySeparatorChar);
                }

                if (s.Length > i + 1)
                {
                    s = s.Substring(i + 1);

                    i = s.IndexOf(":");
                    if (i != -1)
                    {
                        if (int.TryParse(s.Substring(0, i), out line))
                        {
                            if (s.Length > i + 1)
                            {
                                s = s.Substring(i + 1);
                            }
                        }
                        else
                        {
                            s = s.Substring(i);
                        }
                    }

                    description = s.Trim();
                    description += Environment.NewLine;
                }
            }

            if (description.IndexOf("warning:") == 0)
            {
                Log.LogWarning("", "", "", file, line - 1, 0, 0, 0, description.Substring("warning:".Length));
            }
            else if (description.IndexOf("error:") == 0)
            {
                Log.LogError("", "", "", file, line - 1, 0, 0, 0, description.Substring("error:".Length));
            }
            else if (!string.IsNullOrEmpty(description))
            {
                Log.LogError("", "", "", file, line - 1, 0, 0, 0, description);
            }
        }
    }
}
