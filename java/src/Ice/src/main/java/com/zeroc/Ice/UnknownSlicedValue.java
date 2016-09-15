// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.Ice;

/**
 * Unknown sliced value holds an instance of an unknown Slice class type.
 **/
public final class UnknownSlicedValue extends Value
{
    /**
     * Represents an instance of a Slice class type having the given Slice type.
     *
     * @param unknownTypeId The Slice type ID of the unknown object.
     **/
    public UnknownSlicedValue(String unknownTypeId)
    {
        _unknownTypeId = unknownTypeId;
    }

    /**
     * Determine the Slice type ID associated with this object.
     *
     * @return The type ID.
     **/
    public String getUnknownTypeId()
    {
        return _unknownTypeId;
    }

    @Override
    public void __write(OutputStream __os)
    {
        __os.startValue(_slicedData);
        __os.endValue();
    }

    @Override
    public void __read(InputStream __is)
    {
        __is.startValue();
        _slicedData = __is.endValue(true);
    }

    private final String _unknownTypeId;
    private SlicedData _slicedData;

    public static final long serialVersionUID = 0L;
}
