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
	
    public abstract class BlobjectAsync : Ice.ObjectImpl
    {
	public abstract void ice_invoke_async(AMD_Object_ice_invoke cb, byte[] inParams, Current current);
	
	public override IceInternal.DispatchStatus __dispatch(IceInternal.Incoming inc, Current current)
	{
	    byte[] inParams;
	    int sz = inc.istr().getReadEncapsSize();
	    inParams = inc.istr().readBlob(sz);
	    AMD_Object_ice_invoke cb = new _AMD_Object_ice_invoke(inc);
	    try
	    {
		ice_invoke_async(cb, inParams, current);
	    }
	    catch(System.Exception ex)
	    {
		cb.ice_exception(ex);
	    }
	    return IceInternal.DispatchStatus.DispatchAsync;
	}
    }

}
