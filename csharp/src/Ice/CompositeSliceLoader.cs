// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>Implements <see cref="SliceLoader" /> by combining multiple Slice loaders. </summary>
public sealed class CompositeSliceLoader : SliceLoader
{
    private readonly List<SliceLoader> _loaders = new();

    /// <summary>Adds a <see cref="SliceLoader" /> to this <see cref="CompositeSliceLoader" />.</summary>
    /// <param name="loader">The Slice loader to add.</param>
    public void add(SliceLoader loader) => _loaders.Add(loader);

    /// <inheritdoc />
    public object? createInstance(string typeId)
    {
        foreach (var loader in _loaders)
        {
            object? instance = loader.createInstance(typeId);
            if (instance is not null)
            {
                return instance;
            }
        }
        return null;
    }
}
