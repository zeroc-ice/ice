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
    public interface CancellationHandler
    {
        void asyncRequestCanceled(OutgoingAsyncBase outAsync, Ice.LocalException ex);
    }

    public interface RequestHandler : CancellationHandler
    {
        RequestHandler connect();
        RequestHandler update(RequestHandler previousHandler, RequestHandler newHandler);

        void prepareBatchRequest(BasicStream @out);
        void finishBatchRequest(BasicStream @out);
        void abortBatchRequest();

        bool sendAsyncRequest(OutgoingAsyncBase @out, out Ice.AsyncCallback cb);

        Reference getReference();

        Ice.ConnectionI getConnection();
        Ice.ConnectionI waitForConnection();
    }
}
