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
		IceUtil.Debug.Assert(_connection != null);
	    }
	    try
	    {
		_connection.close();
	    }
	    catch(java.io.IOException ex)
	    {
		Ice.SocketException se = new Ice.SocketException();
		se.initCause(ex);
		throw se;
	    }
	    finally
	    {
	        _connection = null;
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
	    IceUtil.Debug.Assert(_connection != null);
	}

	_shutdown = true;
    }

    public void
    write(BasicStream stream, int timeout)
    {
        ByteBuffer buf = stream.prepareWrite();

	byte[] data = buf.array();

	while(buf.hasRemaining())
	{
	    int pos = buf.position();
	    try
	    {
		if(IceUtil.Debug.ASSERT)
		{
		    IceUtil.Debug.Assert(_connection != null);
		}
		int rem = buf.remaining();
		_out.write(data, pos, rem);
		_out.flush();
		buf.position(pos + rem);

		if(_traceLevels.network >= 3)
		{
		    String s = "sent " + rem + " of " + buf.limit() + " bytes via tcp\n" + toString();
		    _logger.trace(_traceLevels.networkCat, s);
		}
		break;
	    }
	    catch(java.io.InterruptedIOException ex)
	    {
		buf.position(pos + ex.bytesTransferred);
	    }
	    catch(java.io.IOException ex)
	    {
		Ice.SocketException se = new Ice.SocketException();
		se.initCause(ex);
		throw se;
	    }
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
		// _fd.setSoTimeout(interval); XXX
		if(IceUtil.Debug.ASSERT)
		{
		    IceUtil.Debug.Assert(_connection != null);
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
    // Only for use by TcpConnector, TcpAcceptor
    //
    TcpTransceiver(Instance instance, javax.microedition.io.Connection connection)
    {
        _connection = (javax.microedition.io.SocketConnection)connection;
        _traceLevels = instance.traceLevels();
        _logger = instance.logger();
        _desc = Network.toString(_connection);
	try
	{
	    _in = _connection.openInputStream();
	    _out = _connection.openOutputStream();
	}
	catch(java.io.IOException ex)
	{
	    try
	    {
		_connection.close();
	    }
	    catch(java.io.IOException e)
	    {
	    }
	    _connection = null;
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
        IceUtil.Debug.FinalizerAssert(_connection == null);
    }

    private javax.microedition.io.SocketConnection _connection;
    private TraceLevels _traceLevels;
    private Ice.Logger _logger;
    private String _desc;
    private java.io.InputStream _in;
    private java.io.OutputStream _out;
    private volatile boolean _shutdown;
}
