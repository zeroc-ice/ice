// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package Ice;

/**
 * Holder class for shorts that are out- or inout-parameters.
 **/
public final class ShortHolder extends Holder<Short>
{
    /**
     * Instantiates the class with the value zero.
     **/
    public
    ShortHolder()
    {
    }

    /**
     * Instantiates the class with the passed value.
     *
     * @param value The <code>short</code> value for this holder.
     **/
    public
    ShortHolder(short value)
    {
        super(value);
    }
}
