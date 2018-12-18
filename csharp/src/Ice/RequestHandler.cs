// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
