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
	
sealed class _AMD_Object_ice_invoke : IceInternal.IncomingAsync, AMD_Object_ice_invoke
{
    public _AMD_Object_ice_invoke(IceInternal.Incoming inc)
	: base(inc)
    {
    }
    
    public void
    ice_response(bool ok, byte[] outParams)
    {
	try
	{
	    __os().writeBlob(outParams);
	}
	catch(Ice.LocalException ex)
	{
	    __exception(ex);
	    return ;
	}
	
	__response(ok);
    }
    
    public void
    ice_exception(System.Exception ex)
    {
	__exception(ex);
    }
}
	
}
