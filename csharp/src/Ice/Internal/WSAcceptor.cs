// Copyright (c) ZeroC, Inc.

namespace Ice.Internal;

internal class WSAcceptor : Acceptor
{
    public void close() => _delegate.close();

    public EndpointI listen()
    {
        _endpoint = _endpoint.endpoint(_delegate.listen());
        return _endpoint;
    }

    public bool startAccept(AsyncCallback callback, object state) => _delegate.startAccept(callback, state);

    public void finishAccept() => _delegate.finishAccept();

    public Transceiver accept() => new WSTransceiver(_instance, _delegate.accept());

    public string protocol() => _delegate.protocol();

    public override string ToString() => _delegate.ToString();

    public string toDetailedString() => _delegate.toDetailedString();

    public Acceptor getDelegate() => _delegate;

    internal WSAcceptor(WSEndpoint endpoint, ProtocolInstance instance, Acceptor del)
    {
        _endpoint = endpoint;
        _instance = instance;
        _delegate = del;
    }

    private WSEndpoint _endpoint;
    private readonly ProtocolInstance _instance;
    private readonly Acceptor _delegate;
}
