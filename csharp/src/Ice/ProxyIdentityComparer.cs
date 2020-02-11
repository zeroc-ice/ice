//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Ice
{
    /// <summary>
    /// This class allows a proxy to be used as the key for a hashed collection.
    /// The GetHashCode, Equals, and Compare methods are based on the object identity
    /// of the proxy.
    /// </summary>
    public struct ProxyIdentityComparer : IEqualityComparer<IObjectPrx>
    {
        /// <summary>
        /// Computes a hash value based on the object identity of the proxy.
        /// </summary>
        /// <param name="obj">The proxy whose hash value to compute.</param>
        /// <returns>The hash value for the proxy based on the identity.</returns>
        public int GetHashCode(IObjectPrx obj) => obj.Identity.GetHashCode();

        /// <summary>Compares two proxies for equality.</summary>
        /// <param name="lhs">A proxy to compare.</param>
        /// <param name="rhs">A proxy to compare.</param>
        /// <returns>True if the passed proxies have the same object identity; false, otherwise.</returns>
        public bool Equals(IObjectPrx lhs, IObjectPrx rhs) => lhs.Identity.Equals(rhs.Identity);
    }

    /// <summary>
    /// This class allows a proxy to be used as the key for a hashed collection.
    /// The GetHashCode, Equals, and Compare methods are based on the object identity and
    /// the facet of the proxy.
    /// </summary>
    public struct ProxyIdentityFacetComparer : IEqualityComparer<IObjectPrx>
    {
        /// <summary>
        /// Computes a hash value based on the object identity and facet of the proxy.
        /// </summary>
        /// <param name="obj">The proxy whose hash value to compute.</param>
        /// <returns>The hash value for the proxy based on the identity and facet.</returns>
        public int GetHashCode(IObjectPrx obj) => System.HashCode.Combine(obj.Identity, obj.Facet);

        /// <summary>
        /// Compares two proxies for equality.
        /// </summary>
        /// <param name="lhs">A proxy to compare.</param>
        /// <param name="rhs">A proxy to compare.</param>
        /// <returns>True if the passed proxies have the same object identity and facet; false, otherwise.</returns>
        public bool Equals(IObjectPrx lhs, IObjectPrx rhs) =>
            lhs.Identity.Equals(rhs.Identity) && lhs.Facet.Equals(rhs.Facet);

    }
}
