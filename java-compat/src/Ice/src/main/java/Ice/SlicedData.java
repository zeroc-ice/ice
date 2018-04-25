// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * SlicedData holds the slices of unknown class or exception types.
 **/
public class SlicedData
{
    public SlicedData(SliceInfo[] slices)
    {
        this.slices = slices;
    }

    /**
     * The details of each slice, in order of most-derived to least-derived.
     **/
    public SliceInfo[] slices;
}
