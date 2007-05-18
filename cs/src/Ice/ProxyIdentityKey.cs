// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Globalization;

namespace Ice
{

    //
    // These classes are custom hash code providers and comparers, so proxies can be
    // inserted into hashed or ordered collections using just the identity, or
    // the identity and the facet name as the hash key.
    //

    public class ProxyIdentityKey : System.Collections.IEqualityComparer, System.Collections.IComparer
    {
        public int GetHashCode(object obj)
        {
            return ((Ice.ObjectPrx)obj).ice_getIdentity().GetHashCode();
        }

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

        public int Compare(object obj1, object obj2)
        {
            if(obj1 != null && !(obj1 is Ice.ObjectPrx))
            {
                throw new System.ArgumentException("Argument must be derived from Ice.ObjectPrx", "o1");
            }
            if(obj2 != null && !(obj2 is Ice.ObjectPrx))
            {
                throw new System.ArgumentException("Argument must be derived from Ice.ObjectPrx", "o2");
            }
            return Ice.Util.proxyIdentityCompare((Ice.ObjectPrx)obj1, (Ice.ObjectPrx)obj2);
        }
    }

    public class ProxyIdentityFacetKey : System.Collections.IEqualityComparer, System.Collections.IComparer
    {
        public int GetHashCode(object obj)
        {
            Ice.ObjectPrx o = (Ice.ObjectPrx)obj;
            Ice.Identity identity = o.ice_getIdentity();
            string facet = o.ice_getFacet();
            return 5 * identity.GetHashCode() + facet.GetHashCode();
        }

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

        public int Compare(object obj1, object obj2)
        {
            if(obj1 != null && !(obj1 is Ice.ObjectPrx))
            {
                throw new System.ArgumentException("Argument must be derived from Ice.ObjectPrx", "o1");
            }
            if(obj2 != null && !(obj2 is Ice.ObjectPrx))
            {
                throw new System.ArgumentException("Argument must be derived from Ice.ObjectPrx", "o2");
            }
            return Ice.Util.proxyIdentityAndFacetCompare((Ice.ObjectPrx)obj1, (Ice.ObjectPrx)obj2);
        }
    }

}
