// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Base class for dynamic dispatch servants. A server application
 * derives a concrete servant class from <code>Blobject</code> that
 * implements the {@link Blobject#ice_invoke} method.
 **/
public abstract class Blobject extends Ice.ObjectImpl
{
    /**
     * Dispatch an incoming request.
     *
     * @param inParams The encoded in-parameters for the operation.
     * @param outParams The encoded out-paramaters and return value
     * for the operation. The return value follows any out-parameters.
     * @param current The Current object to pass to the operation.
     * @return If the operation completed successfully, the return value
     * is <code>true</code>. If the operation raises a user exception,
     * the return value is <code>false</code>; in this case, <code>outParams</code>
     * must contain the encoded user exception. If the operation raises an
     * Ice run-time exception, it must throw it directly.
     **/
    public abstract boolean
    ice_invoke(byte[] inParams, ByteSeqHolder outParams, Current current);

    public DispatchStatus
    __dispatch(IceInternal.Incoming in, Current current)
    {
        byte[] inParams;
        ByteSeqHolder outParams = new ByteSeqHolder();
        IceInternal.BasicStream is = in.is();
        is.startReadEncaps();
        int sz = is.getReadEncapsSize();
        inParams = is.readBlob(sz);
        is.endReadEncaps();
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
