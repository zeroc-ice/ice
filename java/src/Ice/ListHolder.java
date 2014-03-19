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
 * Holder class for lists that are out- or inout-parameters.
 **/
public final class ListHolder
{
    /**
     * Instantiates the class with a <code>null</code> <code>List</code>.
     **/
    public
    ListHolder()
    {
    }

    /**
     * Instantiates the class with the passed <code>List</code>.
     *
     * @param value The <code>List</code> stored by this holder.
     **/
    public
    ListHolder(java.util.List value)
    {
        this.value = value;
    }

    /**
     * The <code>List</code> stored by this holder.
     **/
    public java.util.List value;
}
