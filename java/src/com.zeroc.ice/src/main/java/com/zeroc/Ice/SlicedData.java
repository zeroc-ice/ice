// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Holds class slices that cannot be unmarshaled because their types are not known locally. */
public final class SlicedData {
    /**
     * Constructs a SlicedData from the given array of slices.
     *
     *  @param slices the slices of the unknown class, in order of most-derived to least-derived
     */
    public SlicedData(SliceInfo[] slices) {
        this.slices = slices;
    }

    /** The slices of the unknown class, in order of most-derived to least-derived. */
    public final SliceInfo[] slices;
}
