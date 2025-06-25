// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * This class wraps a proxy to allow it to be used the key for a hashed collection. The <code>
 * hashCode</code> and <code>equals</code> methods are based on the object identity of the proxy.
 *
 * @see Util#proxyIdentityCompare
 * @see Util#proxyIdentityAndFacetCompare
 * @see ProxyIdentityFacetKey
 */
public class ProxyIdentityKey {
    /**
     * Initializes this class with the passed proxy.
     *
     * @param proxy The proxy for this instance.
     */
    public ProxyIdentityKey(ObjectPrx proxy) {
        _proxy = proxy;

        //
        // Cache the identity and its hash code.
        //
        _identity = proxy.ice_getIdentity();
        int h = 5381;
        h = HashUtil.hashAdd(h, _identity);
        _hashCode = h;
    }

    /**
     * Computes a hash value based on the object identity of the proxy.
     *
     * @return The hash value.
     */
    @Override
    public int hashCode() {
        return _hashCode;
    }

    /**
     * Compares this proxy with the passed object for equality.
     *
     * @param obj The object to compare this proxy with.
     * @return <code>true</code> if the passed object is a proxy with the same object identity;
     *     <code>
     *     false</code>, otherwise.
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
     * Returns the proxy associated with this key.
     *
     * @return The proxy.
     */
    public ObjectPrx getProxy() {
        return _proxy;
    }

    private final ObjectPrx _proxy;
    private final Identity _identity;
    private final int _hashCode;
}
