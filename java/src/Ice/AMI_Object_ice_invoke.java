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

public abstract class AMI_Object_ice_invoke extends IceInternal.OutgoingAsync
{

    public abstract void ice_response(boolean ok, byte[] outParams);
    public abstract void ice_exception(Ice.LocalException ex);

    protected final void __response(boolean ok) // ok == true means no user exception.
    {
	byte[] outParams;
	try
	{
	    IceInternal.BasicStream __is = this.__is();
	    int sz = __is.getReadEncapsSize();
	    outParams = __is.readBlob(sz);
	}
	catch(LocalException ex)
	{
	    ice_exception(ex);
	    return;
	}
	ice_response(ok, outParams);
    }
};
