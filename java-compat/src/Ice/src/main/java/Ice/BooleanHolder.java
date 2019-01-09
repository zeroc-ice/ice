// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package Ice;

/**
 * Holder class for booleans that are out- or inout-parameters.
 **/
public final class BooleanHolder extends Holder<Boolean>
{
    /**
     * Instantiates the class with the value <code>false</code>.
     **/
    public
    BooleanHolder()
    {
    }

    /**
     * Instantiates the class with the passed value.
     *
     * @param value The <code>boolean</code> value stored by this holder.
     **/
    public
    BooleanHolder(boolean value)
    {
        super(value);
    }
}
