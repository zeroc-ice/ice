// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice.Internal;

public interface CancellationHandler
{
    void asyncRequestCanceled(OutgoingAsyncBase outAsync, LocalException ex);
}

public interface RequestHandler : CancellationHandler
{
    int sendAsyncRequest(ProxyOutgoingAsyncBase outAsync);

    ConnectionI? getConnection();
}
