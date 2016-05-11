// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    public interface CancellationHandler
    {
        void asyncRequestCanceled(OutgoingAsyncBase outAsync, Ice.LocalException ex);
    }

    public interface RequestHandler : CancellationHandler
    {
        RequestHandler update(RequestHandler previousHandler, RequestHandler newHandler);

        bool sendAsyncRequest(ProxyOutgoingAsyncBase @out, out Ice.AsyncCallback cb);

        Reference getReference();

        Ice.ConnectionI getConnection();
    }
}
