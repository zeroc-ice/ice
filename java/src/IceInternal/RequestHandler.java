// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
        throws LocalExceptionWrapper;
    void finishBatchRequest(BasicStream out);
    void abortBatchRequest();

    Ice.ConnectionI sendRequest(Outgoing out)
        throws LocalExceptionWrapper;

    int sendAsyncRequest(OutgoingAsync out)
        throws LocalExceptionWrapper;

    boolean flushBatchRequests(BatchOutgoing out);
    int flushAsyncBatchRequests(BatchOutgoingAsync out);

    Reference getReference();

    Ice.ConnectionI getConnection(boolean wait);

    Outgoing getOutgoing(String operation, Ice.OperationMode mode, java.util.Map<String, String> context, 
                         InvocationObserver observer)
        throws LocalExceptionWrapper;

    void reclaimOutgoing(Outgoing out);
}
