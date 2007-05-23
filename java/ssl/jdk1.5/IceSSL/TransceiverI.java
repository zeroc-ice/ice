// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

import java.nio.*;
import javax.net.ssl.*;
import javax.net.ssl.SSLEngineResult.*;

final class TransceiverI implements IceInternal.Transceiver
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
        if(_instance.networkTraceLevel() >= 1)
        {
            String s = "closing ssl connection\n" + toString();
            _logger.trace(_instance.networkTraceCategory(), s);
        }

        assert(_fd != null);

        if(_readSelector != null)
        {
            try
            {
                _readSelector.close();
            }
            catch(java.io.IOException ex)
            {
                // Ignore.
            }
            _readSelector = null;
        }

        if(_writeSelector != null)
        {
            try
            {
                _writeSelector.close();
            }
            catch(java.io.IOException ex)
            {
                // Ignore.
            }
            _writeSelector = null;
        }

        try
        {
            _engine.closeInbound();
        }
        catch(SSLException ex)
        {
            //
            // SSLEngine always raises an exception with this message:
            //
            // Inbound closed before receiving peer's close_notify: possible truncation attack?
            //
            // We would probably need to wait for a response in shutdown() to avoid this.
            // For now, we'll ignore this exception.
            //
            //_logger.error("IceSSL: error during close\n" + ex.getMessage());
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

    //
    // All methods that can write to the socket are synchronized.
    //
    public synchronized void
    shutdownWrite()
    {
        if(_instance.networkTraceLevel() >= 2)
        {
            String s = "shutting down ssl connection for writing\n" + toString();
            _logger.trace(_instance.networkTraceCategory(), s);
        }

        shutdown();

        assert(_fd != null);
        java.net.Socket socket = _fd.socket();
        try
        {
            socket.shutdownOutput(); // Shutdown socket for writing.
        }
        catch(java.net.SocketException ex)
        {
            //
            // Ignore. We can't reliably figure out if the socket
            // exception is because the socket is not connected.
            //
            // if(!IceInternal.Network.notConnected(ex))
            // {
            //     Ice.SocketException se = new Ice.SocketException();
            //     se.initCause(ex);
            //     throw se;
            // }
        }
        catch(java.io.IOException ex)
        {
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
    }

    //
    // All methods that can write to the socket are synchronized.
    //
    public synchronized void
    shutdownReadWrite()
    {
        if(_instance.networkTraceLevel() >= 2)
        {
            String s = "shutting down ssl connection for reading and writing\n" + toString();
            _logger.trace(_instance.networkTraceCategory(), s);
        }

        shutdown();

        assert(_fd != null);
        java.net.Socket socket = _fd.socket();
        try
        {
            socket.shutdownInput(); // Shutdown socket for reading
            socket.shutdownOutput(); // Shutdown socket for writing
        }
        catch(java.net.SocketException ex)
        {
            //
            // Ignore. We can't reliably figure out if the socket
            // exception is because the socket is not connected.
            //
            // if(!IceInternal.Network.notConnected(ex))
            // {
            //     Ice.SocketException se = new Ice.SocketException();
            //     se.initCause(ex);
            //     throw se;
            // }
        }
        catch(java.io.IOException ex)
        {
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
    }

    //
    // All methods that can write to the socket are synchronized.
    //
    public synchronized void
    write(IceInternal.BasicStream stream, int timeout)
        throws IceInternal.LocalExceptionWrapper
    {
        //
        // Complete handshaking first if necessary.
        //
        if(!_handshakeComplete)
        {
            handshake(timeout);
        }

        ByteBuffer buf = stream.prepareWrite();

        //
        // Write any pending data to the socket.
        //
        flush(timeout);

        try
        {
            while(buf.hasRemaining())
            {
                final int rem = buf.remaining();

                //
                // Encrypt the buffer.
                //
                SSLEngineResult result = _engine.wrap(buf, _netOutput);
                switch(result.getStatus())
                {
                case BUFFER_OVERFLOW:
                    assert(false);
                    break;
                case BUFFER_UNDERFLOW:
                    assert(false);
                    break;
                case CLOSED:
                    throw new Ice.ConnectionLostException();
                case OK:
                    break;
                }

                //
                // Write the encrypted data to the socket.
                //
                if(result.bytesProduced() > 0)
                {
                    flush(timeout);

                    if(_instance.networkTraceLevel() >= 3)
                    {
                        String s = "sent " + result.bytesConsumed() + " of " + rem + " bytes via ssl\n" + toString();
                        _logger.trace(_instance.networkTraceCategory(), s);
                    }

                    if(_stats != null)
                    {
                        _stats.bytesSent(type(), result.bytesConsumed());
                    }
                }
            }
        }
        catch(SSLException ex)
        {
            Ice.SecurityException e = new Ice.SecurityException();
            e.reason = "IceSSL: error while encoding message";
            e.initCause(ex);
            throw e;
        }
    }

    public boolean
    read(IceInternal.BasicStream stream, int timeout)
    {
        ByteBuffer buf = stream.prepareRead();

        int rem = 0;
        if(_instance.networkTraceLevel() >= 3)
        {
            rem = buf.remaining();
        }

        //
        // Complete handshaking first if necessary.
        //
        if(!_handshakeComplete)
        {
            handshake(timeout);
        }

        //
        // Try to satisfy the request from data we've already decrypted.
        //
        int pos = buf.position();
        fill(buf);

        if(_instance.networkTraceLevel() >= 3 && buf.position() > pos)
        {
            String s = "received " + (buf.position() - pos) + " of " + rem + " bytes via ssl\n" + toString();
            _logger.trace(_instance.networkTraceCategory(), s);
        }

        if(_stats != null && buf.position() > pos)
        {
            _stats.bytesReceived(type(), buf.position() - pos);
        }

        //
        // Read and decrypt more data if necessary. Note that we might read
        // more data from the socket than is actually necessary to fill the
        // caller's stream.
        //
        try
        {
            while(buf.hasRemaining())
            {
                _netInput.flip();
                SSLEngineResult result = _engine.unwrap(_netInput, _appInput);
                _netInput.compact();
                switch(result.getStatus())
                {
                case BUFFER_OVERFLOW:
                    assert(false);
                    break;
                case BUFFER_UNDERFLOW:
                    read(timeout);
                    continue;
                case CLOSED:
                    throw new Ice.ConnectionLostException();
                case OK:
                    break;
                }

                pos = buf.position();
                fill(buf);

                if(_instance.networkTraceLevel() >= 3 && buf.position() > pos)
                {
                    String s = "received " + (buf.position() - pos) + " of " + rem + " bytes via ssl\n" + toString();
                    _logger.trace(_instance.networkTraceCategory(), s);
                }

                if(_stats != null && buf.position() > pos)
                {
                    _stats.bytesReceived(type(), buf.position() - pos);
                }
            }
        }
        catch(SSLException ex)
        {
            Ice.SecurityException e = new Ice.SecurityException();
            e.reason = "IceSSL: error during read";
            e.initCause(ex);
            throw e;
        }

        //
        // Return a boolean to indicate whether more data is available.
        //
        return _netInput.position() > 0;
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
    // Only for use by ConnectorI, AcceptorI.
    //
    TransceiverI(Instance instance, javax.net.ssl.SSLEngine engine, java.nio.channels.SocketChannel fd,
                 String host, boolean incoming, String adapterName)
    {
        _instance = instance;
        _engine = engine;
        _fd = fd;
        _host = host;
        _adapterName = adapterName;
        _incoming = incoming;
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
        _maxPacketSize = 0;
        if(System.getProperty("os.name").startsWith("Windows"))
        {
            //
            // On Windows, limiting the buffer size is important to prevent
            // poor throughput performances when transfering large amount of
            // data. See Microsoft KB article KB823764.
            //
            _maxPacketSize = IceInternal.Network.getSendBufferSize(_fd) / 2;
            if(_maxPacketSize < 512)
            {
                _maxPacketSize = 0;
            }
        }

        // TODO: Buffer cache?
        _appInput = ByteBuffer.allocateDirect(engine.getSession().getApplicationBufferSize() * 2);
        _netInput = ByteBuffer.allocateDirect(engine.getSession().getPacketBufferSize() * 2);
        _netOutput = ByteBuffer.allocateDirect(engine.getSession().getPacketBufferSize() * 2);
        _handshakeComplete = false;
    }

    protected void
    finalize()
        throws Throwable
    {
        IceUtil.Assert.FinalizerAssert(_fd == null);

        super.finalize();
    }

    private void
    flush(int timeout)
    {
        _netOutput.flip();

        int size = _netOutput.limit();
        int packetSize = 0;
        if(_maxPacketSize > 0 && size > _maxPacketSize)
        {
            packetSize = _maxPacketSize;
            _netOutput.limit(_netOutput.position() + packetSize);
        }

        while(_netOutput.hasRemaining())
        {
            try
            {
                assert(_fd != null);
                int ret = _fd.write(_netOutput);

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

                    if(_writeSelector == null)
                    {
                        _writeSelector = java.nio.channels.Selector.open();
                        _fd.register(_writeSelector, java.nio.channels.SelectionKey.OP_WRITE, null);
                    }

                    try
                    {
                        if(timeout > 0)
                        {
                            long start = System.currentTimeMillis();
                            int n = _writeSelector.select(timeout);
                            if(n == 0 && System.currentTimeMillis() >= start + timeout)
                            {
                                throw new Ice.TimeoutException();
                            }
                        }
                        else
                        {
                            _writeSelector.select();
                        }
                    }
                    catch(java.io.InterruptedIOException ex)
                    {
                        // Ignore.
                    }

                    continue;
                }
            }
            catch(java.io.InterruptedIOException ex)
            {
                continue;
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

	    if(packetSize > 0)
	    {
		assert(_netOutput.position() == _netOutput.limit());
		int position = _netOutput.position();
		if(size - position > packetSize)
		{
		    _netOutput.limit(position + packetSize);
		}
		else
		{
		    packetSize = 0;
		    _netOutput.limit(size);
		}
	    }
        }
        _netOutput.clear();
    }

    private void
    read(int timeout)
    {
        while(true)
        {
            try
            {
                assert(_fd != null);
                int ret = _fd.read(_netInput);

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

                    if(_readSelector == null)
                    {
                        _readSelector = java.nio.channels.Selector.open();
                        _fd.register(_readSelector, java.nio.channels.SelectionKey.OP_READ, null);
                    }

                    try
                    {
                        if(timeout > 0)
                        {
                            long start = System.currentTimeMillis();
                            int n = _readSelector.select(timeout);
                            if(n == 0 && System.currentTimeMillis() >= start + timeout)
                            {
                                throw new Ice.TimeoutException();
                            }
                        }
                        else
                        {
                            _readSelector.select();
                        }
                    }
                    catch(java.io.InterruptedIOException ex)
                    {
                        // Ignore.
                    }

                    continue;
                }

                break;
            }
            catch(java.io.InterruptedIOException ex)
            {
                continue;
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
    }

    private void
    handshake(int timeout)
    {
        try
        {
            HandshakeStatus status = _engine.getHandshakeStatus();
            while(!_handshakeComplete)
            {
                SSLEngineResult result = null;
                switch(status)
                {
                case FINISHED:
                    handshakeCompleted();
                    break;
                case NEED_TASK:
                {
                    // TODO: Use separate threads & join with timeout?
                    Runnable task;
                    while((task = _engine.getDelegatedTask()) != null)
                    {
                        task.run();
                    }
                    status = _engine.getHandshakeStatus();
                    break;
                }
                case NEED_UNWRAP:
                {
                    //
                    // The engine needs more data. We might already have enough data in
                    // the _netInput buffer to satisfy the engine. If not, the engine
                    // responds with BUFFER_UNDERFLOW and we'll read from the socket.
                    //
                    _netInput.flip();
                    result = _engine.unwrap(_netInput, _appInput);
                    _netInput.compact();
                    //
                    // FINISHED is only returned from wrap or unwrap, not from engine.getHandshakeResult().
                    //
                    status = result.getHandshakeStatus();
                    switch(result.getStatus())
                    {
                    case BUFFER_OVERFLOW:
                        assert(false);
                        break;
                    case BUFFER_UNDERFLOW:
                        assert(status == javax.net.ssl.SSLEngineResult.HandshakeStatus.NEED_UNWRAP);
                        read(timeout);
                        break;
                    case CLOSED:
                        throw new Ice.ConnectionLostException();
                    case OK:
                        break;
                    }
                    break;
                }
                case NEED_WRAP:
                {
                    //
                    // The engine needs to send a message.
                    //
                    result = _engine.wrap(_emptyBuffer, _netOutput);
                    if(result.bytesProduced() > 0)
                    {
                        flush(timeout);
                    }
                    //
                    // FINISHED is only returned from wrap or unwrap, not from engine.getHandshakeResult().
                    //
                    status = result.getHandshakeStatus();
                    break;
                }
                case NOT_HANDSHAKING:
                    assert(false);
                    break;
                }

                if(result != null)
                {
                    switch(result.getStatus())
                    {
                    case BUFFER_OVERFLOW:
                        assert(false);
                        break;
                    case BUFFER_UNDERFLOW:
                        // Need to read again.
                        assert(status == HandshakeStatus.NEED_UNWRAP);
                        break;
                    case CLOSED:
                        throw new Ice.ConnectionLostException();
                    case OK:
                        break;
                    }
                }
            }
        }
        catch(SSLException ex)
        {
            Ice.SecurityException e = new Ice.SecurityException();
            e.reason = "IceSSL: handshake error";
            e.initCause(ex);
            throw e;
        }
    }

    private void
    fill(ByteBuffer buf)
    {
        _appInput.flip();
        if(_appInput.hasRemaining())
        {
            int bytesAvailable = _appInput.remaining();
            int bytesNeeded = buf.remaining();
            if(bytesAvailable > bytesNeeded)
            {
                bytesAvailable = bytesNeeded;
            }
            if(buf.hasArray())
            {
                //
                // Copy directly into the destination buffer's backing array.
                //
                byte[] arr = buf.array();
                int offset = buf.arrayOffset() + buf.position();
                _appInput.get(arr, offset, bytesAvailable);
                buf.position(offset + bytesAvailable);
            }
            else if(_appInput.hasArray())
            {
                //
                // Copy directly from the source buffer's backing array.
                //
                byte[] arr = _appInput.array();
                int offset = _appInput.arrayOffset() + _appInput.position();
                buf.put(arr, offset, bytesAvailable);
                _appInput.position(offset + bytesAvailable);
            }
            else
            {
                //
                // Copy using a temporary array.
                //
                byte[] arr = new byte[bytesAvailable];
                _appInput.get(arr);
                buf.put(arr);
            }
        }
        _appInput.compact();
    }

    private void
    shutdown()
    {
        //
        // Send the close_notify message.
        //
        _engine.closeOutbound();
        try
        {
            _netOutput.clear();
            while(!_engine.isOutboundDone())
            {
                _engine.wrap(_emptyBuffer, _netOutput);
                try
                {
                    flush(-1);
                }
                catch(Ice.ConnectionLostException ex)
                {
                    // Ignore.
                }
            }
        }
        catch(SSLException ex)
        {
            Ice.SecurityException se = new Ice.SecurityException();
            se.reason = "IceSSL: SSL failure while shutting down socket";
            se.initCause(ex);
            throw se;
        }
    }

    private void
    handshakeCompleted()
    {
        _handshakeComplete = true;

        //
        // IceSSL.VerifyPeer is translated into the proper SSLEngine configuration
        // for a server, but we have to do it ourselves for a client.
        //
        if(!_incoming)
        {
            int verifyPeer =
                _instance.communicator().getProperties().getPropertyAsIntWithDefault("IceSSL.VerifyPeer", 2);
            if(verifyPeer > 0)
            {
                try
                {
                    _engine.getSession().getPeerCertificates();
                }
                catch(javax.net.ssl.SSLPeerUnverifiedException ex)
                {
                    Ice.SecurityException e = new Ice.SecurityException();
                    e.reason = "IceSSL: server did not supply a certificate";
                    e.initCause(ex);
                    throw e;
                }
            }
        }

        //
        // Additional verification.
        //
        _info = Util.populateConnectionInfo(_engine.getSession(), _fd.socket(), _adapterName, _incoming);
        _instance.verifyPeer(_info, _fd, _host, _incoming);

        if(_instance.networkTraceLevel() >= 1)
        {
            String s;
            if(_incoming)
            {
                s = "accepted ssl connection\n" + IceInternal.Network.fdToString(_fd);
            }
            else
            {
                s = "ssl connection established\n" + IceInternal.Network.fdToString(_fd);
            }
            _logger.trace(_instance.networkTraceCategory(), s);
        }

        if(_instance.securityTraceLevel() >= 1)
        {
            _instance.traceConnection(_fd, _engine, _incoming);
        }
    }

    private Instance _instance;
    private java.nio.channels.SocketChannel _fd;
    private javax.net.ssl.SSLEngine _engine;
    private String _host;
    private String _adapterName;
    private boolean _incoming;
    private Ice.Logger _logger;
    private Ice.Stats _stats;
    private String _desc;
    private int _maxPacketSize;
    private ByteBuffer _appInput; // Holds clear-text data to be read by the application.
    private ByteBuffer _netInput; // Holds encrypted data read from the socket.
    private ByteBuffer _netOutput; // Holds encrypted data to be written to the socket.
    private static ByteBuffer _emptyBuffer = ByteBuffer.allocate(0); // Used during handshaking.
    private boolean _handshakeComplete;
    private java.nio.channels.Selector _readSelector;
    private java.nio.channels.Selector _writeSelector;
    private ConnectionInfo _info;
}
