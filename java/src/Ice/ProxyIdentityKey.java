// **********************************************************************
//
// Copyright (c) 2004
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

package Ice;

//
// This class wraps a proxy and supplies the necessary methods to allow
// it to be used as a key in a hashed collection. Only the proxy's
// identity is used in comparisons.
//
public class ProxyIdentityKey
{
    public
    ProxyIdentityKey(Ice.ObjectPrx proxy)
    {
        _proxy = proxy;

        //
        // Cache the identity and its hash code.
        //
        _identity = proxy.ice_getIdentity();
        _hashCode = _identity.hashCode();
    }

    public int
    hashCode()
    {
        return _hashCode;
    }

    public boolean
    equals(java.lang.Object obj)
    {
        ProxyIdentityKey other = (ProxyIdentityKey)obj;
        return (_hashCode == other._hashCode) && _identity.equals(other._identity);
    }

    public Ice.ObjectPrx
    getProxy()
    {
        return _proxy;
    }

    final private Ice.ObjectPrx _proxy;
    final private Ice.Identity _identity;
    final private int _hashCode;
}
