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
    public struct ProxyIdentityComparer : IEqualityComparer<IObjectPrx>, IComparer<IObjectPrx>
    {
        /// <summary>
        /// Computes a hash value based on the object identity of the proxy.
        /// </summary>
        /// <param name="obj">The proxy whose hash value to compute.</param>
        /// <returns>The hash value for the proxy based on the identity.</returns>
        public int GetHashCode(IObjectPrx obj)
        {
            int h = 5381;
            IceInternal.HashUtil.hashAdd(ref h, obj.Identity);
            return h;
        }

        /// Compares two proxies for equality.
        /// <param name="lhs">A proxy to compare.</param>
        /// <param name="rhs">A proxy to compare.</param>
        /// <returns>True if the passed proxies have the same object
        /// identity; false, otherwise.</returns>
        public bool Equals(IObjectPrx lhs, IObjectPrx rhs)
        {
            return Compare(lhs, rhs) == 0;
        }

        /// Compares two proxies using the object identity for comparison.
        /// <param name="lhs">A proxy to compare.</param>
        /// <param name="rhs">A proxy to compare.</param>
        /// <returns>&lt; 0 if obj1 is less than obj2; &gt; 0 if obj1 is greater than obj2;
        /// 0, otherwise.</returns>
        public int Compare(IObjectPrx? lhs, IObjectPrx? rhs)
        {
            if (ReferenceEquals(lhs, rhs))
            {
                return 0;
            }

            if (lhs == null)
            {
                return -1;
            }

            if (rhs == null)
            {
                return 1;
            }

            Identity lhsIdentity = lhs.Identity;
            Identity rhsIdentity = rhs.Identity;
            int n = string.CompareOrdinal(lhsIdentity.name, rhsIdentity.name);
            if (n != 0)
            {
                return n;
            }
            return string.CompareOrdinal(lhsIdentity.category, rhsIdentity.category);
        }
    }

    /// <summary>
    /// This class allows a proxy to be used as the key for a hashed collection.
    /// The GetHashCode, Equals, and Compare methods are based on the object identity and
    /// the facet of the proxy.
    /// </summary>
    public struct ProxyIdentityFacetComparer : IEqualityComparer<IObjectPrx>, IComparer<IObjectPrx>
    {
        /// <summary>
        /// Computes a hash value based on the object identity and facet of the proxy.
        /// </summary>
        /// <param name="obj">The proxy whose hash value to compute.</param>
        /// <returns>The hash value for the proxy based on the identity and facet.</returns>
        public int GetHashCode(IObjectPrx obj)
        {
            int h = 5381;
            IceInternal.HashUtil.hashAdd(ref h, obj.Identity);
            IceInternal.HashUtil.hashAdd(ref h, obj.Facet);
            return h;
        }

        /// Compares two proxies for equality.
        /// <param name="lhs">A proxy to compare.</param>
        /// <param name="rhs">A proxy to compare.</param>
        /// <returns>True if the passed proxies have the same object
        /// identity and facet; false, otherwise.</returns>
        public bool Equals(IObjectPrx lhs, IObjectPrx rhs)
        {
            return Compare(lhs, rhs) == 0;
        }

        /// Compares two proxies using the object identity and facet for comparison.
        /// <param name="lhs">A proxy to compare.</param>
        /// <param name="rhs">A proxy to compare.</param>
        /// <returns>&lt; 0 if obj1 is less than obj2; &gt; 0 if obj1 is greater than obj2;
        /// 0, otherwise.</returns>
        public int Compare(IObjectPrx? lhs, IObjectPrx? rhs)
        {
            if (ReferenceEquals(lhs, rhs))
            {
                return 0;
            }

            if (lhs == null)
            {
                return -1;
            }

            if (rhs == null)
            {
                return 1;
            }

            Identity lhsIdentity = lhs.Identity;
            Identity rhsIdentity = rhs.Identity;
            int n = string.CompareOrdinal(lhsIdentity.name, rhsIdentity.name);
            if (n != 0)
            {
                return n;
            }
            n = string.CompareOrdinal(lhsIdentity.category, rhsIdentity.category);
            if (n != 0)
            {
                return n;
            }
            return string.CompareOrdinal(lhs.Facet, rhs.Facet);
        }
    }
}
