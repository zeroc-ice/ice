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

public abstract class Blobject extends Ice.ObjectImpl
{
    // Returns true if ok, false if user exception.
    public abstract boolean
    ice_invoke(byte[] inParams, ByteSeqHolder outParams, Current current);

    public IceInternal.DispatchStatus
    __dispatch(IceInternal.Incoming in, Current current)
    {
        byte[] inParams;
        ByteSeqHolder outParams = new ByteSeqHolder();
        int sz = in.is().getReadEncapsSize();
        inParams = in.is().readBlob(sz);
        boolean ok = ice_invoke(inParams, outParams, current);
        in.os().writeBlob(outParams.value);
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
