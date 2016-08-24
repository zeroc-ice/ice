// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
