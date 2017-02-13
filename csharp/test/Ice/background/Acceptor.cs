// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

internal class Acceptor : IceInternal.Acceptor
{
    public void close()
    {
        _acceptor.close();
    }

    public IceInternal.EndpointI listen()
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

    public IceInternal.Transceiver accept()
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

    public IceInternal.Acceptor getDelegate()
    {
        return _acceptor;
    }

    internal Acceptor(EndpointI endpoint, IceInternal.Acceptor acceptor)
    {
        _endpoint = endpoint;
        _acceptor = acceptor;
    }

    private EndpointI _endpoint;
    private IceInternal.Acceptor _acceptor;
}
