//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

internal class Acceptor : IceInternal.IAcceptor
{
    public void Close()
    {
        _acceptor.Close();
    }

    public IceInternal.Endpoint Listen()
    {
        _endpoint = _endpoint.endpoint(_acceptor.Listen());
        return _endpoint;
    }

    public bool StartAccept(IceInternal.AsyncCallback callback, object state)
    {
        return _acceptor.StartAccept(callback, state);
    }

    public void FinishAccept()
    {
        _acceptor.FinishAccept();
    }

    public IceInternal.ITransceiver Accept()
    {
        return new Transceiver(_acceptor.Accept());
    }

    public string Protocol()
    {
        return _acceptor.Protocol();
    }

    public override string ToString()
    {
        return _acceptor.ToString();
    }

    public string ToDetailedString()
    {
        return _acceptor.ToDetailedString();
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
