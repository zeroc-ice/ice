//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace ZeroC.Ice
{
    /// <summary>Represents a <see cref="IObjectPrx">object proxy</see>comparison operation that uses specific rules.
    /// The property <see cref="EqualityComparer{T}.Default"/> can be used to get a comparer that delegates to the
    /// object proxy <see cref="System.IEquatable{T}"/> implementation, the <see cref="Identity"/> and
    /// <see cref="IdentityAndFacet"/> properties provide ProxyComparer objects that use different rules.</summary>
    public abstract class ProxyComparer : EqualityComparer<IObjectPrx>
    {
        /// <summary>Gets a <see cref="ProxyComparer"/> that compare proxies based on the proxies' object identity.
        /// </summary>
        public static ProxyComparer Identity { get; } = new IdentityComparer();

        /// <summary>Gets a <see cref="ProxyComparer"/> that compare proxies, based on the proxies' object identity
        /// and facet.</summary>
        public static ProxyComparer IdentityAndFacet { get; } = new IdentityAndFacetComparer();

        private class IdentityComparer : ProxyComparer
        {
            public override int GetHashCode(IObjectPrx obj) => obj.Identity.GetHashCode();

            public override bool Equals(IObjectPrx? lhs, IObjectPrx? rhs)
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

        private class IdentityAndFacetComparer : ProxyComparer
        {
            public override int GetHashCode(IObjectPrx obj) => System.HashCode.Combine(obj.Identity, obj.Facet);

            public override bool Equals(IObjectPrx? lhs, IObjectPrx? rhs)
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
}