// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
        }
        catch(java.io.IOException ex)
        {
        }
        _fd = null;
    }

    public void
    shutdown()
    {
        if(_traceLevels.network >= 2)
        {
            String s = "shutting down tcp connection\n" + toString();
            _logger.trace(_traceLevels.networkCat, s);
        }

        assert(_fd != null);
        java.net.Socket socket = _fd.socket();
        try
        {
            socket.shutdownOutput(); // Shutdown socket for writing
        }
        catch(java.io.IOException ex)
        {
        }
    }

    public void
    write(BasicStream stream, int timeout)
    {
	// TODO: Timeouts are ignored!!

        java.nio.ByteBuffer buf = stream.prepareWrite();

	java.nio.channels.Selector selector = null; // TODO: Very inefficient!!

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
			}

			continue;
		    }
		    
		    if(_traceLevels.network >= 3)
		    {
			String s = "sent " + ret + " of " + buf.limit() + " bytes via tcp\n" + toString();
			_logger.trace(_traceLevels.networkCat, s);
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

	java.nio.channels.Selector selector = null; // TODO: Very inefficient!!

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
			}

			continue;
		    }
		    
		    if(ret > 0 && _traceLevels.network >= 3)
		    {
			String s = "received " + ret + " of " + remaining + " bytes via tcp\n" + toString();
			_logger.trace(_traceLevels.networkCat, s);
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
		}
	    }
	}
    }

    public String
    toString()
    {
        return Network.fdToString(_fd);
    }

    //
    // Only for use by TcpConnector, TcpAcceptor
    //
    TcpTransceiver(Instance instance, java.nio.channels.SocketChannel fd)
    {
        _fd = fd;
        _traceLevels = instance.traceLevels();
        _logger = instance.logger();
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
}
