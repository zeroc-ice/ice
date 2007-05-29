// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

class AcceptorI implements IceInternal.Acceptor
{
    public java.nio.channels.ServerSocketChannel
    fd()
    {
        return null;
    }

    public void
    close()
    {
        if(_instance.networkTraceLevel() >= 1)
        {
            String s = "stopping to accept ssl connections at " + toString();
            _logger.trace(_instance.networkTraceCategory(), s);
        }

        javax.net.ssl.SSLServerSocket fd;
        synchronized(this)
        {
            fd = _fd;
            _fd = null;
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

        javax.net.ssl.SSLSocket fd = null;
        ConnectionInfo connInfo = null;
        try
        {
            if(timeout == -1)
            {
                timeout = 0; // Infinite
            }
            else if(timeout == 0)
            {
                timeout = 1;
            }
            _fd.setSoTimeout(timeout);
            fd = (javax.net.ssl.SSLSocket)_fd.accept();

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
            java.net.SocketAddress remoteAddr = fd.getRemoteSocketAddress();
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

            fd.setUseClientMode(false);

            //
            // getSession blocks until the initial handshake completes.
            //
            if(timeout == 0)
            {
                fd.getSession();
            }
            else
            {
                HandshakeThread ht = new HandshakeThread(fd);
                ht.start();
                if(!ht.waitForHandshake(timeout))
                {
                    throw new Ice.TimeoutException();
                }
            }

            connInfo = Util.populateConnectionInfo(fd, _adapterName, true);
            _instance.verifyPeer(connInfo, fd, "", true);
        }
        catch(java.net.SocketTimeoutException ex)
        {
            if(fd != null)
            {
                try
                {
                    fd.close();
                }
                catch(java.io.IOException e)
                {
                }
            }
            Ice.TimeoutException e = new Ice.TimeoutException();
            e.initCause(ex);
            throw e;
        }
        catch(javax.net.ssl.SSLException ex)
        {
            if(fd != null)
            {
                try
                {
                    fd.close();
                }
                catch(java.io.IOException e)
                {
                }
            }

            //
            // Unfortunately, the situation where the cipher suite does not match
            // the certificates is not detected until accept is called. If we were
            // to throw a LocalException, the IncomingConnectionFactory would
            // simply log it and call accept again, resulting in an infinite loop.
            // To avoid this problem, we check for the special case and throw
            // an exception that IncomingConnectionFactory doesn't trap.
            //
            if(ex.getMessage().toLowerCase().startsWith("no available certificate corresponds to the ssl cipher " +
                                                        "suites which are enabled"))
            {
                RuntimeException e = new RuntimeException();
                e.initCause(ex);
                throw e;
            }

            Ice.SecurityException e = new Ice.SecurityException();
            e.initCause(ex);
            throw e;
        }
        catch(java.io.IOException ex)
        {
            if(fd != null)
            {
                try
                {
                    fd.close();
                }
                catch(java.io.IOException e)
                {
                }
            }

            if(IceInternal.Network.connectionLost(ex))
            {
                throw new Ice.ConnectionLostException();
            }

            Ice.SocketException e = new Ice.SocketException();
            e.initCause(ex);
            throw e;
        }
        catch(RuntimeException ex)
        {
            if(fd != null)
            {
                try
                {
                    fd.close();
                }
                catch(java.io.IOException e)
                {
                }
            }
            throw ex;
        }

        if(_instance.networkTraceLevel() >= 1)
        {
            String s = "accepted ssl connection\n" + IceInternal.Network.fdToString(fd);
            _logger.trace(_instance.networkTraceCategory(), s);
        }

        if(_instance.securityTraceLevel() > 0)
        {
            _instance.traceConnection(fd, true);
        }

        return new TransceiverI(_instance, fd, connInfo);
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
            javax.net.ssl.SSLServerSocketFactory factory = _instance.context().getServerSocketFactory();
            _addr = new java.net.InetSocketAddress(host, port);
            if(_instance.networkTraceLevel() >= 2)
            {
                String s = "attempting to bind to ssl socket " + toString();
                _logger.trace(_instance.networkTraceCategory(), s);
            }
            java.net.InetSocketAddress iface = IceInternal.Network.getAddress(host, port);
            _fd = (javax.net.ssl.SSLServerSocket)factory.createServerSocket(port, _backlog, iface.getAddress());
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
                _fd.setReuseAddress(true);
            }
            _addr = (java.net.InetSocketAddress)_fd.getLocalSocketAddress();

            int verifyPeer =
                _instance.communicator().getProperties().getPropertyAsIntWithDefault("IceSSL.VerifyPeer", 2);
            if(verifyPeer == 0)
            {
                _fd.setWantClientAuth(false);
                _fd.setNeedClientAuth(false);
            }
            else if(verifyPeer == 1)
            {
                _fd.setWantClientAuth(true);
            }
            else
            {
                _fd.setNeedClientAuth(true);
            }

            String[] cipherSuites =
                _instance.filterCiphers(_fd.getSupportedCipherSuites(), _fd.getEnabledCipherSuites());
            try
            {
                _fd.setEnabledCipherSuites(cipherSuites);
            }
            catch(IllegalArgumentException ex)
            {
                Ice.SecurityException e = new Ice.SecurityException();
                e.reason = "IceSSL: invalid ciphersuite";
                e.initCause(ex);
                throw e;
            }
            if(_instance.securityTraceLevel() > 0)
            {
                StringBuffer s = new StringBuffer();
                s.append("enabling SSL ciphersuites for server socket " + toString() + ":");
                for(int i = 0; i < cipherSuites.length; ++i)
                {
                    s.append("\n  " + cipherSuites[i]);
                }
                _logger.trace(_instance.securityTraceCategory(), s.toString());
            }

            String[] protocols = _instance.protocols();
            if(protocols != null)
            {
                try
                {
                    _fd.setEnabledProtocols(protocols);
                }
                catch(IllegalArgumentException ex)
                {
                    Ice.SecurityException e = new Ice.SecurityException();
                    e.reason = "IceSSL: invalid protocol";
                    e.initCause(ex);
                    throw e;
                }
            }
        }
        catch(java.io.IOException ex)
        {
            try
            {
                if(_fd != null)
                {
                    _fd.close();
                }
            }
            catch(java.io.IOException e)
            {
            }
            _fd = null;
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
    }

