// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Ice;

public abstract class Blobject extends Ice.Object
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
        if (ok)
        {
            return IceInternal.DispatchStatus.DispatchOK;
        }
        else
        {
            return IceInternal.DispatchStatus.DispatchUserException;
        }
    }
}
