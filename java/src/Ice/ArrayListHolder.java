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
 * Holder class for array lists that are out- or inout-parameters.
 **/
public final class ArrayListHolder
{
    /**
     * Instantiates the class with a <code>null</code> <code>ArrayList</code>.
     **/
    public
    ArrayListHolder()
    {
    }

    /**
     * Instantiates the class with the passed <code>ArrayList</code>.
     *
     * @param value The <code>ArrayList</code> stored by this holder.
     **/
    public
    ArrayListHolder(java.util.ArrayList value)
    {
        this.value = value;
    }

    /**
     * The <code>ArrayList</code> stored by this holder.
     **/
    public java.util.ArrayList value;
}
