// Copyright (c) ZeroC, Inc.

namespace Ice;

/// <summary>Represents an assembly attribute for assemblies that contain Slice generated code.</summary>
// Corresponds to IceRPC's ZeroC.Slice.SliceAttribute.
[AttributeUsage(AttributeTargets.Assembly, AllowMultiple = true)]
public sealed class SliceAttribute : Attribute
{
    /// <summary>Gets the name of the file that contains the Slice definitions.</summary>
    public string SourceFileName { get; }

    /// <summary>Constructs a new instance of <see cref="SliceAttribute" />.</summary>
    /// <param name="sourceFileName">The name of the source file.</param>
    public SliceAttribute(string sourceFileName) => SourceFileName = sourceFileName;
}
