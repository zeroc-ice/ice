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
	
    sealed class _AMD_Object_ice_invoke : IceInternal.IncomingAsync, AMD_Object_ice_invoke
    {
	public _AMD_Object_ice_invoke(IceInternal.Incoming inc)
	    : base(inc)
	{
	}
	
	public void ice_response(bool ok, byte[] outParams)
	{
	    try
	    {
		__os().writeBlob(outParams);
	    }
	    catch(Ice.LocalException ex)
	    {
		__exception(ex);
		return;
	    }
	    
	    __response(ok);
	}
	
	public void ice_exception(System.Exception ex)
	{
	    __exception(ex);
	}
    }
	
}
