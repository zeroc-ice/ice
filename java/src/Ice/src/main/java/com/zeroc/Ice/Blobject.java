// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.Ice;

import java.util.concurrent.CompletionStage;

/**
 * Base class for dynamic dispatch servants. A server application
 * derives a concrete servant class from <code>Blobject</code> that
 * implements the {@link Blobject#ice_invoke} method.
 **/
public interface Blobject extends com.zeroc.Ice.Object
{
    /**
     * Dispatch an incoming request.
     *
     * @param inEncaps The encoded in-parameters for the operation.
     * @param current The Current object to pass to the operation.
     * @return The method returns an instance of <code>Ice_invokeResult</code>.
     * If the operation completed successfully, set the <code>returnValue</code>
     * member to <code>true</code> and the <code>outParams</code> member to
     * the encoded results. If the operation raises a user exception, you can
     * either throw it directly or set the <code>returnValue</code> member to
     * <code>false</code> and the <code>outParams</code> member to the encoded
     * user exception. If the operation raises an Ice run-time exception, it
     * must throw it directly.
     * @throws UserException A user exception can be raised directly and the
     * run time will marshal it.
     **/
    com.zeroc.Ice.Object.Ice_invokeResult ice_invoke(byte[] inEncaps, Current current)
        throws UserException;

    @Override
    default CompletionStage<OutputStream> __dispatch(com.zeroc.IceInternal.Incoming in, Current current)
        throws UserException
    {
        byte[] inEncaps = in.readParamEncaps();
        com.zeroc.Ice.Object.Ice_invokeResult r = ice_invoke(inEncaps, current);
        return in.setResult(in.writeParamEncaps(r.outParams, r.returnValue));
    }
}
