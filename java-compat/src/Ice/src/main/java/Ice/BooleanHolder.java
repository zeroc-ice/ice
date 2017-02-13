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
