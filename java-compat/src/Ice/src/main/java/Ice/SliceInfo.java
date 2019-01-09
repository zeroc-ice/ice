// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package Ice;

/**
 * SliceInfo encapsulates the details of a slice for an unknown class or exception type.
 **/
public class SliceInfo
{
    /**
     * The Slice type ID for this slice.
     **/
    public String typeId;

    /**
     * The Slice compact type ID for this slice.
     **/
    public int compactId;

    /**
     * The encoded bytes for this slice, including the leading size integer.
     **/
    public byte[] bytes;

    /**
     * The class instances referenced by this slice.
     **/
    public Ice.Object[] instances;

    /**
     * Whether or not the slice contains optional members.
     **/
    public boolean hasOptionalMembers;

    /**
     * Whether or not this is the last slice.
     **/
    public boolean isLastSlice;
}
