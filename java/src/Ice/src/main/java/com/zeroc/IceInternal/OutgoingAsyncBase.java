// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
