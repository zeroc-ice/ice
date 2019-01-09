// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
