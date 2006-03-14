// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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

#if ICE_DOTNET_1X
    public class ProxyIdentityKey : System.Collections.IHashCodeProvider, System.Collections.IComparer
#else
    public class ProxyIdentityKey : System.Collections.IEqualityComparer, System.Collections.IComparer
#endif
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
            if(obj1 == null)
            {
                return obj2 == null ? 0 : -1;
            }
            else if(obj2 == null)
            {
                return 1;
            }
            else
            {
                if(!(obj1 is Ice.ObjectPrx))
                {
                    throw new System.ArgumentException("Argument must be derived from Ice.ObjectPrx", "o1");
                }
                if(!(obj2 is Ice.ObjectPrx))
                {
                    throw new System.ArgumentException("Argument must be derived from Ice.ObjectPrx", "o2");
                }
		Ice.Identity i1 = ((Ice.ObjectPrx)obj1).ice_getIdentity();
		Ice.Identity i2 = ((Ice.ObjectPrx)obj2).ice_getIdentity();
		int comp = string.Compare(i1.name, i2.name, false, CultureInfo.InvariantCulture);
		if(comp != 0)
		{
		    return comp;
		}
		return string.Compare(i1.category, i2.category, false, CultureInfo.InvariantCulture);
            }
	}
    }

#if ICE_DOTNET_1X
    public class ProxyIdentityFacetKey : System.Collections.IHashCodeProvider, System.Collections.IComparer
#else
    public class ProxyIdentityFacetKey : System.Collections.IEqualityComparer, System.Collections.IComparer
#endif
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
            if(obj1 == null)
            {
                return obj2 == null ? 0 : -1;
            }
            else if(obj2 == null)
            {
                return 1;
            }
            else
            {
                if(!(obj1 is Ice.ObjectPrx))
                {
                    throw new System.ArgumentException("Argument must be derived from Ice.ObjectPrx", "obj1");
                }
                if(!(obj2 is Ice.ObjectPrx))
                {
                    throw new System.ArgumentException("Argument must be derived from Ice.ObjectPrx", "obj2");
                }
                Ice.ObjectPrx o1 = (Ice.ObjectPrx)obj1;
                Ice.ObjectPrx o2 = (Ice.ObjectPrx)obj2;
                Ice.Identity i1 = o1.ice_getIdentity();
                Ice.Identity i2 = o2.ice_getIdentity();
                int comp = string.Compare(i1.name, i2.name, false, CultureInfo.InvariantCulture);
                if(comp != 0)
                {
                    return comp;
                }
                comp = string.Compare(i1.category, i2.category, false, CultureInfo.InvariantCulture);
                if(comp != 0)
                {
                    return comp;
                }
                string facet1 = o1.ice_getFacet();
                string facet2 = o2.ice_getFacet();
                return string.Compare(facet1, facet2, false, CultureInfo.InvariantCulture);
            }
        }
    }

}
