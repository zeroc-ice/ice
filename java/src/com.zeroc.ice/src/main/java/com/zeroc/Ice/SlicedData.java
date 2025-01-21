// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Holds class slices that cannot be unmarshaled because their types are not known locally. */
public final class SlicedData {
    public SlicedData(SliceInfo[] slices) {
        this.slices = slices;
    }

    /** The details of each slice, in order of most-derived to least-derived. */
    public final SliceInfo[] slices;
}
