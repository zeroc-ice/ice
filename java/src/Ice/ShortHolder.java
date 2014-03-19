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
 * Holder class for shorts that are out- or inout-parameters.
 **/
public final class ShortHolder
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
     * The <code>short</code> value for this holder.
     **/
    public
    ShortHolder(short value)
    {
        this.value = value;
    }

    /**
     *
     * The <code>short</code> value stored by this holder.
     **/
    public short value;
}
