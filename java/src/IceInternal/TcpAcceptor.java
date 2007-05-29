// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

class TcpAcceptor implements Acceptor
{
    public java.nio.channels.ServerSocketChannel
    fd()
    {
        return _fd;
    }

    public void
    close()
    {
        if(_traceLevels.network >= 1)
        {
            String s = "stopping to accept tcp connections at " + toString();
            _logger.trace(_traceLevels.networkCat, s);
        }

        java.nio.channels.ServerSocketChannel fd;
        java.nio.channels.Selector selector;
        synchronized(this)
        {
            fd = _fd;
            selector = _selector;
            _fd = null;
            _selector = null;
        }
        if(fd != null)
        {
            try
            {
                fd.close();
            }
            catch(java.io.IOException ex)
            {
                // Ignore.
            }
        }
        if(selector != null)
        {
            try
            {
                selector.close();
            }
            catch(java.io.IOException ex)
            {
                // Ignore.
            }
        }
    }

    public void
    listen()
    {
        // Nothing to do.

        if(_traceLevels.network >= 1)
        {
            String s = "accepting tcp connections at " + toString();
            _logger.trace(_traceLevels.networkCat, s);
        }
    }

    public Transceiver
    accept(int timeout)
    {
        java.nio.channels.SocketChannel fd = null;
        while(fd == null)
        {
            try
            {
                fd = _fd.accept();
                if(fd == null)
                {
                    if(_selector == null)
                    {
                        _selector = java.nio.channels.Selector.open();
                    }

                    while(true)
                    {
                        try
                        {
                            java.nio.channels.SelectionKey key =
                                _fd.register(_selector, java.nio.channels.SelectionKey.OP_ACCEPT);
                            if(timeout > 0)
                            {
                                if(_selector.select(timeout) == 0)
                                {
                                    throw new Ice.TimeoutException();
                                }
                            }
                            else if(timeout == 0)
                            {
                                if(_selector.selectNow() == 0)
                                {
                                    throw new Ice.TimeoutException();
                                }
                            }
                            else
                            {
                                _selector.select();
                            }

                            break;
                        }
                        catch(java.io.IOException ex)
                        {
                            if(Network.interrupted(ex))
                            {
                                continue;
                            }
                            Ice.SocketException se = new Ice.SocketException();
                            se.initCause(ex);
                            throw se;
                        }
                    }
                }
            }
            catch(java.io.IOException ex)
            {
                if(Network.interrupted(ex))
                {
                    continue;
                }
                Ice.SocketException se = new Ice.SocketException();
                se.initCause(ex);
                throw se;
            }
        }

        Network.setBlock(fd, false);
        Network.setTcpBufSize(fd, _instance.initializationData().properties, _logger);
        try
        {
            java.net.Socket socket = fd.socket();
            socket.setTcpNoDelay(true);
            socket.setKeepAlive(true);
        }
        catch(java.io.IOException ex)
        {
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }

        if(_traceLevels.network >= 1)
        {
            String s = "accepted tcp connection\n" + Network.fdToString(fd);
            _logger.trace(_traceLevels.networkCat, s);
        }

        return new TcpTransceiver(_instance, fd);
    }

    public void
    connectToSelf()
    {
        java.nio.channels.SocketChannel fd = Network.createTcpSocket();
        Network.setBlock(fd, false);
        Network.doConnect(fd, _addr, -1);
        Network.closeSocket(fd);
    }

    public String
    toString()
    {
        return Network.addrToString(_addr);
    }

    final boolean
    equivalent(String host, int port)
    {
        java.net.InetSocketAddress addr = Network.getAddress(host, port);
        return addr.equals(_addr);
    }

    int
    effectivePort()
    {
        return _addr.getPort();
    }

    TcpAcceptor(Instance instance, String host, int port)
    {
        _instance = instance;
        _traceLevels = instance.traceLevels();
        _logger = instance.initializationData().logger;
        _backlog = 0;

        if(_backlog <= 0)
        {
            _backlog = 5;
        }

        try
        {
            _fd = Network.createTcpServerSocket();
            Network.setBlock(_fd, false);
            Network.setTcpBufSize(_fd, _instance.initializationData().properties, _logger);
            if(!System.getProperty("os.name").startsWith("Windows"))
            {
                //
                // Enable SO_REUSEADDR on Unix platforms to allow
                // re-using the socket even if it's in the TIME_WAIT
                // state. On Windows, this doesn't appear to be
                // necessary and enabling SO_REUSEADDR would actually
                // not be a good thing since it allows a second
                // process to bind to an address even it's already
                // bound by another process.
                //
                // TODO: using SO_EXCLUSIVEADDRUSE on Windows would
                // probably be better but it's only supported by recent
                // Windows versions (XP SP2, Windows Server 2003).
                //
                Network.setReuseAddress(_fd, true);
            }
            _addr = new java.net.InetSocketAddress(host, port);
            if(_traceLevels.network >= 2)
            {
                String s = "attempting to bind to tcp socket " + toString();
                _logger.trace(_traceLevels.networkCat, s);
            }
            _addr = Network.doBind(_fd, _addr);
        }
        catch(RuntimeException ex)
        {
            _fd = null;
            throw ex;
        }
    }

    protected synchronized void
    finalize()
        throws Throwable
    {
        IceUtil.Assert.FinalizerAssert(_fd == null);

        super.finalize();
    }

    private Instance _instance;
    private TraceLevels _traceLevels;
    private Ice.Logger _logger;
    private java.nio.channels.ServerSocketChannel _fd;
    private int _backlog;
    private java.net.InetSocketAddress _addr;
    private java.nio.channels.Selector _selector;
}
