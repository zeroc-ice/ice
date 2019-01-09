// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package Ice;

/**
 * Holder class for doubles that are out- or inout-parameters.
 **/
public final class DoubleHolder extends Holder<Double>
{
    /**
     * Instantiates the class with the value zero.
     **/
    public
    DoubleHolder()
    {
    }

    /**
     * Instantiates the class with the passed value.
     *
     * @param value The <code>double</code> value stored by this holder.
     **/
    public
    DoubleHolder(double value)
    {
        super(value);
    }
}
