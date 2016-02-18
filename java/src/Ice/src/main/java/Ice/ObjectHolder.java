// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Holder class for Ice objects that are out- or inout-parameters.
 **/
public final class ObjectHolder extends ObjectHolderBase<Ice.Object>
{
    /**
     * Instantiates the class with a <code>null</code> value.
     **/
    public ObjectHolder()
    {
    }

    /**
     * Instantiates the class with the passed Ice object.
     **/
    public ObjectHolder(Ice.Object value)
    {
        super(value);
    }

    /**
     * Sets the Ice object of this holder to the passed instance.
     *
     * @param v The new object for this holder.
     **/
    public void objectReady(Ice.Object v)
    {
        value = v;
    }
}
