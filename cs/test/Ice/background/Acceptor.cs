// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Net.Sockets;

internal class Acceptor : IceInternal.Acceptor
{
    public Socket fd()
    {
        return _acceptor.fd();
    }

    public void close()
    {
        _acceptor.close();
    }

    public void listen()
    {
        _acceptor.listen();
    }

    public IAsyncResult beginAccept(AsyncCallback callback, object state)
    {
        return _acceptor.beginAccept(callback, state);
    }

    public IceInternal.Transceiver endAccept(IAsyncResult result)
    {
        return new Transceiver(_acceptor.endAccept(result));
    }

    public override string ToString()
    {
        return _acceptor.ToString();
    }

    internal Acceptor(IceInternal.Acceptor acceptor)
    {
        _acceptor = acceptor;
    }

    private IceInternal.Acceptor _acceptor;
}
