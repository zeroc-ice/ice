//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice.Instrumentation;

/**
 * The invocation observer to instrument invocations on proxies. A
 * proxy invocation can either result in a collocated or remote
 * invocation. If it results in a remote invocation, a sub-observer is
 * requested for the remote invocation.
 **/
public interface InvocationObserver extends Observer
{
    /**
     * Notification of the invocation being retried.
     **/
    void retried();

    /**
     * Notification of a user exception.
     **/
    void userException();

    /**
     * Get a remote observer for this invocation.
     * @param con The connection information.
     * @param endpt The connection endpoint.
     * @param requestId The ID of the invocation.
     * @param size The size of the invocation.
     * @return The observer to instrument the remote invocation.
     **/
    RemoteObserver getRemoteObserver(com.zeroc.Ice.ConnectionInfo con, com.zeroc.Ice.Endpoint endpt, int requestId, int size);

    /**
     * Get a collocated observer for this invocation.
     * @param adapter The object adapter hosting the collocated Ice object.
     * @param requestId The ID of the invocation.
     * @param size The size of the invocation.
     * @return The observer to instrument the collocated invocation.
     **/
    CollocatedObserver getCollocatedObserver(com.zeroc.Ice.ObjectAdapter adapter, int requestId, int size);
}
