// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.background;

class Acceptor implements IceInternal.Acceptor
{
    public java.nio.channels.ServerSocketChannel
    fd()
    {
        return _acceptor.fd();
    }

    public void
    close()
    {
        _acceptor.close();
    }

    public void
    listen()
    {
        _acceptor.listen();
    }

    public IceInternal.Transceiver
    accept()
    {
        return new Transceiver(_configuration, _acceptor.accept());
    }

    public String
    toString()
    {
        return _acceptor.toString();
    }

    Acceptor(Configuration configuration, IceInternal.Acceptor acceptor)
    {
        _configuration = configuration;
        _acceptor = acceptor;
    }

    final private IceInternal.Acceptor _acceptor;
    private Configuration _configuration;
}
