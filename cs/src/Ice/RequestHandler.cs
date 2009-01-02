// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections.Generic;

namespace IceInternal
{
    public interface RequestHandler
    {
        void prepareBatchRequest(BasicStream @out);
        void finishBatchRequest(BasicStream @out);
        void abortBatchRequest();

        Ice.ConnectionI sendRequest(Outgoing @out);

        bool sendAsyncRequest(OutgoingAsync @out);

        bool flushBatchRequests(BatchOutgoing @out);
        bool flushAsyncBatchRequests(BatchOutgoingAsync @out);

        Reference getReference();

        Ice.ConnectionI getConnection(bool wait);

        Outgoing getOutgoing(string operation, Ice.OperationMode mode, Dictionary<string, string> context);

        void reclaimOutgoing(Outgoing @out);
    }
}
