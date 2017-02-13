// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

import java.nio.*;
import javax.net.ssl.*;
import javax.net.ssl.SSLEngineResult.*;

final class TransceiverI implements IceInternal.Transceiver, IceInternal.WSTransceiverDelegate
{
    @Override
    public java.nio.channels.SelectableChannel fd()
    {
        return _stream.fd();
    }

    @Override
    public int initialize(IceInternal.Buffer readBuffer, IceInternal.Buffer writeBuffer, Ice.Holder<Boolean> moreData)
    {
        int status = _stream.connect(readBuffer, writeBuffer);
        if(status != IceInternal.SocketOperation.None)
        {
            return status;
        }

        status = handshakeNonBlocking();
        if(status != IceInternal.SocketOperation.None)
        {
            return status;
        }

        //
        // Additional verification.
        //
        _instance.verifyPeer((NativeConnectionInfo)getInfo(), _stream.fd(), _host);

        if(_instance.securityTraceLevel() >= 1)
        {
            _instance.traceConnection(_stream.fd(), _engine, _incoming);
        }
        return IceInternal.SocketOperation.None;
    }

    @Override
    public int closing(boolean initiator, Ice.LocalException ex)
    {
        // If we are initiating the connection closure, wait for the peer
        // to close the TCP/IP connection. Otherwise, close immediately.
        return initiator ? IceInternal.SocketOperation.Read : IceInternal.SocketOperation.None;
    }

