// Copyright (c) ZeroC, Inc.

#nullable enable

using System.Reflection;

namespace Ice;

/// <summary>Implements <see cref="SliceLoader" /> by searching for Slice classes in the specified assemblies and their
/// referenced assemblies.</summary>
public sealed class AssemblySliceLoader : SliceLoader
{
    private readonly Internal.Activator _activator;

    /// <summary>Initializes a new instance of the <see cref="AssemblySliceLoader"/> class.</summary>
    /// <param name="assemblies">The assemblies to search.</param>
    /// <remarks>The Slice classes defined in assemblies referenced by <paramref name="assemblies" /> are included as
    /// well, recursively, provided they contain generated code (as determined by the presence of the
    /// <see cref="SliceAttribute" /> attribute). </remarks>
    public AssemblySliceLoader(params Assembly[] assemblies) =>
        _activator = Internal.Activator.Merge(assemblies.Select(Internal.ActivatorFactory.Instance.Get));

    /// <inheritdoc />
    public object? createInstance(string typeId) => _activator.CreateInstance(typeId);
}
