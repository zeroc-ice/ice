// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final public class Transceiver
{
    public void
    close()
    {
        if(_traceLevels.network >= 1)
        {
            String s = "closing tcp connection\n" + toString();
            _logger.trace(_traceLevels.networkCat, s);
        }

	synchronized(this)
	{
	    if(IceUtil.Debug.ASSERT)
	    {
		IceUtil.Debug.Assert(_fd != null);
	    }
	    try
	    {
		_fd.close();
	    }
	    catch(java.io.IOException ex)
	    {
		Ice.SocketException se = new Ice.SocketException();
		se.initCause(ex);
		throw se;
	    }
	    finally
	    {
	        _fd = null;
	    }
	}
    }

    public void
    shutdownWrite()
    {
	//
	// Not implemented.
	//
    }

    public void
    shutdownReadWrite()
    {
	if(_traceLevels.network >= 2)
	{
	    String s = "shutting down tcp connection for reading and writing\n" + toString();
	    _logger.trace(_traceLevels.networkCat, s);
	}

	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_fd != null);
	}

	_shutdown = true;
    }

    public void
    write(BasicStream stream, int timeout)
    {
        ByteBuffer buf = stream.prepareWrite();

	byte[] data = buf.array();

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
	}
	catch(java.net.SocketException ex)
	{
	    Ice.SocketException se = new Ice.SocketException();
	    se.initCause(ex);
	    throw se;
	}

        int remaining = buf.remaining();
        int packetSize = remaining;
        if(_maxPacketSize > 0 && packetSize > _maxPacketSize)
        {
            packetSize = _maxPacketSize;
        }

        int pos = buf.position();
	while(buf.hasRemaining() && !_shutdown)
	{
	    try
	    {
		if(IceUtil.Debug.ASSERT)
		{
		    IceUtil.Debug.Assert(_fd != null);
		}

		_out.write(data, pos, packetSize);
                pos += packetSize;

		if(_traceLevels.network >= 3)
		{
		    String s = "sent " + packetSize + " of " + buf.limit() + " bytes via tcp\n" + toString();
		    _logger.trace(_traceLevels.networkCat, s);
		}
	    }
	    catch(java.io.InterruptedIOException ex)
	    {
                pos += ex.bytesTransferred;
	    }
	    catch(java.io.IOException ex)
	    {
		Ice.SocketException se = new Ice.SocketException();
		se.initCause(ex);
		throw se;
	    }

            buf.position(pos);
            if(buf.remaining() < packetSize)
            {
                packetSize = buf.remaining();
            }
        }

	if(_shutdown && buf.hasRemaining())
	{
	    throw new Ice.ConnectionLostException();
	}
    }

    public void
    read(BasicStream stream, int timeout)
    {
	ByteBuffer buf = stream.prepareRead();

	int remaining = 0;
	if(_traceLevels.network >= 3)
	{
	    remaining = buf.remaining();
	}

	byte[] data = buf.array();

	int interval = 500;
	if(timeout >= 0 && timeout < interval)
	{
	    interval = timeout;
	}

	while(buf.hasRemaining() && !_shutdown)
	{
	    int pos = buf.position();
	    try
	    {
		_fd.setSoTimeout(interval);
		if(IceUtil.Debug.ASSERT)
		{
		    IceUtil.Debug.Assert(_fd != null);
		}
		int ret = _in.read(data, pos, buf.remaining());
		
		if(ret == -1)
		{
		    throw new Ice.ConnectionLostException();
		}

		if(ret > 0)
		{
		    if(_traceLevels.network >= 3)
		    {
			String s = "received " + ret + " of " + remaining + " bytes via tcp\n" + toString();
			_logger.trace(_traceLevels.networkCat, s);
		    }

		    buf.position(pos + ret);
		}
	    }
	    catch(java.io.InterruptedIOException ex)
	    {
		if(ex.bytesTransferred > 0)
		{
		    buf.position(pos + ex.bytesTransferred);
		}
		if(timeout >= 0)
		{
		    if(interval >= timeout)
		    {
			throw new Ice.TimeoutException();
		    }
		    timeout -= interval;
		}
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

	if(_shutdown)
	{
	    throw new Ice.ConnectionLostException();
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
    // Only for use by Connector, Acceptor
    //
    Transceiver(Instance instance, java.net.Socket fd)
    {
        _fd = fd;
        _traceLevels = instance.traceLevels();
        _logger = instance.initializationData().logger;
        _desc = Network.fdToString(_fd);
        _maxPacketSize = 0;
        if(System.getProperty("os.name").startsWith("Windows"))
        {
            //
            // On Windows, limiting the buffer size is important to prevent
            // poor throughput performances when transfering large amount of
            // data. See Microsoft KB article KB823764.
            //
            try
            {
                _maxPacketSize = _fd.getSendBufferSize() / 2;
                if(_maxPacketSize < 512)
                {
                    _maxPacketSize = 0;
                }
            }
            catch(java.net.SocketException ex)
            {
                Ice.SocketException se = new Ice.SocketException();
                se.initCause(ex);
                throw se;
            }
        }
        
	try
	{
	    _in = _fd.getInputStream();
	    _out = _fd.getOutputStream();
	}
	catch(java.io.IOException ex)
	{
	    try
	    {
		_fd.close();
	    }
	    catch(java.io.IOException e)
	    {
	    }
	    _fd = null;
	    Ice.SocketException se = new Ice.SocketException();
	    se.initCause(ex);
	    throw se;
	}
	_shutdown = false;
    }

    protected synchronized void
    finalize()
        throws Throwable
    {
        IceUtil.Debug.FinalizerAssert(_fd == null);
    }

    private java.net.Socket _fd;
    private TraceLevels _traceLevels;
    private Ice.Logger _logger;
    private String _desc;
    private java.io.InputStream _in;
    private java.io.OutputStream _out;
    private volatile boolean _shutdown;
    private int _maxPacketSize;
}
