// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Encapsulates the details of a class slice with an unknown type. */
public final class SliceInfo {
    /** The Slice type ID for this slice. It's empty when {@link #compactId} is set (not {@code -1}). */
    public final String typeId;

    /** The Slice compact type ID for this slice. {@code -1} means the slice has no compact ID. */
    public final int compactId;

    /** The encoded bytes for this slice, including the leading size integer. */
    public final byte[] bytes;

    /** The class instances referenced by this slice. */
    public Value[] instances;

    /** Whether or not the slice contains optional members. */
    public final boolean hasOptionalMembers;

    /** Whether or not this is the last slice. */
    public final boolean isLastSlice;

    /**
     * Constructs a SliceInfo instance.
     *
     * @param typeId the Slice type ID for this slice
     * @param compactId the Slice compact type ID for this slice
     * @param bytes the encoded bytes for this slice
     * @param hasOptionalMembers whether or not the slice contains optional members
     * @param isLastSlice whether or not this is the last slice
    */
    public SliceInfo(String typeId, int compactId, byte[] bytes, boolean hasOptionalMembers, boolean isLastSlice) {
        this.typeId = typeId;
        this.compactId = compactId;
        this.bytes = bytes;
        this.hasOptionalMembers = hasOptionalMembers;
        this.isLastSlice = isLastSlice;

        if (compactId == -1) {
            assert (!typeId.isEmpty());
        } else {
            assert (typeId.isEmpty());
        }
    }
}
