// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

final class AcceptorI implements IceInternal.Acceptor
{
    public java.nio.channels.ServerSocketChannel
    fd()
    {
        return _fd;
    }

    public void
    close()
    {
        if(_instance.networkTraceLevel() >= 1)
        {
            String s = "stopping to accept ssl connections at " + toString();
            _logger.trace(_instance.networkTraceCategory(), s);
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

        if(_instance.networkTraceLevel() >= 1)
        {
            String s = "accepting ssl connections at " + toString();
            _logger.trace(_instance.networkTraceCategory(), s);
        }
    }

    public IceInternal.Transceiver
    accept(int timeout)
    {
        //
        // The plugin may not be fully initialized.
        //
        if(!_instance.initialized())
        {
            Ice.PluginInitializationException ex = new Ice.PluginInitializationException();
            ex.reason = "IceSSL: plugin is not initialized";
            throw ex;
        }

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
                            if(IceInternal.Network.interrupted(ex))
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
                if(IceInternal.Network.interrupted(ex))
                {
                    continue;
                }
                Ice.SocketException se = new Ice.SocketException();
                se.initCause(ex);
                throw se;
            }
        }

        //
        // Check whether this socket is the result of a call to connectToSelf.
        // Despite the fact that connectToSelf immediately closes the socket,
        // the server-side handshake process does not raise an exception.
        // Furthermore, we can't simply proceed with the regular handshake
        // process because we don't want to pass such a socket to the
        // certificate verifier (if any).
        //
        // In order to detect a call to connectToSelf, we compare the remote
        // address of the newly-accepted socket to that in _connectToSelfAddr.
        //
        java.net.SocketAddress remoteAddr = fd.socket().getRemoteSocketAddress();
        synchronized(this)
        {
            if(remoteAddr.equals(_connectToSelfAddr))
            {
                try
                {
                    fd.close();
                }
                catch(java.io.IOException e)
                {
                }
                return null;
            }
        }

        javax.net.ssl.SSLEngine engine = null;
        try
        {
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

            IceInternal.Network.setBlock(fd, false);
            IceInternal.Network.setTcpBufSize(fd, _instance.communicator().getProperties(), _logger);

            engine = _instance.createSSLEngine(true);
        }
        catch(RuntimeException ex)
        {
            try
            {
                fd.close();
            }
            catch(java.io.IOException e)
            {
                // Ignore.
            }
            throw ex;
        }

        if(_instance.networkTraceLevel() >= 1)
        {
            _logger.trace(_instance.networkTraceCategory(), "attempting to accept ssl connection\n" +
                          IceInternal.Network.fdToString(fd));
        }

        return new TransceiverI(_instance, engine, fd, "", true, _adapterName);
    }

    public void
    connectToSelf()
    {
        java.nio.channels.SocketChannel fd = IceInternal.Network.createTcpSocket();
        IceInternal.Network.setBlock(fd, false);
        synchronized(this)
        {
            //
            // connectToSelf is called to wake up the thread blocked in
            // accept. We remember the originating address for use in
            // accept. See accept for details.
            //
            IceInternal.Network.doConnect(fd, _addr, -1);
            _connectToSelfAddr = (java.net.InetSocketAddress)fd.socket().getLocalSocketAddress();
        }
        IceInternal.Network.closeSocket(fd);
    }

    public String
    toString()
    {
        return IceInternal.Network.addrToString(_addr);
    }

    final boolean
    equivalent(String host, int port)
    {
        java.net.InetSocketAddress addr = IceInternal.Network.getAddress(host, port);
        return addr.equals(_addr);
    }

    int
    effectivePort()
    {
        return _addr.getPort();
    }

    AcceptorI(Instance instance, String adapterName, String host, int port)
    {
        _instance = instance;
        _adapterName = adapterName;
        _logger = instance.communicator().getLogger();
        _backlog = 0;

        if(_backlog <= 0)
        {
            _backlog = 5;
        }

        try
        {
            _fd = IceInternal.Network.createTcpServerSocket();
            IceInternal.Network.setBlock(_fd, false);
            IceInternal.Network.setTcpBufSize(_fd, _instance.communicator().getProperties(), _logger);
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
                IceInternal.Network.setReuseAddress(_fd, true);
            }
            _addr = new java.net.InetSocketAddress(host, port);
            if(_instance.networkTraceLevel() >= 2)
            {
                String s = "attempting to bind to ssl socket " + toString();
                _logger.trace(_instance.networkTraceCategory(), s);
            }
            _addr = IceInternal.Network.doBind(_fd, _addr);
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
    private String _adapterName;
    private Ice.Logger _logger;
    private java.nio.channels.ServerSocketChannel _fd;
    private int _backlog;
    private java.net.InetSocketAddress _addr;
    private java.nio.channels.Selector _selector;
    private java.net.InetSocketAddress _connectToSelfAddr;
}
