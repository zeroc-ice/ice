// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

import Ice.Instrumentation.InvocationObserver;

public interface RequestHandler
{
    void prepareBatchRequest(BasicStream out)
        throws RetryException;
    void finishBatchRequest(BasicStream out);
    void abortBatchRequest();

    boolean sendRequest(OutgoingMessageCallback out)
        throws RetryException;

    int sendAsyncRequest(OutgoingAsyncMessageCallback out)
        throws RetryException;

    void requestTimedOut(OutgoingMessageCallback out);
    void asyncRequestTimedOut(OutgoingAsyncMessageCallback outAsync);

    Reference getReference();

    Ice.ConnectionI getConnection(boolean wait);
}
