// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Net.Sockets;

internal class Acceptor : IceInternal.Acceptor
{
    public void close()
    {
        _acceptor.close();
    }

    public void listen()
    {
        _acceptor.listen();
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
