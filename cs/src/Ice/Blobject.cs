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
