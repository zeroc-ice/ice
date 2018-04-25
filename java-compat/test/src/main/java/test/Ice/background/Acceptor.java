// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.background;

class Acceptor implements IceInternal.Acceptor
{
    @Override
    public java.nio.channels.ServerSocketChannel
    fd()
    {
        return _acceptor.fd();
    }

    @Override
    public void setReadyCallback(IceInternal.ReadyCallback callback)
    {
        // No need to for the ready callback.
    }

    @Override
    public void
    close()
    {
        _acceptor.close();
    }

    @Override
    public IceInternal.EndpointI
    listen()
    {
        _endpoint = _endpoint.endpoint(_acceptor.listen());
        return _endpoint;
    }

    @Override
    public IceInternal.Transceiver
    accept()
    {
        return new Transceiver(_configuration, _acceptor.accept());
    }

    @Override
    public String
    protocol()
    {
        return _acceptor.protocol();
    }

    @Override
    public String
    toString()
    {
        return _acceptor.toString();
    }

    @Override
    public String
    toDetailedString()
    {
        return _acceptor.toDetailedString();
    }

    public IceInternal.Acceptor
    delegate()
    {
        return _acceptor;
    }

    Acceptor(EndpointI endpoint, Configuration configuration, IceInternal.Acceptor acceptor)
    {
        _endpoint = endpoint;
        _configuration = configuration;
        _acceptor = acceptor;
    }

    private EndpointI _endpoint;
    final private IceInternal.Acceptor _acceptor;
    private Configuration _configuration;
}
