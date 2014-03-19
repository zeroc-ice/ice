// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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

        assert(_fd != null);
        IceInternal.Network.closeSocketNoThrow(_fd);
        _fd = null;
    }

    public void
    listen()
    {
        // Nothing to do.

        if(_instance.networkTraceLevel() >= 1)
        {
            StringBuffer s = new StringBuffer("listening for ssl connections at ");
            s.append(toString());

            java.util.List<String> interfaces = 
                IceInternal.Network.getHostsForEndpointExpand(_addr.getAddress().getHostAddress(), 
                                                              _instance.protocolSupport(), true);
            if(!interfaces.isEmpty())
            {
                s.append("\nlocal interfaces: ");
                s.append(IceUtilInternal.StringUtil.joinString(interfaces, ", "));
            }
            _logger.trace(_instance.networkTraceCategory(), s.toString());
        }
    }

    public IceInternal.Transceiver
    accept()
    {
        //
        // The plug-in may not be fully initialized.
        //
        if(!_instance.initialized())
        {
            Ice.PluginInitializationException ex = new Ice.PluginInitializationException();
            ex.reason = "IceSSL: plug-in is not initialized";
            throw ex;
        }

        java.nio.channels.SocketChannel fd = IceInternal.Network.doAccept(_fd);

        javax.net.ssl.SSLEngine engine = null;
        try
        {
            IceInternal.Network.setBlock(fd, false);
            IceInternal.Network.setTcpBufSize(fd, _instance.communicator().getProperties(), _logger);

            java.net.InetSocketAddress peerAddr = (java.net.InetSocketAddress)fd.socket().getRemoteSocketAddress();
            engine = _instance.createSSLEngine(true, peerAddr);
        }
        catch(RuntimeException ex)
        {
            IceInternal.Network.closeSocketNoThrow(fd);
            throw ex;
        }

        if(_instance.networkTraceLevel() >= 1)
        {
            _logger.trace(_instance.networkTraceCategory(), "accepting ssl connection\n" +
                          IceInternal.Network.fdToString(fd));
        }

        return new TransceiverI(_instance, engine, fd, _adapterName);
    }

    public String
    toString()
    {
        return IceInternal.Network.addrToString(_addr);
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
        _backlog = instance.communicator().getProperties().getPropertyAsIntWithDefault("Ice.TCP.Backlog", 511);

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
            _addr = IceInternal.Network.getAddressForServer(host, port, _instance.protocolSupport(), 
                                                            _instance.preferIPv6());
            if(_instance.networkTraceLevel() >= 2)
            {
                String s = "attempting to bind to ssl socket " + toString();
                _logger.trace(_instance.networkTraceCategory(), s);
            }
            _addr = IceInternal.Network.doBind(_fd, _addr, _backlog);
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
        try
        {
            IceUtilInternal.Assert.FinalizerAssert(_fd == null);
        }
        catch(java.lang.Exception ex)
        {
        }
        finally
        {
            super.finalize();
        }
    }

    private Instance _instance;
    private String _adapterName;
    private Ice.Logger _logger;
    private java.nio.channels.ServerSocketChannel _fd;
    private int _backlog;
    private java.net.InetSocketAddress _addr;
}
