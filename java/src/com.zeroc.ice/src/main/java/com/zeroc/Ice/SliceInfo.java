// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Encapsulates the details of a slice with an unknown type. */
public final class SliceInfo {
    /** The Slice type ID for this slice. */
    public final String typeId;

    /** The encoded bytes for this slice, including the leading size integer. */
    public final byte[] bytes;

    /** The class instances referenced by this slice. */
    public Value[] instances;

    /** Whether or not the slice contains optional members. */
    public final boolean hasOptionalMembers;

    /** Whether or not this is the last slice. */
    public final boolean isLastSlice;

    /** The SliceInfo constructor. */
    public SliceInfo(
            String typeId,
            byte[] bytes,
            boolean hasOptionalMembers,
            boolean isLastSlice) {
        this.typeId = typeId;
        this.bytes = bytes;
        this.hasOptionalMembers = hasOptionalMembers;
        this.isLastSlice = isLastSlice;
    }
}
