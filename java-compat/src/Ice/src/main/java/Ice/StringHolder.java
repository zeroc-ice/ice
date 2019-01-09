// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package Ice;

/**
 * Holder class for strings that are out- or inout-parameters.
 **/
public final class StringHolder extends Holder<String>
{
    /**
     * Instantiates the class with a <code>null</code> string.
     **/
    public
    StringHolder()
    {
    }

    /**
     * Instantiates the class with the passed value.
     *
     * @param value The <code>String</code> value stored by this holder.
     **/
    public
    StringHolder(String value)
    {
        super(value);
    }
}
