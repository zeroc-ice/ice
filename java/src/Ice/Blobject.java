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
    public abstract byte[]
    ice_invoke(byte[] inParams, Current current);

    public IceInternal.DispatchStatus
    __dispatch(IceInternal.Incoming in, Current current)
    {
        byte[] inParams;
        byte[] outParams;
        int sz = in.is().getReadEncapsSize();
        inParams = in.is().readBlob(sz);
        outParams = ice_invoke(inParams, current);
        in.os().writeBlob(outParams);
        return IceInternal.DispatchStatus.DispatchOK;
    }
}
