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
namespace Ice
{
	
public abstract class BlobjectAsync : Ice.ObjectImpl
{
    public abstract void ice_invoke_async(AMD_Object_ice_invoke cb, ByteSeq inParams, Current current);
    
    public override IceInternal.DispatchStatus __dispatch(IceInternal.Incoming inc, Current current)
    {
	ByteSeq inParams;
	int sz = inc.istr().getReadEncapsSize();
	inParams = inc.istr().readBlob(sz);
	AMD_Object_ice_invoke cb = new _AMD_Object_ice_invoke(inc);
	try
	{
	    ice_invoke_async(cb, inParams, current);
	}
	catch (System.Exception ex)
	{
	    cb.ice_exception(ex);
	}
	return IceInternal.DispatchStatus.DispatchAsync;
    }
}

}
