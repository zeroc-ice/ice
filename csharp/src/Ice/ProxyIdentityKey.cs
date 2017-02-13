// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Globalization;

namespace Ice
{
    /// <summary>
    /// This class allows a proxy to be used as the key for a hashed collection.
    /// The GetHashCode, Equals, and Compare methods are based on the object identity
    /// of the proxy.
    /// </summary>
    public class ProxyIdentityKey : System.Collections.IEqualityComparer, System.Collections.IComparer
    {
        /// <summary>
        /// Computes a hash value based on the object identity of the proxy.
        /// </summary>
        /// <param name="obj">The proxy whose hash value to compute.</param>
        /// <returns>The hash value for the proxy based on the identity.</returns>
        public int GetHashCode(object obj)
        {
            int h = 5381;
            IceInternal.HashUtil.hashAdd(ref h, ((ObjectPrx)obj).ice_getIdentity());
            return h;
        }

        /// Compares two proxies for equality.
        /// <param name="obj1">A proxy to compare.</param>
        /// <param name="obj2">A proxy to compare.</param>
        /// <returns>True if the passed proxies have the same object
        /// identity; false, otherwise.</returns>
        public new bool Equals(object obj1, object obj2)
        {
            try
            {
                return Compare(obj1, obj2) == 0;
            }
            catch(System.Exception)
            {
                return false;
            }
        }

        /// Compares two proxies using the object identity for comparison.
        /// <param name="obj1">A proxy to compare.</param>
        /// <param name="obj2">A proxy to compare.</param>
        /// <returns>&lt; 0 if obj1 is less than obj2; &gt; 0 if obj1 is greater than obj2;
        /// 0, otherwise.</returns>
        public int Compare(object obj1, object obj2)
        {
            ObjectPrx proxy1 = obj1 as ObjectPrx;
            if(obj1 != null && proxy1 == null)
            {
                throw new ArgumentException("Argument must be derived from Ice.ObjectPrx", "obj1");
            }

            ObjectPrx proxy2 = obj2 as ObjectPrx;
            if(obj2 != null && proxy2 == null)
            {
                throw new ArgumentException("Argument must be derived from Ice.ObjectPrx", "obj2");
            }
            return Util.proxyIdentityCompare(proxy1, proxy2);
        }
    }

    /// <summary>
    /// This class allows a proxy to be used as the key for a hashed collection.
    /// The GetHashCode, Equals, and Compare methods are based on the object identity and
    /// the facet of the proxy.
    /// </summary>
    public class ProxyIdentityFacetKey : System.Collections.IEqualityComparer, System.Collections.IComparer
    {
        /// <summary>
        /// Computes a hash value based on the object identity and facet of the proxy.
        /// </summary>
        /// <param name="obj">The proxy whose hash value to compute.</param>
        /// <returns>The hash value for the proxy based on the identity and facet.</returns>
        public int GetHashCode(object obj)
        {
            ObjectPrx o = (ObjectPrx)obj;
            Identity identity = o.ice_getIdentity();
            string facet = o.ice_getFacet();
            int h = 5381;
            IceInternal.HashUtil.hashAdd(ref h, identity);
            IceInternal.HashUtil.hashAdd(ref h, facet);
            return h;
        }

        /// Compares two proxies for equality.
        /// <param name="obj1">A proxy to compare.</param>
        /// <param name="obj2">A proxy to compare.</param>
        /// <returns>True if the passed proxies have the same object
        /// identity and facet; false, otherwise.</returns>
        public new bool Equals(object obj1, object obj2)
        {
            try
            {
                return Compare(obj1, obj2) == 0;
            }
            catch(System.Exception)
            {
                return false;
            }
        }

        /// Compares two proxies using the object identity and facet for comparison.
        /// <param name="obj1">A proxy to compare.</param>
        /// <param name="obj2">A proxy to compare.</param>
        /// <returns>&lt; 0 if obj1 is less than obj2; &gt; 0 if obj1 is greater than obj2;
        /// 0, otherwise.</returns>
        public int Compare(object obj1, object obj2)
        {
            ObjectPrx proxy1 = obj1 as ObjectPrx;
            if(obj1 != null && proxy1 == null)
            {
                throw new ArgumentException("Argument must be derived from Ice.ObjectPrx", "obj1");
            }

            ObjectPrx proxy2 = obj2 as ObjectPrx;
            if(obj2 != null && proxy2 == null)
            {
                throw new ArgumentException("Argument must be derived from Ice.ObjectPrx", "obj2");
            }
            return Util.proxyIdentityAndFacetCompare(proxy1, proxy2);
        }
    }

}
