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
 * Holder class for integers that are out- or inout-parameters.
 **/
public final class IntHolder
{
    /**
     * Instantiates the class with the value zero.
     **/
    public
    IntHolder()
    {
    }

    /**
     * Instantiates the class with the passed value.
     *
     * The <code>int</code> value for this holder.
     **/
    public
    IntHolder(int value)
    {
        this.value = value;
    }

    /**
     * The <code>int</code> value stored by this holder.
     **/
    public int value;
}
