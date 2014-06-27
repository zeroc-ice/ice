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
    public java.nio.channels.ServerSocketChannel fd()
    {
        return _delegate.fd();
    }

    public void close()
    {
        _delegate.close();
    }

    public void listen()
    {
        _delegate.listen();
    }

    public IceInternal.Transceiver accept()
    {
        //
        // WebSocket handshaking is performed in TransceiverI::initialize, since
        // accept must not block.
        //
        return new TransceiverI(_instance, _delegate.accept());
    }

    public String protocol()
    {
        return _delegate.protocol();
    }

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
