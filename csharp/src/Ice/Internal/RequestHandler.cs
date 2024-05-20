// Copyright (c) ZeroC, Inc.

namespace Ice.Internal;

public interface CancellationHandler
{
    void asyncRequestCanceled(OutgoingAsyncBase outAsync, Ice.LocalException ex);
}

public interface RequestHandler : CancellationHandler
{
    int sendAsyncRequest(ProxyOutgoingAsyncBase @out);

    ConnectionI getConnection();
}
