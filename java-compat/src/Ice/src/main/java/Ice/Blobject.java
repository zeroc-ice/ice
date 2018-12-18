// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
     * @param inEncaps The encoded in-parameters for the operation.
     * @param outEncaps The encoded out-paramaters and return value
     * for the operation. The return value follows any out-parameters.
     * @param current The Current object to pass to the operation.
     * @return If the operation completed successfully, the return value
     * is <code>true</code>. If the operation raises a user exception,
     * the return value is <code>false</code>; in this case, <code>outEncaps</code>
     * must contain the encoded user exception. If the operation raises an
     * Ice run-time exception, it must throw it directly.
     *
     * @throws UserException A user exception can be raised directly and the
     * run time will marshal it.
     **/
    public abstract boolean
    ice_invoke(byte[] inEncaps, ByteSeqHolder outEncaps, Current current)
        throws UserException;

    @Override
    public boolean
    _iceDispatch(IceInternal.Incoming in, Current current)
        throws UserException
    {
        byte[] inEncaps;
        ByteSeqHolder outEncaps = new ByteSeqHolder();
        inEncaps = in.readParamEncaps();
        boolean ok = ice_invoke(inEncaps, outEncaps, current);
        in.writeParamEncaps(outEncaps.value, ok);
        return true;
    }
}
