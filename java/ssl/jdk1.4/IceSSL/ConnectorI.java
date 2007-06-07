// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

final class ConnectorI implements IceInternal.Connector, java.lang.Comparable
{
    final static short TYPE = 2;

    public IceInternal.Transceiver
    connect(int timeout)
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

        if(_instance.networkTraceLevel() >= 2)
        {
            String s = "trying to establish ssl connection to " + toString();
            _logger.trace(_instance.networkTraceCategory(), s);
        }

        javax.net.ssl.SSLSocket fd = null;
        ConnectionInfo connInfo = null;
        try
        {
            //
            // If a connect timeout is specified, do the connect in a separate thread.
            //
            if(timeout >= 0)
            {
                ConnectThread ct = new ConnectThread(_instance.context(), _addr);
                ct.start();
                fd = ct.getFd(timeout == 0 ? 1 : timeout);
                if(fd == null)
                {
                    throw new Ice.ConnectTimeoutException();
                }
            }
            else
            {
                javax.net.SocketFactory factory = _instance.context().getSocketFactory();
                fd = (javax.net.ssl.SSLSocket)factory.createSocket(_addr.getAddress(), _addr.getPort());
            }

            fd.setUseClientMode(true);

            String[] cipherSuites = _instance.filterCiphers(fd.getSupportedCipherSuites(), fd.getEnabledCipherSuites());
            try
            {
                fd.setEnabledCipherSuites(cipherSuites);
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
                s.append("enabling SSL ciphersuites for socket\n" + IceInternal.Network.fdToString(fd) + ":");
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
                    fd.setEnabledProtocols(protocols);
                }
                catch(IllegalArgumentException ex)
                {
                    Ice.SecurityException e = new Ice.SecurityException();
                    e.reason = "IceSSL: invalid protocol";
                    e.initCause(ex);
                    throw e;
                }
            }

            //
            // If a connect timeout is specified, do the SSL handshake in a separate thread.
            //
            if(timeout >= 0)
            {
                HandshakeThread ht = new HandshakeThread(fd);
                ht.start();
                if(!ht.waitForHandshake(timeout == 0 ? 1 : timeout))
                {
                    throw new Ice.ConnectTimeoutException();
                }
            }
            else
            {
                fd.startHandshake();
            }

            //
            // Check IceSSL.VerifyPeer.
            //
            int verifyPeer =
                _instance.communicator().getProperties().getPropertyAsIntWithDefault("IceSSL.VerifyPeer", 2);
            if(verifyPeer > 0)
            {
                try
                {
                    fd.getSession().getPeerCertificates();
                }
                catch(javax.net.ssl.SSLPeerUnverifiedException ex)
                {
                    Ice.SecurityException e = new Ice.SecurityException();
                    e.reason = "IceSSL: server did not supply a certificate";
                    e.initCause(ex);
                    throw e;
                }
            }

            connInfo = Util.populateConnectionInfo(fd, "", false);
            _instance.verifyPeer(connInfo, fd, _host, false);
        }
        catch(java.net.ConnectException ex)
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
            Ice.ConnectFailedException se;
            if(IceInternal.Network.connectionRefused(ex))
            {
                se = new Ice.ConnectionRefusedException();
            }
            else
            {
                se = new Ice.ConnectFailedException();
            }
            se.initCause(ex);
            throw se;
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
            String s = "ssl connection established\n" + IceInternal.Network.fdToString(fd);
            _logger.trace(_instance.networkTraceCategory(), s);
        }

        if(_instance.securityTraceLevel() > 0)
        {
            _instance.traceConnection(fd, false);
        }

        return new TransceiverI(_instance, fd, connInfo);
    }

    public short
    type()
    {
        return TYPE;
    }

    public String
    toString()
    {
        return IceInternal.Network.addrToString(_addr);
    }

    public int
    hashCode()
    {
        return _hashCode;
    }

    //
    // Only for use by EndpointI
    //
    ConnectorI(Instance instance, java.net.InetSocketAddress addr, int timeout, String connectionId)
    {
        _instance = instance;
        _logger = instance.communicator().getLogger();
        _host = addr.getHostName();
        _addr = addr;
        _timeout = timeout;
        _connectionId = connectionId;

        _hashCode = _addr.getAddress().getHostAddress().hashCode();
        _hashCode = 5 * _hashCode + _addr.getPort();
        _hashCode = 5 * _hashCode + _timeout;
        _hashCode = 5 * _hashCode + _connectionId.hashCode();
    }

    //
    // Compare connectors for sorting purposes
    //
    public boolean
    equals(java.lang.Object obj)
    {
        return compareTo(obj) == 0;
    }

    public int
    compareTo(java.lang.Object obj) // From java.lang.Comparable
    {
        ConnectorI p = null;

        try
        {
            p = (ConnectorI)obj;
        }
        catch(ClassCastException ex)
        {
            try
            {
                IceInternal.Connector c = (IceInternal.Connector)obj;
                return type() < c.type() ? -1 : 1;
            }
            catch(ClassCastException ee)
            {
                assert(false);
            }
        }

        if(this == p)
        {
            return 0;
        }

        if(_timeout < p._timeout)
        {
            return -1;
        }
        else if(p._timeout < _timeout)
        {
            return 1;
        }

        if(!_connectionId.equals(p._connectionId))
        {
            return _connectionId.compareTo(p._connectionId);
        }

        if(_timeout < p._timeout)
        {
            return -1;
        }
        else if(p._timeout < _timeout)
        {
            return 1;
        }

        return IceInternal.Network.compareAddress(_addr, p._addr);
    }


    private static class ConnectThread extends Thread
    {
        ConnectThread(javax.net.ssl.SSLContext ctx, java.net.InetSocketAddress addr)
        {
            _ctx = ctx;
            _addr = addr;
        }

        public void
        run()
        {
            try
            {
                javax.net.SocketFactory factory = _ctx.getSocketFactory();
                javax.net.ssl.SSLSocket fd =
                    (javax.net.ssl.SSLSocket)factory.createSocket(_addr.getAddress(), _addr.getPort());
                synchronized(this)
                {
                    _fd = fd;
                    notifyAll();
                }
            }
            catch(java.io.IOException ex)
            {
                synchronized(this)
                {
                    _ex = ex;
                    notifyAll();
                }
            }
        }

        javax.net.ssl.SSLSocket
        getFd(int timeout)
            throws java.io.IOException
        {
            javax.net.ssl.SSLSocket fd = null;

            synchronized(this)
            {
                while(_fd == null && _ex == null)
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

                fd = _fd;
                _fd = null;
            }

            return fd;
        }

        private javax.net.ssl.SSLContext _ctx;
        private java.net.InetSocketAddress _addr;
        private javax.net.ssl.SSLSocket _fd;
        private java.io.IOException _ex;
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
                _fd.startHandshake();
                synchronized(this)
                {
                    _ok = true;
                    notifyAll();
                }

            }
            catch(java.io.IOException ex)
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
            throws java.io.IOException
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
        private java.io.IOException _ex;
    }

    private Instance _instance;
    private Ice.Logger _logger;
    private String _host;
    private java.net.InetSocketAddress _addr;
    private int _timeout;
    private String _connectionId;
    private int _hashCode;
}
