// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
        if (_traceLevels.network >= 1)
        {
            String s = "stopping to accept tcp connections at " + toString();
            _logger.trace(_traceLevels.networkCat, s);
        }

        java.nio.channels.ServerSocketChannel fd = _fd;
        if (fd != null)
        {
            _fd = null;
            try
            {
                fd.close();
            }
            catch (java.io.IOException ex)
            {
                // Ignore
            }
        }
    }

    public void
    shutdown()
    {
        if (_traceLevels.network >= 2)
        {
            String s = "shutting down accepting tcp connections at " +
                toString();
            _logger.trace(_traceLevels.networkCat, s);
        }

        //
        // Java doesn't support shutting down a server socket
        //
        java.nio.channels.ServerSocketChannel fd = _fd;
        _fd = null;
        try
        {
            fd.close();
        }
        catch (java.io.IOException ex)
        {
            // Ignore
        }
    }

    public void
    listen()
    {
        // Nothing to do

        if (_traceLevels.network >= 1)
        {
            String s = "accepting tcp connections at " + toString();
            _logger.trace(_traceLevels.networkCat, s);
        }
    }

    public Transceiver
    accept(int timeout)
    {
        java.nio.channels.SocketChannel fd = Network.doAccept(_fd, timeout);

        if (_traceLevels.network >= 1)
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

    final boolean
    equivalent(String host, int port)
    {
        java.net.InetSocketAddress addr = Network.getAddress(host, port);
        java.net.InetSocketAddress localAddr =
            Network.getLocalAddress(_addr.getPort());
        return addr.equals(localAddr);
    }

    int
    effectivePort()
    {
        return _addr.getPort();
    }

    TcpAcceptor(Instance instance, int port)
    {
        _instance = instance;
        _traceLevels = instance.traceLevels();
        _logger = instance.logger();
        _backlog = 0;

        if (_backlog <= 0)
        {
            _backlog = 5;
        }

        try
        {
            java.net.InetSocketAddress addr =
                new java.net.InetSocketAddress(port);

            _fd = Network.createTcpServerSocket();
            _addr = Network.doBind(_fd, addr);
        }
        catch (RuntimeException ex)
        {
            _fd = null;
            throw ex;
        }
    }

    protected void
    finalize()
        throws Throwable
    {
        assert(_fd == null);

        super.finalize();
    }

    private Instance _instance;
    private TraceLevels _traceLevels;
    private Ice.Logger _logger;
    private java.nio.channels.ServerSocketChannel _fd;
    private int _backlog;
    private java.net.InetSocketAddress _addr;
}
