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
    //
    // TODO- This needs to be refactored. The two worker thread classes are essentially the same.
    //
    private class ReadThread extends Thread
    {
	ReadThread(BasicStream stream, int timeout)
	{
	    _stream = stream;
	}

	public void
	run()
	{
	    try
	    {
		readImpl(_stream);
	    }
	    catch(RuntimeException ex)
	    {
		_ex = ex;
	    }
	    
	    synchronized(this)
	    {
		_done = true;
		notifyAll();
	    }
	}

	public void 
	read()
	{
	    long absoluteTimeout = System.currentTimeMillis() + _timeout;
	    long interval = _timeout;
	    
	    synchronized(this)
	    {
		//
		// The _done flag protects against the situation where the read thread has completed before we get
		// this far in this call.
		//
		while(_ex == null && !_done)
		{
		    try
		    {
			wait(interval);
			break;
		    }
		    catch(InterruptedException ex)
		    {
			//
			// Reduce the wait interval by the amount of time already waited.
			//
			interval = absoluteTimeout - System.currentTimeMillis();
			if(interval <= 0)
			{
			    throw new Ice.TimeoutException();
			}
			continue;
		    }
		}

		if(_ex != null)
		{
		    throw _ex;
		}
	    }
	}

	int _timeout;
	BasicStream _stream;
	java.lang.RuntimeException _ex = null;
	boolean _done = false;
    }

    private class WriteThread extends Thread
    {
	WriteThread(BasicStream stream, int timeout)
	{
	    _stream = stream;
	}

	public void
	run()
	{
	    try
	    {
		writeImpl(_stream);
	    }
	    catch(RuntimeException ex)
	    {
		_ex = ex;
	    }
	    
	    synchronized(this)
	    {
		_done = true;
		notifyAll();
	    }
	}

	public void 
	write()
	{
	    long absoluteTimeout = System.currentTimeMillis() + _timeout;
	    long interval = _timeout;
	    
	    synchronized(this)
	    {
		//
		// The _done flag protects against the situation where the write thread has completed before we get
		// this far in this call.
		//
		while(_ex == null && !_done)
		{
		    try
		    {
			wait(interval);
			break;
		    }
		    catch(InterruptedException ex)
		    {
			//
			// Reduce the wait interval by the amount of time already waited.
			//
			interval = absoluteTimeout - System.currentTimeMillis();
			if(interval <= 0)
			{
			    throw new Ice.TimeoutException();
			}
			continue;
		    }
		}

		if(_ex != null)
		{
		    throw _ex;
		}
	    }
	}

	int _timeout;
	BasicStream _stream;
	java.lang.RuntimeException _ex = null;
	boolean _done = false;
    }
    
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
	try
	{
	    _out.close();
	}
	catch(java.io.IOException ex)
	{
	    //
	    // TODO: Take appropriate action here.
	    //
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

	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_connection != null);
	}

	try
	{
	    _in.close();
	    _out.close();
	}
	catch(java.io.IOException ex)
	{
	    //
	    // TODO: Take appropriate action here.
	    //
	}
	
	_shutdown = true;
    }

    public void
    write(BasicStream stream, int timeout)
    {
	//
	// TODO: TcpConnector translates a 0 timeout to 1. Do we want to do the same here for consistency or do we
	// assert?
	//
	IceUtil.Debug.Assert(timeout == -1 || timeout > 0);
	if(timeout < 0)
	{
	    writeImpl(stream);
	}
	else
	{
	    WriteThread t = new WriteThread(stream, timeout);
	    t.start();

	    //
	    // This blocks until either an exception is thrown by writeImpl() or the timeout expires.
	    //
	    t.write();
	}
    }

    protected void
    writeImpl(BasicStream stream)
    {
        ByteBuffer buf = stream.prepareWrite();

	byte[] data = buf.array();
	int chunkSize = WRITE_CHUNK; 

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
		if(chunkSize > 0 && chunkSize < rem)
		{
		    rem = chunkSize;
		}
		_out.write(data, pos, rem);
		_out.flush();
		buf.position(pos + rem);

		if(_traceLevels.network >= 3)
		{
		    String s = "sent " + rem + " of " + buf.limit() + " bytes via tcp\n" + toString();
		    _logger.trace(_traceLevels.networkCat, s);
		}
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
	//
	// TODO: TcpConnector translates a 0 timeout to 1. Do we want to do the same here for consistency or do we
	// assert?
	//
	IceUtil.Debug.Assert(timeout == -1 || timeout > 0);
	if(timeout < 0)
	{
	    readImpl(stream);
	}
	else
	{
	    ReadThread t = new ReadThread(stream, timeout);
	    t.start();

	    //
	    // This blocks until either an exception is thrown by readImpl() or the timeout expires.
	    //
	    t.read();
	}
    }

    protected void
    readImpl(BasicStream stream)
    {
	ByteBuffer buf = stream.prepareRead();

	int remaining = 0;
	if(_traceLevels.network >= 3)
	{
	    remaining = buf.remaining();
	}

	byte[] data = buf.array();

	while(buf.hasRemaining() && !_shutdown)
	{
	    int pos = buf.position();
	    try
	    {
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
    private static final int WRITE_CHUNK = 512;
}
