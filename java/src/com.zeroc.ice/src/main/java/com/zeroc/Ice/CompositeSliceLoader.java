// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/** Implements {@link SliceLoader} by combining multiple SliceLoader instances. */
public final class CompositeSliceLoader implements SliceLoader {
    private final List<SliceLoader> _loaders;

    /**
     * Constructs a CompositeSliceLoader that combines the given SliceLoaders.
     *
     * @param loaders the Slice loaders to use
     */
    public CompositeSliceLoader(SliceLoader... loaders) {
        _loaders = new ArrayList<>(Arrays.asList(loaders));
    }

    /**
     * Adds a SliceLoader to this CompositeSliceLoader.
     *
     * @param loader the SliceLoader to add
     */
    public void add(SliceLoader loader) {
        _loaders.add(loader);
    }

    @Override
    public java.lang.Object newInstance(String typeId) {
        for (SliceLoader loader : _loaders) {
            java.lang.Object instance = loader.newInstance(typeId);
            if (instance != null) {
                return instance;
            }
        }
        return null;
    }
}
