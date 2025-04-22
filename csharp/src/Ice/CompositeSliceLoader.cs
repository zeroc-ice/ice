// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>Implements <see cref="SliceLoader" /> by combining multiple Slice loaders.</summary>
public sealed class CompositeSliceLoader : SliceLoader
{
    private readonly List<SliceLoader> _loaders;

    /// <summary>Initializes a new instance of the <see cref="CompositeSliceLoader" /> class.</summary>
    /// <param name="loaders">The initial Slice loaders to combine.</param>
    public CompositeSliceLoader(params SliceLoader[] loaders) => _loaders = new List<SliceLoader>(loaders);

    /// <summary>Adds a <see cref="SliceLoader" /> to this <see cref="CompositeSliceLoader" />.</summary>
    /// <param name="loader">The Slice loader to add.</param>
    public void add(SliceLoader loader) => _loaders.Add(loader);

    /// <inheritdoc />
    public object? newInstance(string typeId)
    {
        foreach (SliceLoader loader in _loaders)
        {
            if (loader.newInstance(typeId) is object instance)
            {
                return instance;
            }
        }
        return null;
    }
}
