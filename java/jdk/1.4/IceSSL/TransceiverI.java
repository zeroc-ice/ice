// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

final class TransceiverI implements IceInternal.Transceiver
{
    public java.nio.channels.SelectableChannel
    fd()
    {
        return null;
    }

    public void
    close()
    {
        if(_instance.networkTraceLevel() >= 1)
        {
            String s = "closing ssl connection\n" + toString();
            _logger.trace(_instance.networkTraceCategory(), s);
        }

        synchronized(this)
        {
            assert(_fd != null);
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
        /*
         * shutdownOutput is not supported by an SSL socket.
         *
        if(_instance.networkTraceLevel() >= 2)
        {
            String s = "shutting down ssl connection for writing\n" + toString();
            _logger.trace(_instance.networkTraceCategory(), s);
        }

        assert(_fd != null);
        try
        {
            _fd.shutdownOutput(); // Shutdown socket for writing
        }
        catch(UnsupportedOperationException ex)
        {
            // Ignore - shutdownOutput not supported.
        }
        catch(java.io.IOException ex)
        {
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
        */
    }

    public void
    shutdownReadWrite()
    {
        if(_instance.networkTraceLevel() >= 2)
        {
            String s = "shutting down ssl connection for reading and writing\n" + toString();
            _logger.trace(_instance.networkTraceCategory(), s);
        }

        assert(_fd != null);

        _shutdown = true;

        /*
         * shutdownInput is not supported by an SSL socket.
         *
        try
        {
            _fd.shutdownInput(); // Shutdown socket for reading
            //_fd.shutdownOutput(); // Shutdown socket for writing
        }
        catch(UnsupportedOperationException ex)
        {
            // Ignore - shutdownInput not supported.
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
        */
    }

    public void
    write(IceInternal.BasicStream stream, int timeout)
        throws IceInternal.LocalExceptionWrapper
    {
        java.nio.ByteBuffer buf = stream.prepareWrite();

        byte[] data = null;
        int off = 0;
        try
        {
            data = buf.array();
            off = buf.arrayOffset();
        }
        catch(UnsupportedOperationException ex)
        {
            assert(false);
        }

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

        while(buf.hasRemaining() && !_shutdown)
        {
            int pos = buf.position();
            try
            {
                assert(_fd != null);
                int rem = buf.remaining();
                _out.write(data, off + pos, rem);
                buf.position(pos + rem);

                if(_instance.networkTraceLevel() >= 3)
                {
                    String s = "sent " + rem + " of " + buf.limit() + " bytes via ssl\n" + toString();
                    _logger.trace(_instance.networkTraceCategory(), s);
                }

                if(_stats != null)
                {
                    _stats.bytesSent(type(), rem);
                }

                break;
            }
            catch(java.io.InterruptedIOException ex)
            {
                buf.position(pos + ex.bytesTransferred);
            }
            catch(java.io.IOException ex)
            {
                if(IceInternal.Network.connectionLost(ex))
                {
                    //
                    // Java's SSL implementation might have successfully sent the
                    // packet but then detected loss of connection and raised an
                    // exception. As a result, we cannot be sure that it is safe
                    // to retry in this situation, so we raise LocalExceptionWrapper.
                    //
                    Ice.ConnectionLostException se = new Ice.ConnectionLostException();
                    se.initCause(ex);
                    throw new IceInternal.LocalExceptionWrapper(se, false);
                }
                
                Ice.SocketException se = new Ice.SocketException();
                se.initCause(ex);
                throw se;
            }
        }

        if(_shutdown && buf.hasRemaining())
        {
            throw new Ice.ConnectionLostException();
        }
    }

    public boolean
    read(IceInternal.BasicStream stream, int timeout)
    {
        java.nio.ByteBuffer buf = stream.prepareRead();

        int remaining = 0;
        if(_instance.networkTraceLevel() >= 3)
        {
            remaining = buf.remaining();
        }

        byte[] data = null;
        int off = 0;
        try
        {
            data = buf.array();
            off = buf.arrayOffset();
        }
        catch(UnsupportedOperationException ex)
        {
            assert(false);
        }

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
                assert(_fd != null);
                int ret = _in.read(data, off + pos, buf.remaining());
                
                if(ret == -1)
                {
                    throw new Ice.ConnectionLostException();
                }

                if(ret > 0)
                {
                    if(_instance.networkTraceLevel() >= 3)
                    {
                        String s = "received " + ret + " of " + remaining + " bytes via ssl\n" + toString();
                        _logger.trace(_instance.networkTraceCategory(), s);
                    }

                    if(_stats != null)
                    {
                        _stats.bytesReceived(type(), ret);
                    }

                    buf.position(pos + ret);
                }
            }
            catch(java.net.SocketTimeoutException ex)
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
            catch(java.io.InterruptedIOException ex)
            {
                buf.position(pos + ex.bytesTransferred);
            }
            catch(java.io.IOException ex)
            {
                if(IceInternal.Network.connectionLost(ex))
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

        return false;
    }

    public String
    type()
    {
        return "ssl";
    }

    public String
    toString()
    {
        return _desc;
    }

    public void
    checkSendSize(IceInternal.BasicStream stream, int messageSizeMax)
    {
        if(stream.size() > messageSizeMax)
        {
            throw new Ice.MemoryLimitException();
        }
    }

    ConnectionInfo
    getConnectionInfo()
    {
        //
        // This can only be called on an open transceiver.
        //
        assert(_fd != null);
        return _info;
    }

    //
    // Only for use by ConnectorI, AcceptorI
    //
    TransceiverI(Instance instance, javax.net.ssl.SSLSocket fd, ConnectionInfo info)
    {
        _instance = instance;
        _fd = fd;
        _info = info;
        _logger = instance.communicator().getLogger();
        try
        {
            _stats = instance.communicator().getStats();
        }
        catch(Ice.CommunicatorDestroyedException ex)
        {
            // Ignore.
        }
        _desc = IceInternal.Network.fdToString(_fd);
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
        assert(_fd == null);

        super.finalize();
    }

    private Instance _instance;
    private javax.net.ssl.SSLSocket _fd;
    private ConnectionInfo _info;
    private Ice.Logger _logger;
    private Ice.Stats _stats;
    private String _desc;
    private java.io.InputStream _in;
    private java.io.OutputStream _out;
    private volatile boolean _shutdown;
}
