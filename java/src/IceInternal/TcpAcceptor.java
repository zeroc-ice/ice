// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
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

        java.nio.channels.ServerSocketChannel fd = _fd;
        if(fd != null)
        {
            _fd = null;
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

        if(_traceLevels.network >= 1)
        {
            String s = "accepting tcp connections at " + toString();
            _logger.trace(_traceLevels.networkCat, s);
        }
    }

    public Transceiver
    accept(int timeout)
    {
        java.nio.channels.SocketChannel fd = Network.doAccept(_fd, timeout);
        Network.setBlock(fd, false);

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
        _logger = instance.logger();
        _backlog = 0;

        if(_backlog <= 0)
        {
            _backlog = 5;
        }

        try
        {
            _fd = Network.createTcpServerSocket();
            Network.setBlock(_fd, false);
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
