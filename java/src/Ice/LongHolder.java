// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Holder class for longs that are out- or inout-parameters.
 **/
public final class LongHolder
{
    /**
     * Instantiates the class with the value zero.
     **/
    public
    LongHolder()
    {
    }

    /**
     * Instantiates the class with the passed value.
     **/
    public
    LongHolder(long value)
    {
        this.value = value;
    }

    /**
     * The <code>long</code> value stored by this holder.
     **/
    public long value;
}
