// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package com.zeroc.Ice;

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
