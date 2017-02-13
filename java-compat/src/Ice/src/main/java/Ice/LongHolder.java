// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Holder class for longs that are out- or inout-parameters.
 **/
public final class LongHolder extends Holder<Long>
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
     *
     * @param value The <code>long</code> value for this holder.
     **/
    public
    LongHolder(long value)
    {
        super(value);
    }
}
