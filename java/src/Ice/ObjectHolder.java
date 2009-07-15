// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Holder class for Ice objects that are out- or inout-parameters.
 **/
public final class ObjectHolder
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
        this.value = value;
    }

    /**
     * This class allows the unmarshaling code to patch the reference
     * to the Ice object in this holder once the Ice object has been unmarshaled.
     **/
    public class Patcher implements IceInternal.Patcher
    {
	/**
	 * Sets the Ice object of this holder to the passed instance.
	 *
	 * @param v The new object for this holder.
	 **/
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
        public String
        type()
        {
            return Ice.ObjectImpl.ice_staticId();
        }
    }

    /**
     * Returns the patcher for this Ice object.
     *
     * @return The patcher for this Ice object.
     **/
    public Patcher
    getPatcher()
    {
        return new Patcher();
    }

    /**
     * The Ice object stored by this holder.
     **/
    public Ice.Object value;
}
