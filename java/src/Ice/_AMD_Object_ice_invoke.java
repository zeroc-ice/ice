// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
}
