// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

package Ice;

public class ObjectPrxHelper extends ObjectPrxHelperBase
{
    public static ObjectPrx
    checkedCast(Ice.ObjectPrx b)
    {
	return b;
    }

    public static ObjectPrx
    checkedCast(Ice.ObjectPrx b, String f)
    {
	ObjectPrx d = null;
	if(b != null)
	{
	    Ice.ObjectPrx bb = b.ice_newFacet(f);
	    try
	    {
		boolean ok = bb.ice_isA("::Object");
		assert(ok);
		ObjectPrxHelper h = new ObjectPrxHelper();
		h.__copyFrom(bb);
		d = h;
	    }
	    catch(Ice.FacetNotExistException ex)
	    {
	    }
	}
	return d;
    }

    public static ObjectPrx
    uncheckedCast(Ice.ObjectPrx b)
    {
	return b;
    }

    public static ObjectPrx
    uncheckedCast(Ice.ObjectPrx b, String f)
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
