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
 * Holder class for Ice objects that are out- or inout-parameters.
 **/
public final class ObjectHolder extends ObjectHolderBase<Ice.Object>
{
    /**
     * Instantiates the class with a <code>null</code> value.
     **/
    public
    ObjectHolder()
    {
    }

    /**
     * Instantiates the class with the passed Ice object.
     **/
    public
    ObjectHolder(Ice.Object value)
    {
        super(value);
    }

    /**
     * Sets the Ice object of this holder to the passed instance.
     *
     * @param v The new object for this holder.
     **/
    @Override
    public void
    patch(Ice.Object v)
    {
        value = v;
    }

    /**
     * Returns the Slice type ID of the most-derived Slice type supported
     * by this instance.
     *
     * @return The Slice type ID.
     **/
    @Override
    public String
    type()
    {
        return Ice.ObjectImpl.ice_staticId();
    }
}
