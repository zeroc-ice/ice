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

    public abstract class Blobject : Ice.ObjectImpl
    {
	// Returns true if ok, false if user exception.
	public abstract bool ice_invoke(byte[] inParams, out byte[] outParams, Current current);
	
	public override IceInternal.DispatchStatus __dispatch(IceInternal.Incoming inc, Current current)
	{
	    byte[] inParams;
	    byte[] outParams;
	    int sz = inc.istr().getReadEncapsSize();
	    inParams = inc.istr().readBlob(sz);
	    bool ok = ice_invoke(inParams, out outParams, current);
	    if(outParams != null)
	    {
		inc.ostr().writeBlob(outParams);
	    }
	    if(ok)
	    {
		return IceInternal.DispatchStatus.DispatchOK;
	    }
	    else
	    {
		return IceInternal.DispatchStatus.DispatchUserException;
	    }
	}
    }

}
