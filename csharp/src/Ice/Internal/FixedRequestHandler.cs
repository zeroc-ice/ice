// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice.Internal;

internal class FixedRequestHandler : RequestHandler
{
    private readonly Reference _reference;
    private readonly bool _response;
    private readonly ConnectionI _connection;
    private readonly bool _compress;

    public int sendAsyncRequest(ProxyOutgoingAsyncBase outAsync) =>
        outAsync.invokeRemote(_connection, _compress, _response);

    public void asyncRequestCanceled(OutgoingAsyncBase outAsync, LocalException ex) =>
        _connection.asyncRequestCanceled(outAsync, ex);

    public ConnectionI? getConnection() => _connection;

    internal FixedRequestHandler(Reference reference, ConnectionI connection, bool compress)
    {
        _reference = reference;
        _response = _reference.isTwoway;
        _connection = connection;
        _compress = compress;
    }
}
