// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package com.zeroc.IceInternal;

//
// This non-generic base interface allows us to pass instances without causing "rawtypes" lint warnings.
//
public interface OutgoingAsyncBase
{
    boolean sent();

    boolean completed(com.zeroc.Ice.InputStream is);

    boolean completed(com.zeroc.Ice.Exception ex);

    void attachRemoteObserver(com.zeroc.Ice.ConnectionInfo info, com.zeroc.Ice.Endpoint endpt, int requestId);

    void attachCollocatedObserver(com.zeroc.Ice.ObjectAdapter adapter, int requestId);

    com.zeroc.Ice.OutputStream getOs();

    void invokeSent();

    void invokeSentAsync();

    void invokeCompleted();

    void invokeCompletedAsync();

    void cancelable(CancellationHandler handler);
}
