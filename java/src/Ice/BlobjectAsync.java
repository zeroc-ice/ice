// **********************************************************************
//
// Copyright (c) 2002
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

public abstract class BlobjectAsync extends Ice.ObjectImpl
{
    // Returns true if ok, false if user exception.
    public abstract boolean
    ice_invoke_async(AMD_Object_ice_invoke cb, byte[] inParams, Current current);

    public IceInternal.DispatchStatus
    __dispatch(IceInternal.Incoming in, Current current)
    {
        byte[] inParams;
	int sz = in.is().getReadEncapsSize();
        inParams = in.is().readBlob(sz);
	AMD_Object_ice_invoke cb = new _AMD_Object_ice_invoke(in);
	try
	{
	    ice_invoke_async(cb, inParams, current);
	}
	catch(java.lang.Exception ex)
	{
	    cb.ice_exception(ex);
	}
	return IceInternal.DispatchStatus.DispatchOK;
    }
}
