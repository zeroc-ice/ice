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

// Ice version 1.3.0
// Generated from file `C:\cygwin\home\michi\src\icicle\demo\Ice\latency\Latency.ice'

using _System = System;
using _Microsoft = Microsoft;

public class Ping : Ice.ObjectImpl
{
    #region Slice type-related members

    public static new string[] __ids = 
    {
	"::Ice::Object",
	"::Ping"
    };

    private static readonly Ice.StringSeq __idSeq = new Ice.StringSeq(__ids);

    public override bool ice_isA(string s, Ice.Current __current)
    {
	return _System.Array.BinarySearch(__ids, s, _System.Collections.Comparer.DefaultInvariant) >= 0;
    }

    public override Ice.StringSeq ice_ids(Ice.Current __current)
    {
	return __idSeq;
    }

    public override string ice_id(Ice.Current __current)
    {
	return __ids[1];
    }

    public static new string ice_staticId()
    {
	return __ids[1];
    }

    #endregion

    #region Marshaling support

    public override void __write(IceInternal.BasicStream __os, bool __marshalFacets)
    {
	__os.writeTypeId(ice_staticId());
	__os.startWriteSlice();
	__os.endWriteSlice();
	base.__write(__os, __marshalFacets);
    }

    public override void
    __read(IceInternal.BasicStream __is, bool __rid)
    {
	if(__rid)
	{
	    string myId = __is.readTypeId();
	}
	__is.startReadSlice();
	__is.endReadSlice();
	base.__read(__is, true);
    }

    #endregion
}

public interface PingPrx : Ice.ObjectPrx
{
}

public class PingPrxHelper : Ice.ObjectPrxHelper, PingPrx
{
    protected override Ice.Object_DelM __createDelegateM()
    {
	return new Ping_DelM();
    }

    protected override Ice.Object_DelD __createDelegateD()
    {
	return new Ping_DelD();
    }

    public static PingPrx checkedCast(Ice.ObjectPrx b)
    {
	if(b == null)
	{
	    return null;
	}
	if(b is PingPrx)
	{
	    return (PingPrx)b;
	}
	if(b.ice_isA("::Ping"))
	{
	    PingPrxHelper h = new PingPrxHelper();
	    h.__copyFrom(b);
	    return h;
	}
	return null;
    }

    public static PingPrx checkedCast(Ice.ObjectPrx b, string f)
    {
	if(b == null)
	{
	    return null;
	}
	Ice.ObjectPrx bb = b.ice_appendFacet(f);
	try
	{
	    if(bb.ice_isA("::Ping"))
	    {
		PingPrxHelper h = new PingPrxHelper();
		h.__copyFrom(bb);
		return h;
	    }
	}
	catch(Ice.FacetNotExistException)
	{
	}
	return null;
    }

    public static PingPrx uncheckedCast(Ice.ObjectPrx b)
    {
	if(b == null)
	{
	    return null;
	}
	PingPrxHelper h = new PingPrxHelper();
	h.__copyFrom(b);
	return h;
    }

    public static PingPrx uncheckedCast(Ice.ObjectPrx b, string f)
    {
	if(b == null)
	{
	    return null;
	}
	Ice.ObjectPrx bb = b.ice_appendFacet(f);
	PingPrxHelper h = new PingPrxHelper();
	h.__copyFrom(bb);
	return h;
    }

    public static void __write(IceInternal.BasicStream __os, PingPrx __v)
    {
	__os.writeProxy(__v);
    }

    public static PingPrx __read(IceInternal.BasicStream __is)
    {
	Ice.ObjectPrx proxy = __is.readProxy();
	if(proxy != null)
	{
	    PingPrxHelper result = new PingPrxHelper();
	    result.__copyFrom(proxy);
	    return result;
	}
	return null;
    }
}

public interface Ping_Del : Ice.Object_Del
{
}

public sealed class Ping_DelM : Ice.Object_DelM, Ping_Del
{
}

public sealed class Ping_DelD : Ice.Object_DelD, Ping_Del
{
}