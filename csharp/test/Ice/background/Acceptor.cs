// Copyright (c) ZeroC, Inc.

internal class Acceptor : Ice.Internal.Acceptor
{
    public void close()
    {
        _acceptor.close();
    }

    public Ice.Internal.EndpointI listen()
    {
        _endpoint = _endpoint.endpoint(_acceptor.listen());
        return _endpoint;
    }

    public bool startAccept(Ice.Internal.AsyncCallback callback, object state)
    {
        return _acceptor.startAccept(callback, state);
    }

    public void finishAccept()
    {
        _acceptor.finishAccept();
    }

    public Ice.Internal.Transceiver accept()
    {
        return new Transceiver(_acceptor.accept());
    }

    public string protocol()
    {
        return _acceptor.protocol();
    }

    public override string ToString()
    {
        return _acceptor.ToString();
    }

    public string toDetailedString()
    {
        return _acceptor.toDetailedString();
    }

    public Ice.Internal.Acceptor getDelegate()
    {
        return _acceptor;
    }

    internal Acceptor(EndpointI endpoint, Ice.Internal.Acceptor acceptor)
    {
        _endpoint = endpoint;
        _acceptor = acceptor;
    }

    private EndpointI _endpoint;
    private readonly Ice.Internal.Acceptor _acceptor;
}