    @Override
    public void close()
    {
        if(_stream.isConnected())
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
                        // close_notify message might therefore not be received by the peer. This is
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
                //_instance.logger().error("IceSSL: error during close\n" + ex.getMessage());
            }
        }

        _stream.close();
    }

    @Override
    public IceInternal.EndpointI bind()
    {
        assert(false);
        return null;
    }

    @Override
    public int write(IceInternal.Buffer buf)
    {
        if(!_stream.isConnected())
        {
            return _stream.write(buf);
        }

        int status = writeNonBlocking(buf.b);
        assert(status == IceInternal.SocketOperation.None || status == IceInternal.SocketOperation.Write);
        return status;
    }

    @Override
    public int read(IceInternal.Buffer buf, Ice.Holder<Boolean> moreData)
    {
        moreData.value = false;

        if(!_stream.isConnected())
        {
            return _stream.read(buf);
        }

        //
        // Try to satisfy the request from data we've already decrypted.
        //
        fill(buf.b);

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

                Status status = result.getStatus();
                assert status != Status.BUFFER_OVERFLOW;

                if(status == Status.CLOSED)
                {
                    throw new Ice.ConnectionLostException();
                }
                // Android API 21 SSLEngine doesn't report underflow, so look at the absence of
                // network data and application data to signal a network read.
                else if(status == Status.BUFFER_UNDERFLOW || (_appInput.position() == 0 && _netInput.position() == 0))
                {
                    if(_stream.read(_netInput) == 0)
                    {
                        return IceInternal.SocketOperation.Read;
                    }
                    continue;
                }

                fill(buf.b);
            }

            // If there is no more application data, do one further unwrap to ensure
            // that the SSLEngine has no buffered data (Android R21 and greater only).
            if(_appInput.position() == 0)
            {
                _netInput.flip();
                _engine.unwrap(_netInput, _appInput);
                _netInput.compact();

                // Don't check the status here since we may have already filled
                // the buffer with a complete request which must be processed.
            }
        }
        catch(SSLException ex)
        {
            throw new Ice.SecurityException("IceSSL: error during read", ex);
        }


        //
        // Return a boolean to indicate whether more data is available.
        //
        moreData.value = _netInput.position() > 0 || _appInput.position() > 0;

        return IceInternal.SocketOperation.None;
    }

    @Override
    public String protocol()
    {
        return _instance.protocol();
    }

    @Override
    public String toString()
    {
        return _stream.toString();
    }

    @Override
    public String toDetailedString()
    {
        return toString();
    }

    @Override
    public Ice.ConnectionInfo getInfo()
    {
        NativeConnectionInfo info = new NativeConnectionInfo();
        info.nativeCerts = fillConnectionInfo(info);
        return info;
    }

    @Override
    public Ice.ConnectionInfo getWSInfo(java.util.Map<String, String> headers)
    {
        WSSNativeConnectionInfo info = new WSSNativeConnectionInfo();
        info.nativeCerts = fillConnectionInfo(info);
        info.headers = headers; // Provided header is a copy so no need to clone here.
        return info;
    }

    @Override
    public void setBufferSize(int rcvSize, int sndSize)
    {
        _stream.setBufferSize(rcvSize, sndSize);
    }

    @Override
    public void checkSendSize(IceInternal.Buffer buf)
    {
    }

    TransceiverI(Instance instance, javax.net.ssl.SSLEngine engine, IceInternal.StreamSocket stream, String hostOrAdapterName,
                 boolean incoming)
    {
        _instance = instance;
        _engine = engine;
        _appInput = ByteBuffer.allocateDirect(engine.getSession().getApplicationBufferSize() * 2);
        _netInput = ByteBuffer.allocateDirect(engine.getSession().getPacketBufferSize() * 2);
        _netOutput = ByteBuffer.allocateDirect(engine.getSession().getPacketBufferSize() * 2);
        _stream = stream;
        _incoming = incoming;
        if(_incoming)
        {
            _adapterName = hostOrAdapterName;
        }
        else
        {
            _host = hostOrAdapterName;
        }
    }

    private java.security.cert.Certificate[] fillConnectionInfo(ConnectionInfo info)
    {
        //
        // This can only be called on an open transceiver.
        //
        java.security.cert.Certificate[] nativeCerts = null;
        if(_stream.fd() != null)
        {
            java.net.Socket socket = _stream.fd().socket();
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

            if(!socket.isClosed())
            {
                info.rcvSize = IceInternal.Network.getRecvBufferSize(_stream.fd());
                info.sndSize = IceInternal.Network.getSendBufferSize(_stream.fd());
            }

            SSLSession session = _engine.getSession();
            info.cipher = session.getCipherSuite();
            try
            {
                java.security.cert.Certificate[] pcerts = session.getPeerCertificates();
                java.security.cert.Certificate[] vcerts = _instance.engine().getVerifiedCertificateChain(pcerts);
                info.verified = vcerts != null;
                nativeCerts = vcerts != null ? vcerts : pcerts;
                java.util.ArrayList<String> certs = new java.util.ArrayList<String>();
                for(java.security.cert.Certificate c : nativeCerts)
                {
                    StringBuilder s = new StringBuilder("-----BEGIN CERTIFICATE-----\n");
                    s.append(IceUtilInternal.Base64.encode(c.getEncoded()));
                    s.append("\n-----END CERTIFICATE-----");
                    certs.add(s.toString());
                }
                info.certs = certs.toArray(new String[certs.size()]);
            }
            catch(javax.net.ssl.SSLPeerUnverifiedException ex)
            {
                // No peer certificates.
            }
            catch(java.security.cert.CertificateEncodingException ex)
            {
            }
        }
        info.adapterName = _adapterName;
        info.incoming = _incoming;
        return nativeCerts;
    }

    private int handshakeNonBlocking()
    {
        try
        {
            HandshakeStatus status = _engine.getHandshakeStatus();
            while(!_engine.isOutboundDone() && !_engine.isInboundDone())
            {
                SSLEngineResult result = null;
                switch(status)
                {
                case FINISHED:
                case NOT_HANDSHAKING:
                {
                    return IceInternal.SocketOperation.None;
                }
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
                    if(_netInput.position() == 0 && _stream.read(_netInput) == 0)
                    {
                        return IceInternal.SocketOperation.Read;
                    }

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
                    {
                        assert(false);
                        break;
                    }
                    case BUFFER_UNDERFLOW:
                    {
                        assert(status == javax.net.ssl.SSLEngineResult.HandshakeStatus.NEED_UNWRAP);
                        if(_stream.read(_netInput) == 0)
                        {
                            return IceInternal.SocketOperation.Read;
                        }
                        break;
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
                    break;
                }
                case NEED_WRAP:
                {
                    //
                    // The engine needs to send a message.
                    //
                    result = _engine.wrap(_emptyBuffer, _netOutput);
                    if(result.bytesProduced() > 0 && !flushNonBlocking())
                    {
                        return IceInternal.SocketOperation.Write;
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

    private int writeNonBlocking(ByteBuffer buf)
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
                if(buf.hasRemaining())
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
                }

                //
                // Write the encrypted data to the socket. We continue writing until we've written
                // all of _netOutput, or until flushNonBlocking indicates that it cannot write
                // (i.e., by returning SocketOperation.Write).
                //
                if(_netOutput.position() > 0 && !flushNonBlocking())
                {
                    return IceInternal.SocketOperation.Write;
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

    private boolean flushNonBlocking()
    {
        _netOutput.flip();

        try
        {
            _stream.write(_netOutput);
        }
        catch(Ice.SocketException ex)
        {
            throw new Ice.ConnectionLostException(ex);
        }

        if(_netOutput.hasRemaining())
        {
            _netOutput.compact();
            return false;
        }
        else
        {
            _netOutput.clear();
            return true;
        }
    }

    private void fill(ByteBuffer buf)
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
                _appInput.get(arr, buf.arrayOffset() + buf.position(), bytesAvailable);
                buf.position(buf.position() + bytesAvailable);
            }
            else if(_appInput.hasArray())
            {
                //
                // Copy directly from the source buffer's backing array.
                //
                byte[] arr = _appInput.array();
                buf.put(arr, _appInput.arrayOffset() + _appInput.position(), bytesAvailable);
                _appInput.position(_appInput.position() + bytesAvailable);
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

    private Instance _instance;
    private IceInternal.StreamSocket _stream;
    private javax.net.ssl.SSLEngine _engine;
    private String _host = "";
    private String _adapterName = "";
    private boolean _incoming;

    private ByteBuffer _appInput; // Holds clear-text data to be read by the application.
    private ByteBuffer _netInput; // Holds encrypted data read from the socket.
    private ByteBuffer _netOutput; // Holds encrypted data to be written to the socket.
    private static ByteBuffer _emptyBuffer = ByteBuffer.allocate(0); // Used during handshaking.
}
