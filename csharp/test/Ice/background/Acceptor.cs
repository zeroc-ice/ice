//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

internal class Acceptor : IceInternal.IAcceptor
{
    public void close()
    {
        _acceptor.close();
    }

    public IceInternal.Endpoint listen()
    {
        _endpoint = _endpoint.endpoint(_acceptor.listen());
        return _endpoint;
    }

    public bool startAccept(IceInternal.AsyncCallback callback, object state)
    {
        return _acceptor.startAccept(callback, state);
    }

    public void finishAccept()
    {
        _acceptor.finishAccept();
    }

    public IceInternal.ITransceiver accept()
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

    public IceInternal.IAcceptor getDelegate()
    {
        return _acceptor;
    }

    internal Acceptor(Endpoint endpoint, IceInternal.IAcceptor acceptor)
    {
        _endpoint = endpoint;
        _acceptor = acceptor;
    }

    private Endpoint _endpoint;
    private IceInternal.IAcceptor _acceptor;
}
