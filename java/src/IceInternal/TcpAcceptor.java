// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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

        assert(_fd != null);
        Network.closeSocketNoThrow(_fd);
        _fd = null;
    }

    public void
    listen()
    {
        // Nothing to do.

        if(_traceLevels.network >= 1)
        {
            StringBuffer s = new StringBuffer("listening for tcp connections at ");
            s.append(toString());

            java.util.List<String> interfaces = 
                Network.getHostsForEndpointExpand(_addr.getAddress().getHostAddress(), _instance.protocolSupport(),
                                                  true);
            if(!interfaces.isEmpty())
            {
                s.append("\nlocal interfaces: ");
                s.append(IceUtilInternal.StringUtil.joinString(interfaces, ", "));
            }
            _logger.trace(_traceLevels.networkCat, s.toString());
        }
    }

    public Transceiver
    accept()
    {
        java.nio.channels.SocketChannel fd = Network.doAccept(_fd);
        Network.setBlock(fd, false);
        Network.setTcpBufSize(fd, _instance.initializationData().properties, _logger);

        if(_traceLevels.network >= 1)
        {
            String s = "accepted tcp connection\n" + Network.fdToString(fd);
            _logger.trace(_traceLevels.networkCat, s);
        }

        return new TcpTransceiver(_instance, fd);
    }

    public String
    toString()
    {
        return Network.addrToString(_addr);
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
        _backlog = instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.TCP.Backlog", 511);

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
            _addr = Network.getAddressForServer(host, port, _instance.protocolSupport(), _instance.preferIPv6());
            if(_traceLevels.network >= 2)
            {
                String s = "attempting to bind to tcp socket " + toString();
                _logger.trace(_traceLevels.networkCat, s);
            }
            _addr = Network.doBind(_fd, _addr, _backlog);
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
    private TraceLevels _traceLevels;
    private Ice.Logger _logger;
    private java.nio.channels.ServerSocketChannel _fd;
    private int _backlog;
    private java.net.InetSocketAddress _addr;
}
