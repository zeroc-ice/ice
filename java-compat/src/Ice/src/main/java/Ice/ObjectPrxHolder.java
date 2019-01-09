// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package Ice;

/**
 * Holder class for proxies that are out- or inout-parameters.
 **/
public final class ObjectPrxHolder extends Holder<ObjectPrx>
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
     * @param value The proxy value for this holder.
     **/
    public
    ObjectPrxHolder(ObjectPrx value)
    {
        super(value);
    }
}
