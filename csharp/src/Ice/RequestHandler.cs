// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

        int sendAsyncRequest(ProxyOutgoingAsyncBase @out);

        Reference getReference();

        Ice.ConnectionI getConnection();
    }
}
