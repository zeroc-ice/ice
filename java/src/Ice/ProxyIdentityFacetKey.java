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
// it to be used as a key in a hashed collection. The proxy's identity
// and facet are used in comparisons.
//
public class ProxyIdentityFacetKey
{
    public
    ProxyIdentityFacetKey(Ice.ObjectPrx proxy)
    {
        _proxy = proxy;

        //
        // Cache the identity and facet, and compute the hash code.
        //
        _identity = proxy.ice_getIdentity();
        _facet = proxy.ice_getFacet();
        int h = _identity.hashCode();
        h = 5 * h + _facet.hashCode();
        _hashCode = h;
    }

    public int
    hashCode()
    {
        return _hashCode;
    }

    public boolean
    equals(java.lang.Object obj)
    {
        ProxyIdentityFacetKey other = (ProxyIdentityFacetKey)obj;
        return (_hashCode == other._hashCode) && _identity.equals(other._identity) && _facet.equals(other._facet);
    }

    public Ice.ObjectPrx
    getProxy()
    {
        return _proxy;
    }

    final private Ice.ObjectPrx _proxy;
    final private Ice.Identity _identity;
    final private String _facet;
    final private int _hashCode;
}
