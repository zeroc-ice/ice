// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public interface RequestHandler extends CancellationHandler
{
    RequestHandler connect();
    RequestHandler update(RequestHandler previousHandler, RequestHandler newHandler);

    void prepareBatchRequest(BasicStream out)
        throws RetryException;
    void finishBatchRequest(BasicStream out);
    void abortBatchRequest();

    int sendAsyncRequest(OutgoingAsyncBase out)
        throws RetryException;

    Reference getReference();

    Ice.ConnectionI getConnection();
    Ice.ConnectionI waitForConnection()
        throws InterruptedException, RetryException;
}
