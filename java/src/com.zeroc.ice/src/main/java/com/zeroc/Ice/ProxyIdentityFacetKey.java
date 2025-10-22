// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * This class wraps a proxy to allow it to be used the key for a hashed collection.
 * The {@code hashCode} and {@code equals} methods are based on the object identity and facet of the proxy.
 *
 * @see Util#proxyIdentityAndFacetCompare
 * @see Util#proxyIdentityCompare
 * @see ProxyIdentityKey
 */
public class ProxyIdentityFacetKey {
    /**
     * Creates a ProxyIdentityFacetKey from the passed proxy.
     *
     * @param proxy the proxy to use
     */
    public ProxyIdentityFacetKey(ObjectPrx proxy) {
        _proxy = proxy;

        // Cache the identity and facet, and compute the hash code.
        _identity = proxy.ice_getIdentity();
        _facet = proxy.ice_getFacet();
        int h = 5381;
        h = HashUtil.hashAdd(h, _identity);
        h = HashUtil.hashAdd(h, _facet);
        _hashCode = h;
    }

    /**
     * Returns a hash value based on the object identity and facet of the wrapped proxy.
     *
     * @return The hash value.
     */
    @Override
    public int hashCode() {
        return _hashCode;
    }

    /**
     * Compares the wrapped proxy with the provided object for equality.
     *
     * @param obj the object to compare this proxy with
     * @return {@code true} if the passed object is a proxy with the same object identity and facet;
     *     {@code false}, otherwise.
     */
    @Override
    public boolean equals(java.lang.Object obj) {
        if (this == obj) {
            return true;
        }

        if (obj instanceof ProxyIdentityFacetKey) {
            ProxyIdentityFacetKey other = (ProxyIdentityFacetKey) obj;
            return (_hashCode == other._hashCode)
                && _identity.equals(other._identity)
                && _facet.equals(other._facet);
        }

        return false;
    }

    /**
     * Returns the proxy stored by this class.
     *
     * @return the proxy stored by this class
     */
    public ObjectPrx getProxy() {
        return _proxy;
    }

    private final ObjectPrx _proxy;
    private final Identity _identity;
    private final String _facet;
    private final int _hashCode;
}
