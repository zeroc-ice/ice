// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice.Internal;

/// <summary>Implements Slice loader over a lazily-created SliceLoader.</summary>
internal sealed class LazySliceLoader : SliceLoader
{
    private readonly Lazy<SliceLoader> _lazy;

    /// <inheritdoc />
    public object? createInstance(string typeId) => _lazy.Value.createInstance(typeId);

    internal LazySliceLoader(Func<SliceLoader> factory) => _lazy = new(factory);
}
