// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
	
    using System.Collections;
    using System.Diagnostics;

    public class ObjectPrxHelper : ObjectPrxHelperBase
    {
        public static ObjectPrx checkedCast(Ice.ObjectPrx b)
	{
	    return b;
	}

	public static ObjectPrx checkedCast(Ice.ObjectPrx b, string f)
	{
	    ObjectPrx d = null;
	    if(b != null)
	    {
		try {
		    Ice.ObjectPrx bb = b.ice_newFacet(f);
		    bool ok = bb.ice_isA("::Object");
		    Debug.Assert(ok);
		    ObjectPrxHelper h = new ObjectPrxHelper();
		    h.__copyFrom(bb);
		    d = h;
		}
		catch(Ice.FacetNotExistException)
		{
		}
	    }
	    return d;
	}

	public static ObjectPrx uncheckedCast(Ice.ObjectPrx b)
	{
	    return b;
	}

	public static ObjectPrx uncheckedCast(Ice.ObjectPrx b, string f)
	{
	    ObjectPrx d = null;
	    if(b != null)
	    {
	        Ice.ObjectPrx bb = b.ice_newFacet(f);
		ObjectPrxHelper h = new ObjectPrxHelper();
		h.__copyFrom(bb);
		d = h;
	    }
	    return d;
	}
    }
}
