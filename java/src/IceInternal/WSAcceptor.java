// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
    public EndpointI listen(EndpointI endp)
    {
        WSEndpoint p = (WSEndpoint)endp;
        EndpointI endpoint = _delegate.listen(p.delegate());
        return endp.endpoint(this);
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

    WSAcceptor(ProtocolInstance instance, IceInternal.Acceptor del)
    {
        _instance = instance;
        _delegate = del;
    }

    private ProtocolInstance _instance;
    private IceInternal.Acceptor _delegate;
}
