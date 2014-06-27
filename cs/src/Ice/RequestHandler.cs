// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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

        bool sendRequest(OutgoingMessageCallback @out);
        bool sendAsyncRequest(OutgoingAsyncMessageCallback @out, out Ice.AsyncCallback cb);

        void requestTimedOut(OutgoingMessageCallback @out);
        void asyncRequestTimedOut(OutgoingAsyncMessageCallback outAsync);

        Reference getReference();

        Ice.ConnectionI getConnection(bool wait);
    }
}
