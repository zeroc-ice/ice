// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * Implements SliceLoader by combining multiple SliceLoaders.
 */
public final class CompositeSliceLoader implements SliceLoader {
    private final List<SliceLoader> _loaders;

    /**
     * Creates a CompositeSliceLoader that combines the given SliceLoaders.
     *
     * @param loaders The initial Slice loaders.
     */
    public CompositeSliceLoader(SliceLoader... loaders) {
        _loaders = new ArrayList<>(Arrays.asList(loaders));
    }

    /**
     * Adds a SliceLoader to this CompositeSliceLoader.
     *
     * @param loader The SliceLoader to add.
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
