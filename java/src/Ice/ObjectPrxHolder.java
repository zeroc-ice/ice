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
 * Holder class for proxies that are out- or inout-parameters.
 **/
public final class ObjectPrxHolder
{
    /**
     * Instantiates the class with a <code>null</code> proxy.
     **/
    public
    ObjectPrxHolder()
    {
    }

    /**
     * Instantiates the class with the passed proxy.
     *
     * @param value The proxy stored this holder.
     **/
    public
    ObjectPrxHolder(ObjectPrx value)
    {
        this.value = value;
    }

    /**
     * The proxy stored by this holder.
     **/
    public ObjectPrx value;
}
