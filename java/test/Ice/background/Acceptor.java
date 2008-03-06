// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
        return new Transceiver(_acceptor.accept());
    }

    public String
    toString()
    {
        return _acceptor.toString();
    }

    Acceptor(IceInternal.Acceptor acceptor)
    {
        _acceptor = acceptor;
    }

    protected synchronized void
    finalize()
        throws Throwable
    {
        super.finalize();
    }

    final private IceInternal.Acceptor _acceptor;
}
