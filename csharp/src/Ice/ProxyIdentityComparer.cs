//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace ZeroC.Ice
{
    /// <summary>A ProxyIdentityComparer allows you to use proxies as keys of a Dictionary, while using only the
    /// proxies' object identity for equality and hashing.</summary>
    public struct ProxyIdentityComparer : IEqualityComparer<IObjectPrx>
    {
        /// <summary>Computes a hash value based on the object identity of the proxy.</summary>
        /// <param name="obj">The proxy.</param>
        /// <returns>The hash value for the proxy based on the identity.</returns>
        public int GetHashCode(IObjectPrx obj) => obj.Identity.GetHashCode();

        /// <summary>Compares two proxies for equality based on their identity.</summary>
        /// <param name="lhs">The proxy.</param>
        /// <param name="rhs">The other proxy.</param>
        /// <returns>True if the proxies have the same object identity; false, otherwise.</returns>
        public bool Equals(IObjectPrx? lhs, IObjectPrx? rhs)
        {
            if (ReferenceEquals(lhs, rhs))
            {
                return true;
            }

            if (lhs == null || rhs == null)
            {
                return false;
            }

            return lhs.Identity.Equals(rhs.Identity);
        }
    }

    /// <summary>A ProxyIdentityFacetComparer allows you to use proxies as keys of a Dictionary, while using only the
    /// proxies' object identity and facet for equality and hashing.</summary>
    public struct ProxyIdentityFacetComparer : IEqualityComparer<IObjectPrx>
    {
        /// <summary>Computes a hash value based on the object identity and facet of the proxy.</summary>
        /// <param name="obj">The proxy.</param>
        /// <returns>The hash value for the proxy based on the identity and facet.</returns>
        public int GetHashCode(IObjectPrx obj) => System.HashCode.Combine(obj.Identity, obj.Facet);

        /// <summary>Compares two proxies for equality based on their identity and facet.</summary>
        /// <param name="lhs">The proxy.</param>
        /// <param name="rhs">The other proxy.</param>
        /// <returns>True if the proxies have the same object identity and facet; false, otherwise.</returns>
        public bool Equals(IObjectPrx? lhs, IObjectPrx? rhs)
        {
            if (ReferenceEquals(lhs, rhs))
            {
                return true;
            }

            if (lhs == null || rhs == null)
            {
                return false;
            }

            return lhs.Identity.Equals(rhs.Identity) && lhs.Facet.Equals(rhs.Facet);
        }
    }
}
