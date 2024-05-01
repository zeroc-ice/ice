// Copyright (c) ZeroC, Inc.

namespace Ice.Internal;

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
