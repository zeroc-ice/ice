// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceWS;

final class AcceptorI implements IceInternal.Acceptor
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
    public void listen()
    {
        _delegate.listen();
    }

    @Override
    public IceInternal.Transceiver accept()
    {
        //
        // WebSocket handshaking is performed in TransceiverI::initialize, since
        // accept must not block.
        //
        return new TransceiverI(_instance, _delegate.accept());
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

    AcceptorI(Instance instance, IceInternal.Acceptor del)
    {
        _instance = instance;
        _delegate = del;
    }

    private Instance _instance;
    private IceInternal.Acceptor _delegate;
}
