// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final class TcpTransceiver implements Transceiver
{
    public java.nio.channels.SelectableChannel
    fd()
    {
        assert(_fd != null);
        return _fd;
    }

    public void
    close()
    {
        if(_traceLevels.network >= 1)
        {
            String s = "closing tcp connection\n" + toString();
            _logger.trace(_traceLevels.networkCat, s);
        }

        assert(_fd != null);
        try
        {
            _fd.close();
	    _fd = null;
        }
        catch(java.io.IOException ex)
        {
	    _fd = null;
	    Ice.SocketException se = new Ice.SocketException();
	    se.initCause(ex);
	    throw se;
        }
    }

    public void
    shutdownWrite()
    {
        if(_traceLevels.network >= 2)
        {
            String s = "shutting down tcp connection for writing\n" + toString();
            _logger.trace(_traceLevels.networkCat, s);
        }

        assert(_fd != null);
        java.net.Socket socket = _fd.socket();
        try
        {
            socket.shutdownOutput(); // Shutdown socket for writing
        }
        catch(java.net.SocketException ex)
        {
	    //
	    // Ignore errors indicating that we are shutdown already.
	    //
	    if(Network.notConnected(ex))
	    {
		return;
	    }

	    Ice.SocketException se = new Ice.SocketException();
	    se.initCause(ex);
	    throw se;
        }
        catch(java.io.IOException ex)
        {
	    Ice.SocketException se = new Ice.SocketException();
	    se.initCause(ex);
	    throw se;
        }
    }

    public void
    shutdownReadWrite()
    {
        if(_traceLevels.network >= 2)
        {
            String s = "shutting down tcp connection for reading and writing\n" + toString();
            _logger.trace(_traceLevels.networkCat, s);
        }

        assert(_fd != null);
        java.net.Socket socket = _fd.socket();
        try
        {
	    //
	    // TODO: Java does not support SHUT_RDWR. Calling both
	    // shutdownInput and shutdownOutput results in an exception.
	    //
	    socket.shutdownInput(); // Shutdown socket for reading
	    //socket.shutdownOutput(); // Shutdown socket for writing
        }
        catch(java.net.SocketException ex)
        {
	    // Ignore.
        }
        catch(java.io.IOException ex)
        {
	    Ice.SocketException se = new Ice.SocketException();
	    se.initCause(ex);
	    throw se;
        }
    }

    public void
    write(BasicStream stream, int timeout)
    {
        java.nio.ByteBuffer buf = stream.prepareWrite();

	java.nio.channels.Selector selector = null;

	try
	{
	    while(buf.hasRemaining())
	    {
		try
		{
		    assert(_fd != null);
		    int ret = _fd.write(buf);
		    
		    if(ret == -1)
		    {
			throw new Ice.ConnectionLostException();
		    }
		    
		    if(ret == 0)
		    {
			if(timeout == 0)
			{
			    throw new Ice.TimeoutException();
			}
			
			if(selector == null)
			{
			    selector = java.nio.channels.Selector.open();
			    _fd.register(selector, java.nio.channels.SelectionKey.OP_WRITE, null);
			}
			
			try
			{
			    if(timeout > 0)
			    {
				long start = System.currentTimeMillis();
				int n = selector.select(timeout);
				if(n == 0 && System.currentTimeMillis() >= start + timeout)
				{
				    throw new Ice.TimeoutException();
				}
			    }
			    else
			    {
				selector.select();
			    }
			}
			catch(java.io.InterruptedIOException ex)
			{
			    // Ignore.
			}

			continue;
		    }
		    
		    if(_traceLevels.network >= 3)
		    {
			String s = "sent " + ret + " of " + buf.limit() + " bytes via tcp\n" + toString();
			_logger.trace(_traceLevels.networkCat, s);
		    }

                    if(_stats != null)
                    {
                        _stats.bytesSent(type(), ret);
                    }
		}
		catch(java.io.InterruptedIOException ex)
		{
		    continue;
		}
		catch(java.io.IOException ex)
		{
		    Ice.SocketException se = new Ice.SocketException();
		    se.initCause(ex);
		    throw se;
		}
	    }
	}
	finally
	{
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
    }

    public void
    read(BasicStream stream, int timeout)
    {
        java.nio.ByteBuffer buf = stream.prepareRead();

        int remaining = 0;
        if(_traceLevels.network >= 3)
        {
            remaining = buf.remaining();
        }

	java.nio.channels.Selector selector = null;

	try
	{
	    while(buf.hasRemaining())
	    {
		try
		{
		    assert(_fd != null);
		    int ret = _fd.read(buf);
		    
		    if(ret == -1)
		    {
			throw new Ice.ConnectionLostException();
		    }
		    
		    if(ret == 0)
		    {
			if(timeout == 0)
			{
			    throw new Ice.TimeoutException();
			}

			if(selector == null)
			{
			    selector = java.nio.channels.Selector.open();
			    _fd.register(selector, java.nio.channels.SelectionKey.OP_READ, null);
			}
			
			try
			{
			    if(timeout > 0)
			    {
				long start = System.currentTimeMillis();
				int n = selector.select(timeout);
				if(n == 0 && System.currentTimeMillis() >= start + timeout)
				{
				    throw new Ice.TimeoutException();
				}
			    }
			    else
			    {
				selector.select();
			    }
			}
			catch(java.io.InterruptedIOException ex)
			{
			    // Ignore.
			}

			continue;
		    }
		    
		    if(ret > 0)
		    {
                        if(_traceLevels.network >= 3)
                        {
                            String s = "received " + ret + " of " + remaining + " bytes via tcp\n" + toString();
                            _logger.trace(_traceLevels.networkCat, s);
                        }

                        if(_stats != null)
                        {
                            _stats.bytesReceived(type(), ret);
                        }
                    }
		}
		catch(java.io.InterruptedIOException ex)
		{
		    continue;
		}
		catch(java.io.IOException ex)
		{
		    if(Network.connectionLost(ex))
		    {
			Ice.ConnectionLostException se = new Ice.ConnectionLostException();
			se.initCause(ex);
			throw se;
		    }
		    
		    Ice.SocketException se = new Ice.SocketException();
		    se.initCause(ex);
		    throw se;
		}
	    }
        }
	finally
	{
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
    }

    public String
    type()
    {
        return "tcp";
    }

    public String
    toString()
    {
        return _desc;
    }

    //
    // Only for use by TcpConnector, TcpAcceptor
    //
    TcpTransceiver(Instance instance, java.nio.channels.SocketChannel fd)
    {
        _fd = fd;
        _traceLevels = instance.traceLevels();
        _logger = instance.logger();
        _stats = instance.stats();
        _desc = Network.fdToString(_fd);
    }

    protected void
    finalize()
        throws Throwable
    {
        assert(_fd == null);

        super.finalize();
    }

    private java.nio.channels.SocketChannel _fd;
    private TraceLevels _traceLevels;
    private Ice.Logger _logger;
    private Ice.Stats _stats;
    private String _desc;
}