    protected void
    finalize()
        throws Throwable
    {
        assert(_fd == null);

        super.finalize();
    }

    private static class HandshakeThread extends Thread
    {
        HandshakeThread(javax.net.ssl.SSLSocket fd)
        {
            _fd = fd;
            _ok = false;
        }

        public void
        run()
        {
            try
            {
                _fd.getSession();
                synchronized(this)
                {
                    _ok = true;
                    notifyAll();
                }

            }
            catch(RuntimeException ex)
            {
                synchronized(this)
                {
                    _ex = ex;
                    notifyAll();
                }
            }
        }

        boolean
        waitForHandshake(int timeout)
        {
            boolean result = false;

            synchronized(this)
            {
                while(!_ok && _ex == null)
                {
                    try
                    {
                        wait(timeout);
                        break;
                    }
                    catch(InterruptedException ex)
                    {
                        continue;
                    }
                }

                if(_ex != null)
                {
                    throw _ex;
                }

                result = _ok;
            }

            return result;
        }

        private javax.net.ssl.SSLSocket _fd;
        private boolean _ok;
        private RuntimeException _ex;
    }

    private Instance _instance;
    private String _adapterName;
    private Ice.Logger _logger;
    private javax.net.ssl.SSLServerSocket _fd;
    private int _backlog;
    private java.net.InetSocketAddress _addr;
    private java.net.InetSocketAddress _connectToSelfAddr;
}
