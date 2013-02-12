// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections.Generic;
using Ice.Instrumentation;

namespace IceInternal
{
    public interface RequestHandler
    {
        void prepareBatchRequest(BasicStream @out);
        void finishBatchRequest(BasicStream @out);
        void abortBatchRequest();

        Ice.ConnectionI sendRequest(Outgoing @out);
        bool sendAsyncRequest(OutgoingAsync @out, out Ice.AsyncCallback cb);

        bool flushBatchRequests(BatchOutgoing @out);
        bool flushAsyncBatchRequests(BatchOutgoingAsync @out, out Ice.AsyncCallback cb);

        Reference getReference();

        Ice.ConnectionI getConnection(bool wait);

        Outgoing getOutgoing(string operation, Ice.OperationMode mode, Dictionary<string, string> context, 
                             InvocationObserver observer);

        void reclaimOutgoing(Outgoing @out);
    }
}
