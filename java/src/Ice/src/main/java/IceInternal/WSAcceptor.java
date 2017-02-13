// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final class WSAcceptor implements IceInternal.Acceptor
{
    @Override
    public java.nio.channels.ServerSocketChannel fd()
    {
        return _delegate.fd();
    }

    @Override
    public void close()
    {
        _delegate.close();
    }

    @Override
    public EndpointI listen()
    {
        _endpoint = _endpoint.endpoint(_delegate.listen());
        return _endpoint;
    }

    @Override
    public IceInternal.Transceiver accept()
    {
        //
        // WebSocket handshaking is performed in TransceiverI::initialize, since
        // accept must not block.
        //
        return new WSTransceiver(_instance, _delegate.accept());
    }

    @Override
    public String protocol()
    {
        return _delegate.protocol();
    }

    @Override
    public String toString()
    {
        return _delegate.toString();
    }

    @Override
    public String toDetailedString()
    {
        return _delegate.toDetailedString();
    }

    public Acceptor delegate()
    {
        return _delegate;
    }

    WSAcceptor(WSEndpoint endpoint, ProtocolInstance instance, IceInternal.Acceptor del)
    {
        _endpoint = endpoint;
        _instance = instance;
        _delegate = del;
    }

    private WSEndpoint _endpoint;
    private ProtocolInstance _instance;
    private IceInternal.Acceptor _delegate;
}
