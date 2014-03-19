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
 * Holder class for linked lists that are out- or inout-parameters.
 **/
public final class LinkedListHolder
{
    /**
     * Instantiates the class with a <code>null</code> <code>LinkedList</code>.
     **/
    public
    LinkedListHolder()
    {
    }

    /**
     * Instantiates the class with the passed <code>LinkedList</code>.
     *
     * @param value The <code>LinkedList</code> stored by this holder.
     **/
    public
    LinkedListHolder(java.util.LinkedList value)
    {
        this.value = value;
    }

    /**
     * The <code>LinkedList</code> stored by this holder.
     **/
    public java.util.LinkedList value;
}
