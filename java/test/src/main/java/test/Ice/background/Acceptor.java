// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.background;

class Acceptor implements com.zeroc.IceInternal.Acceptor
{
    @Override
    public java.nio.channels.ServerSocketChannel fd()
    {
        return _acceptor.fd();
    }

    @Override
    public void setReadyCallback(com.zeroc.IceInternal.ReadyCallback callback)
    {
        // No need to for the ready callback.
    }

    @Override
    public void close()
    {
        _acceptor.close();
    }

    @Override
    public com.zeroc.IceInternal.EndpointI listen()
    {
        _endpoint = _endpoint.endpoint(_acceptor.listen());
        return _endpoint;
    }

    @Override
    public com.zeroc.IceInternal.Transceiver accept()
    {
        return new Transceiver(_configuration, _acceptor.accept());
    }

    @Override
    public String protocol()
    {
        return _acceptor.protocol();
    }

    @Override
    public String toString()
    {
        return _acceptor.toString();
    }

    @Override
    public String toDetailedString()
    {
        return _acceptor.toDetailedString();
    }

    public com.zeroc.IceInternal.Acceptor delegate()
    {
        return _acceptor;
    }

    Acceptor(EndpointI endpoint, Configuration configuration, com.zeroc.IceInternal.Acceptor acceptor)
    {
        _endpoint = endpoint;
        _configuration = configuration;
        _acceptor = acceptor;
    }

    private EndpointI _endpoint;
    final private com.zeroc.IceInternal.Acceptor _acceptor;
    private Configuration _configuration;
}
