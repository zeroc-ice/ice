//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

internal class Acceptor : IceInternal.IAcceptor
{
    public void Close()
    {
        _acceptor.Close();
    }

    public Ice.Endpoint Listen()
    {
        _endpoint = _endpoint.GetEndpoint(_acceptor.Listen());
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

    public string Transport()
    {
        return _acceptor.Transport();
    }

    public override string ToString()
    {
        return _acceptor.ToString();
    }

    public string ToDetailedString()
    {
        return _acceptor.ToDetailedString();
    }

    internal Acceptor(Endpoint endpoint, IceInternal.IAcceptor acceptor)
    {
        _endpoint = endpoint;
        _acceptor = acceptor;
    }

    private Endpoint _endpoint;
    private IceInternal.IAcceptor _acceptor;
}
