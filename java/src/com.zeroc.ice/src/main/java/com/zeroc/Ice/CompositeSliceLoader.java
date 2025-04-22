// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.ArrayList;
import java.util.List;

/**
 * Implements SliceLoader by combining multiple SliceLoaders.
 */
final public class CompositeSliceLoader implements SliceLoader {
    private final List<SliceLoader> _loaders = new ArrayList<>();

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
