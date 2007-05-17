// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public abstract class Blobject extends Ice.ObjectImpl
{
    // Returns true if ok, false if user exception.
    public abstract boolean
    ice_invoke(byte[] inParams, ByteSeqHolder outParams, Current current);

    public DispatchStatus
    __dispatch(IceInternal.Incoming in, Current current)
    {
        byte[] inParams;
        ByteSeqHolder outParams = new ByteSeqHolder();
        int sz = in.is().getReadEncapsSize();
        inParams = in.is().readBlob(sz);
        boolean ok = ice_invoke(inParams, outParams, current);
        if(outParams.value != null)
        {
            in.os().writeBlob(outParams.value);
        }
        if(ok)
        {
            return DispatchStatus.DispatchOK;
        }
        else
        {
            return DispatchStatus.DispatchUserException;
        }
    }
}
