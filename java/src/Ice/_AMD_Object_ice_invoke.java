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

final class _AMD_Object_ice_invoke extends IceInternal.IncomingAsync implements AMD_Object_ice_invoke
{
    public
    _AMD_Object_ice_invoke(IceInternal.Incoming in)
    {
	super(in);
    }

    public void
    ice_response(boolean ok, byte[] outParams)
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

    public void
    ice_exception(java.lang.Exception ex)
    {
	__exception(ex);
    }
};
