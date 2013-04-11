// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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

    public int
    initialize(IceInternal.Buffer readBuffer, IceInternal.Buffer writeBuffer)
    {
        try
        {
            if(_state == StateNeedConnect)
            {
                _state = StateConnectPending;
                return IceInternal.SocketOperation.Connect;
            }
            else if(_state == StateConnectPending)
            {
                IceInternal.Network.doFinishConnect(_fd);
                _desc = IceInternal.Network.fdToString(_fd, _proxy, _addr);

                if(_proxy != null)
                {
                    //
                    // Prepare the read & write buffers in advance.
                    //
                    _proxy.beginWriteConnectRequest(_addr, writeBuffer);
                    _proxy.beginReadConnectRequestResponse(readBuffer);

                    //
                    // Write the proxy connection message.
                    //
                    if(writeRaw(writeBuffer))
                    {
                        //
                        // Write completed without blocking.
                        //
                        _proxy.endWriteConnectRequest(writeBuffer);

                        //
                        // Try to read the response.
                        //
                        if(readRaw(readBuffer))
                        {
                            //
                            // Read completed without blocking - fall through.
                            //
                            _proxy.endReadConnectRequestResponse(readBuffer);
                        }
                        else
                        {
                            //
                            // Return SocketOperationRead to indicate we need to complete the read.
                            //
                            _state = StateProxyConnectRequestPending; // Wait for proxy response
                            return IceInternal.SocketOperation.Read;
                        }
                    }
                    else
                    {
                        //
                        // Return SocketOperationWrite to indicate we need to complete the write.
                        //
                        _state = StateProxyConnectRequest; // Send proxy connect request
                        return IceInternal.SocketOperation.Write;
                    }
                }

                _state = StateConnected;
            }
            else if(_state == StateProxyConnectRequest)
            {
                //
                // Write completed.
                //
                _proxy.endWriteConnectRequest(writeBuffer);
                _state = StateProxyConnectRequestPending; // Wait for proxy response
                return IceInternal.SocketOperation.Read;
            }
            else if(_state == StateProxyConnectRequestPending)
            {
                //
                // Read completed.
                //
                _proxy.endReadConnectRequestResponse(readBuffer);
                _state = StateConnected;
            }

            if(_state == StateConnected)
            {
                return handshakeNonBlocking();
            }
        }
        catch(Ice.LocalException ex)
        {
            if(_instance.networkTraceLevel() >= 2)
            {
                StringBuilder s = new StringBuilder(128);
                s.append("failed to establish ssl connection\n");
                s.append(IceInternal.Network.fdToString(_fd, _proxy, _addr));
                _logger.trace(_instance.networkTraceCategory(), s.toString());
            }
            throw ex;
        }

        return IceInternal.SocketOperation.None;
    }

    public void
    close()
    {
        if(_state == StateHandshakeComplete && _instance.networkTraceLevel() >= 1)
        {
            String s = "closing ssl connection\n" + toString();
            _logger.trace(_instance.networkTraceCategory(), s);
        }

        assert(_fd != null);

        if(_state >= StateConnected)
        {
            try
            {
                //
                // Send the close_notify message.
                //
                _engine.closeOutbound();
                _netOutput.clear();
                while(!_engine.isOutboundDone())
                {
                    _engine.wrap(_emptyBuffer, _netOutput);
                    try
                    {
                        //
                        // Note: we can't block to send the close_notify message. In some cases, the
                        // close_notify message might therefore not be receieved by the peer. This is
                        // not a big issue since the Ice protocol isn't subject to truncation attacks.
                        //
                        flushNonBlocking();
                    }
                    catch(Ice.LocalException ex)
                    {
                        // Ignore.
                    }
                }
            }
            catch(SSLException ex)
            {
                //
                // We can't throw in close.
                //
                // Ice.SecurityException se = new Ice.SecurityException(
                //     "IceSSL: SSL failure while shutting down socket", ex);
                //
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
        }

        try
        {
            IceInternal.Network.closeSocket(_fd);
        }
        finally
        {
            _fd = null;
        }
    }

    public boolean
    write(IceInternal.Buffer buf)
    {
        if(_state == StateProxyConnectRequest)
        {
            //
            // We need to write the proxy message, but we have to use TCP and not SSL.
            //
            return writeRaw(buf);
        }

        //
        // If the handshake isn't completed yet, we shouldn't be writing.
        //
        if(_state < StateHandshakeComplete)
        {
            throw new Ice.ConnectionLostException();
        }

        //
        // We don't want write to be called on android main thread as this will cause
        // NetworkOnMainThreadException to be thrown. If that is the android main thread
        // we return false and this method will be later called from the thread pool.
        //
        if(IceInternal.Util.isAndroidMainThread(Thread.currentThread()))
        {
            return false;
        }

        int status = writeNonBlocking(buf.b);
        if(status != IceInternal.SocketOperation.None)
        {
            assert(status == IceInternal.SocketOperation.Write);
            return false;
        }
        return true;
    }

    @SuppressWarnings("deprecation")
    public boolean
    read(IceInternal.Buffer buf, Ice.BooleanHolder moreData)
    {
        moreData.value = false;

        if(_state == StateProxyConnectRequestPending)
        {
            //
            // We need to read the proxy reply, but we have to use TCP and not SSL.
            //
            return readRaw(buf);
        }

        //
        // If the handshake isn't completed yet, we shouldn't be reading (read can be
        // called by the thread pool when the connection is registered/unregistered
        // with the pool to be closed).
        //
        if(_state < StateHandshakeComplete)
        {
            throw new Ice.ConnectionLostException();
        }

        int rem = 0;
        if(_instance.networkTraceLevel() >= 3)
        {
            rem = buf.b.remaining();
        }

        //
        // Try to satisfy the request from data we've already decrypted.
        //
        int pos = buf.b.position();
        fill(buf.b);

        if(_instance.networkTraceLevel() >= 3 && buf.b.position() > pos)
        {
            String s = "received " + (buf.b.position() - pos) + " of " + rem + " bytes via ssl\n" + toString();
            _logger.trace(_instance.networkTraceCategory(), s);
        }

        if(_stats != null && buf.b.position() > pos)
        {
            _stats.bytesReceived(type(), buf.b.position() - pos);
        }

        //
        // Read and decrypt more data if necessary. Note that we might read
        // more data from the socket than is actually necessary to fill the
        // caller's stream.
        //
        try
        {
            while(buf.b.hasRemaining())
            {
                _netInput.flip();
                SSLEngineResult result = _engine.unwrap(_netInput, _appInput);
                _netInput.compact();
                switch(result.getStatus())
                {
                case BUFFER_OVERFLOW:
                {
                    assert(false);
                    break;
                }
                case BUFFER_UNDERFLOW:
                {
                    int status = readNonBlocking();
                    if(status != IceInternal.SocketOperation.None)
                    {
                        assert(status == IceInternal.SocketOperation.Read);
                        return false;
                    }
                    continue;
                }
                case CLOSED:
                {
                    throw new Ice.ConnectionLostException();
                }
                case OK:
                {
                    break;
                }
                }

                pos = buf.b.position();
                fill(buf.b);

                if(_instance.networkTraceLevel() >= 3 && buf.b.position() > pos)
                {
                    String s = "received " + (buf.b.position() - pos) + " of " + rem + " bytes via ssl\n" + toString();
                    _logger.trace(_instance.networkTraceCategory(), s);
                }

                if(_stats != null && buf.b.position() > pos)
                {
                    _stats.bytesReceived(type(), buf.b.position() - pos);
                }
            }
        }
        catch(SSLException ex)
        {
            throw new Ice.SecurityException("IceSSL: error during read", ex);
        }

        //
        // Return a boolean to indicate whether more data is available.
        //
        moreData.value = _netInput.position() > 0;
        return true;
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

    public Ice.ConnectionInfo
    getInfo()
    {
        return getNativeConnectionInfo();
    }

    public void
    checkSendSize(IceInternal.Buffer buf, int messageSizeMax)
    {
        if(buf.size() > messageSizeMax)
        {
            IceInternal.Ex.throwMemoryLimitException(buf.size(), messageSizeMax);
        }
    }

    TransceiverI(Instance instance, javax.net.ssl.SSLEngine engine, java.nio.channels.SocketChannel fd,
                 IceInternal.NetworkProxy proxy, String host, java.net.InetSocketAddress addr)
    {
        init(instance, engine, fd);
        _proxy = proxy;
        _host = host;
        _incoming = false;
        _addr = addr;
        _state = StateNeedConnect;
        _desc = IceInternal.Network.fdToString(_fd, _proxy, _addr);
    }

    TransceiverI(Instance instance, javax.net.ssl.SSLEngine engine, java.nio.channels.SocketChannel fd,
                 String adapterName)
    {
        init(instance, engine, fd);
        _host = "";
        _adapterName = adapterName;
        _incoming = true;
        _state = StateConnected;
        _desc = IceInternal.Network.fdToString(_fd);
    }

    @SuppressWarnings("deprecation")
    private void init(Instance instance, javax.net.ssl.SSLEngine engine, java.nio.channels.SocketChannel fd)
    {
        _instance = instance;
        _engine = engine;
        _fd = fd;
        _logger = instance.communicator().getLogger();
        try
        {
            _stats = instance.communicator().getStats();
        }
        catch(Ice.CommunicatorDestroyedException ex)
        {
            // Ignore.
        }
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

        _appInput = ByteBuffer.allocateDirect(engine.getSession().getApplicationBufferSize() * 2);
        _netInput = ByteBuffer.allocateDirect(engine.getSession().getPacketBufferSize() * 2);
        _netOutput = ByteBuffer.allocateDirect(engine.getSession().getPacketBufferSize() * 2);
    }

    protected void
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

    private NativeConnectionInfo
    getNativeConnectionInfo()
    {
        //
        // This can only be called on an open transceiver.
        //
        NativeConnectionInfo info = new NativeConnectionInfo();
        if(_fd != null)
        {
            java.net.Socket socket = _fd.socket();
            //
            // On some platforms (e.g., early Android releases), sockets don't
            // correctly return address information.
            //
            if(socket.getLocalAddress() != null)
            {
                info.localAddress = socket.getLocalAddress().getHostAddress();
                info.localPort = socket.getLocalPort();
            }
            
            if(socket.getInetAddress() != null)
            {
                info.remoteAddress = socket.getInetAddress().getHostAddress();
                info.remotePort = socket.getPort();
            }

            SSLSession session = _engine.getSession();
            info.cipher = session.getCipherSuite();
            try
            {
                java.util.ArrayList<String> certs = new java.util.ArrayList<String>();
                info.nativeCerts = session.getPeerCertificates();
                for(java.security.cert.Certificate c : info.nativeCerts)
                {
                    StringBuffer s = new StringBuffer("-----BEGIN CERTIFICATE-----\n");
                    s.append(IceUtilInternal.Base64.encode(c.getEncoded()));
                    s.append("\n-----END CERTIFICATE-----");
                    certs.add(s.toString());
                }
                info.certs = certs.toArray(new String[0]);
            }
            catch(java.security.cert.CertificateEncodingException ex)
            {
            }
            catch(javax.net.ssl.SSLPeerUnverifiedException ex)
            {
                // No peer certificates.
            }
        }
        info.adapterName = _adapterName;
        info.incoming = _incoming;
        return info;
    }

    private int
    handshakeNonBlocking()
    {
        try
        {
            HandshakeStatus status = _engine.getHandshakeStatus();
            while(_state != StateHandshakeComplete)
            {
                SSLEngineResult result = null;
                switch(status)
                {
                case FINISHED:
                case NOT_HANDSHAKING:
                    handshakeCompleted();
                    break;
                case NEED_TASK:
                {
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
                    {
                        assert(status == javax.net.ssl.SSLEngineResult.HandshakeStatus.NEED_UNWRAP);
                        int ss = readNonBlocking();
                        if(ss != IceInternal.SocketOperation.None)
                        {
                            return ss;
                        }
                        break;
                    }
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
                        int ss = flushNonBlocking();
                        if(ss != IceInternal.SocketOperation.None)
                        {
                            return ss;
                        }
                    }
                    //
                    // FINISHED is only returned from wrap or unwrap, not from engine.getHandshakeResult().
                    //
                    status = result.getHandshakeStatus();
                    break;
                }
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
            throw new Ice.SecurityException("IceSSL: handshake error", ex);
        }

        return IceInternal.SocketOperation.None;
    }

    private void
    handshakeCompleted()
    {
        _state = StateHandshakeComplete;

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
                    throw new Ice.SecurityException("IceSSL: server did not supply a certificate", ex);
                }
            }
        }

        //
        // Additional verification.
        //
        _instance.verifyPeer(getNativeConnectionInfo(), _fd, _host);

        if(_instance.networkTraceLevel() >= 1)
        {
            String s;
            if(_incoming)
            {
                s = "accepted ssl connection\n" + _desc;
            }
            else
            {
                s = "ssl connection established\n" + _desc;
            }
            _logger.trace(_instance.networkTraceCategory(), s);
        }

        if(_instance.securityTraceLevel() >= 1)
        {
            _instance.traceConnection(_fd, _engine, _incoming);
        }
    }

    @SuppressWarnings("deprecation")
    private int
    writeNonBlocking(ByteBuffer buf)
    {
        //
        // This method has two purposes: encrypt the application's message buffer into our
        // _netOutput buffer, and write the contents of _netOutput to the socket without
        // blocking.
        //
        try
        {
            while(buf.hasRemaining() || _netOutput.position() > 0)
            {
                final int rem = buf.remaining();

                if(rem > 0)
                {
                    //
                    // Encrypt the buffer.
                    //
                    SSLEngineResult result = _engine.wrap(buf, _netOutput);
                    switch(result.getStatus())
                    {
                    case BUFFER_OVERFLOW:
                        //
                        // Need to make room in _netOutput.
                        //
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
                    // If the SSL engine consumed any of the application's message buffer,
                    // then log it.
                    //
                    if(result.bytesConsumed() > 0)
                    {
                        if(_instance.networkTraceLevel() >= 3)
                        {
                            String s = "sent " + result.bytesConsumed() + " of " + rem + " bytes via ssl\n" +
                                toString();
                            _logger.trace(_instance.networkTraceCategory(), s);
                        }

                        if(_stats != null)
                        {
                            _stats.bytesSent(type(), result.bytesConsumed());
                        }
                    }
                }

                //
                // Write the encrypted data to the socket. We continue writing until we've written
                // all of _netOutput, or until flushNonBlocking indicates that it cannot write
                // (i.e., by returning NeedWrite).
                //
                if(_netOutput.position() > 0)
                {
                    int ss = flushNonBlocking();
                    if(ss != IceInternal.SocketOperation.None)
                    {
                        assert(ss == IceInternal.SocketOperation.Write);
                        return ss;
                    }
                }
            }
        }
        catch(SSLException ex)
        {
            throw new Ice.SecurityException("IceSSL: error while encoding message", ex);
        }

        assert(_netOutput.position() == 0);
        return IceInternal.SocketOperation.None;
    }

    private int
    flushNonBlocking()
    {
        _netOutput.flip();

        final int size = _netOutput.limit();
        int packetSize = size - _netOutput.position();
        if(_maxPacketSize > 0 && packetSize > _maxPacketSize)
        {
            packetSize = _maxPacketSize;
            _netOutput.limit(_netOutput.position() + packetSize);
        }

        int status = IceInternal.SocketOperation.None;
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
                else if(ret == 0)
                {
                    status = IceInternal.SocketOperation.Write;
                    break;
                }

                if(packetSize == _maxPacketSize)
                {
                    assert(_netOutput.position() == _netOutput.limit());
                    packetSize = size - _netOutput.position();
                    if(packetSize > _maxPacketSize)
                    {
                        packetSize = _maxPacketSize;
                    }
                    _netOutput.limit(_netOutput.position() + packetSize);
                }
            }
            catch(java.io.InterruptedIOException ex)
            {
                continue;
            }
            catch(java.io.IOException ex)
            {
                throw new Ice.ConnectionLostException(ex);
            }
        }

        if(status == IceInternal.SocketOperation.None)
        {
            _netOutput.clear();
        }
        else
        {
            _netOutput.limit(size);
            _netOutput.compact();
        }

        return status;
    }

    private int
    readNonBlocking()
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
                else if(ret == 0)
                {
                    return IceInternal.SocketOperation.Read;
                }

                break;
            }
            catch(java.io.InterruptedIOException ex)
            {
                continue;
            }
            catch(java.io.IOException ex)
            {
                throw new Ice.ConnectionLostException(ex);
            }
        }

        return IceInternal.SocketOperation.None;
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

    @SuppressWarnings("deprecation")
    private boolean
    writeRaw(IceInternal.Buffer buf)
    {
        //
        // We don't want write to be called on android main thread as this will cause
        // NetworkOnMainThreadException to be thrown. If that is the android main thread
        // we return false and this method will be later called from the thread pool.
        //
        if(IceInternal.Util.isAndroidMainThread(Thread.currentThread()))
        {
            return false;
        }

        final int size = buf.b.limit();
        int packetSize = size - buf.b.position();

        while(buf.b.hasRemaining())
        {
            try
            {
                assert(_fd != null);
                int ret = _fd.write(buf.b);

                if(ret == -1)
                {
                    throw new Ice.ConnectionLostException();
                }
                else if(ret == 0)
                {
                    return false;
                }

                if(_instance.networkTraceLevel() >= 3)
                {
                    String s = "sent " + ret + " of " + packetSize + " bytes via tcp\n" + toString();
                    _logger.trace(_instance.networkTraceCategory(), s);
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
                throw new Ice.SocketException(ex);
            }
        }
        return true;
    }

    @SuppressWarnings("deprecation")
    private boolean
    readRaw(IceInternal.Buffer buf)
    {
        int packetSize = buf.b.remaining();

        while(buf.b.hasRemaining())
        {
            try
            {
                assert(_fd != null);
                int ret = _fd.read(buf.b);

                if(ret == -1)
                {
                    throw new Ice.ConnectionLostException();
                }

                if(ret == 0)
                {
                    return false;
                }

                if(ret > 0)
                {
                    if(_instance.networkTraceLevel() >= 3)
                    {
                        String s = "received " + ret + " of " + packetSize + " bytes via tcp\n" + toString();
                        _logger.trace(_instance.networkTraceCategory(), s);
                    }

                    if(_stats != null)
                    {
                        _stats.bytesReceived(type(), ret);
                    }
                }

                packetSize = buf.b.remaining();
            }
            catch(java.io.InterruptedIOException ex)
            {
                continue;
            }
            catch(java.io.IOException ex)
            {
                throw new Ice.ConnectionLostException(ex);
            }
        }

        return true;
    }

    private Instance _instance;
    private java.nio.channels.SocketChannel _fd;
    private javax.net.ssl.SSLEngine _engine;
    private IceInternal.NetworkProxy _proxy;
    private String _host;
    private boolean _incoming;
    private String _adapterName;
    private java.net.InetSocketAddress _addr;
    private int _state;
    private Ice.Logger _logger;

    @SuppressWarnings("deprecation")
    private Ice.Stats _stats;
    private String _desc;
    private int _maxPacketSize;
    private ByteBuffer _appInput; // Holds clear-text data to be read by the application.
    private ByteBuffer _netInput; // Holds encrypted data read from the socket.
    private ByteBuffer _netOutput; // Holds encrypted data to be written to the socket.
    private static ByteBuffer _emptyBuffer = ByteBuffer.allocate(0); // Used during handshaking.

    private static final int StateNeedConnect = 0;
    private static final int StateConnectPending = 1;
    private static final int StateProxyConnectRequest = 2;
    private static final int StateProxyConnectRequestPending = 3;
    private static final int StateConnected = 4;
    private static final int StateHandshakeComplete = 5;
}
