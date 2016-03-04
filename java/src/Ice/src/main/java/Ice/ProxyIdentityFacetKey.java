// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * This class wraps a proxy to allow it to be used the key for a hashed collection.
 * The <code>hashCode</code> and <code>equals</code> methods are based on the object identity and
 * the facet of the proxy.
 *
 * @see Util#proxyIdentityAndFacetCompare
 * @see Util#proxyIdentityCompare
 * @see ProxyIdentityKey
 *
 **/
public class ProxyIdentityFacetKey
{
    /**
     * Initializes this class with the passed proxy.
     *
     * @param proxy The proxy for this instance.
     **/
    public
    ProxyIdentityFacetKey(Ice.ObjectPrx proxy)
    {
        _proxy = proxy;

        //
        // Cache the identity and facet, and compute the hash code.
        //
        _identity = proxy.ice_getIdentity();
        _facet = proxy.ice_getFacet();
        int h = 5381;
        h = IceInternal.HashUtil.hashAdd(h, _identity);
        h = IceInternal.HashUtil.hashAdd(h, _facet);
        _hashCode = h;
    }

    /**
     * Computes a hash value based on the object identity and the facet of the proxy.
     *
     * @return The hash value.
     **/
    @Override
    public int
    hashCode()
    {
        return _hashCode;
    }

    /**
     * Compares this proxy with the passed object for equality.
     *
     * @param obj The object to compare this proxy with.
     * @return <code>true</code> if the passed object is a proxy with the same object
     * identity and facet as this proxy; <code>false</code>, otherwise.
     **/
    @Override
    public boolean
    equals(java.lang.Object obj)
    {
        if(this == obj)
        {
            return true;
        }

        if(obj instanceof ProxyIdentityFacetKey)
        {
            ProxyIdentityFacetKey other = (ProxyIdentityFacetKey)obj;
            return (_hashCode == other._hashCode) && _identity.equals(other._identity) && _facet.equals(other._facet);
        }

        return false;
    }

    /**
     * Returns the proxy stored by this class.
     *
     * @return The proxy stored by this class.
     **/
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
