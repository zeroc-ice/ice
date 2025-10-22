// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * This class wraps a proxy to allow it to be used the key for a hashed collection.
 * The {@code hashCode} and {@code equals} methods are based on the object identity of the proxy.
 *
 * @see Util#proxyIdentityCompare
 * @see Util#proxyIdentityAndFacetCompare
 * @see ProxyIdentityFacetKey
 */
public class ProxyIdentityKey {
    /**
     * Creates a ProxyIdentityKey from the passed proxy.
     *
     * @param proxy the proxy to use
     */
    public ProxyIdentityKey(ObjectPrx proxy) {
        _proxy = proxy;

        // Cache the identity and compute its hash code.
        _identity = proxy.ice_getIdentity();
        int h = 5381;
        h = HashUtil.hashAdd(h, _identity);
        _hashCode = h;
    }

    /**
     * Returns a hash value based on the object identity of the wrapped proxy.
     *
     * @return the hash value
     */
    @Override
    public int hashCode() {
        return _hashCode;
    }

    /**
     * Compares the wrapped proxy with the provided object for equality.
     *
     * @param obj the object to compare the wrapped proxy with
     * @return {@code true} if the passed object is a proxy with the same object identity; {@code false}, otherwise.
     */
    @Override
    public boolean equals(java.lang.Object obj) {
        if (this == obj) {
            return true;
        }

        if (obj instanceof ProxyIdentityKey) {
            ProxyIdentityKey other = (ProxyIdentityKey) obj;
            return (_hashCode == other._hashCode) && _identity.equals(other._identity);
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
    private final int _hashCode;
}
