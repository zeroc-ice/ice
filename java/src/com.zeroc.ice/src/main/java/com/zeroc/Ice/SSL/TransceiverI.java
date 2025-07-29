// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice.SSL;

import com.zeroc.Ice.Buffer;
import com.zeroc.Ice.ConnectionLostException;
import com.zeroc.Ice.EndpointI;
import com.zeroc.Ice.IPConnectionInfo;
import com.zeroc.Ice.LocalException;
import com.zeroc.Ice.ReadyCallback;
import com.zeroc.Ice.SecurityException;
import com.zeroc.Ice.SocketException;
import com.zeroc.Ice.SocketOperation;
import com.zeroc.Ice.Transceiver;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.SelectableChannel;
import java.security.cert.Certificate;

import javax.net.ssl.SSLEngine;
import javax.net.ssl.SSLEngineResult;
import javax.net.ssl.SSLEngineResult.HandshakeStatus;
import javax.net.ssl.SSLEngineResult.Status;
import javax.net.ssl.SSLException;
import javax.net.ssl.SSLPeerUnverifiedException;
import javax.net.ssl.SSLSession;

final class TransceiverI implements Transceiver {
    @Override
    public SelectableChannel fd() {
        return _delegate.fd();
    }

    @Override
    public void setReadyCallback(ReadyCallback callback) {
        _readyCallback = callback;
        _delegate.setReadyCallback(callback);
    }

    @Override
    public int initialize(Buffer readBuffer, Buffer writeBuffer) {
        if (!_isConnected) {
            int status = _delegate.initialize(readBuffer, writeBuffer);
            if (status != SocketOperation.None) {
                return status;
            }
            _isConnected = true;

            IPConnectionInfo ipInfo = null;
            for (com.zeroc.Ice.ConnectionInfo p = _delegate.getInfo(_incoming, _adapterName, "");
                    p != null;
                    p = p.underlying) {
                if (p instanceof IPConnectionInfo) {
                    ipInfo = (IPConnectionInfo) p;
                }
            }
            final String host = _incoming ? (ipInfo != null ? ipInfo.remoteAddress : "") : _host;
            final int port = ipInfo != null ? ipInfo.remotePort : -1;
            _engine = _sslEngineFactory.createSSLEngine(host, port);
            _engine.setUseClientMode(!_incoming);

            try {
                _engine.beginHandshake();
            } catch (SSLException ex) {
                _engine = null;
                throw new SecurityException("SSL transport: handshake error", ex);
            }

            _appInput = ByteBuffer.allocate(_engine.getSession().getApplicationBufferSize() * 2);

            // Require BIG_ENDIAN byte buffers. This is needed for Android >= 8.0 which can read
            // the SSL messages directly with these buffers.
            int bufSize = _engine.getSession().getPacketBufferSize() * 2;
            _netInput =
                new Buffer(
                    ByteBuffer.allocateDirect(bufSize * 2), ByteOrder.BIG_ENDIAN);
            _netOutput =
                new Buffer(
                    ByteBuffer.allocateDirect(bufSize * 2), ByteOrder.BIG_ENDIAN);
        }

        int status = handshakeNonBlocking();
        if (status != SocketOperation.None) {
            return status;
        }

        assert (_engine != null);

        SSLSession session = _engine.getSession();
        _cipher = session.getCipherSuite();
        try {
            _certs = session.getPeerCertificates();
            _verified = true;
        } catch (SSLPeerUnverifiedException ex) {
            // No peer certificates.
        }

        //
        // Additional verification.
        //
        _instance.verifyPeer(
            _host,
            (ConnectionInfo) getInfo(_incoming, _adapterName, ""),
            _delegate.toString());

        if (_instance.securityTraceLevel() >= 1) {
            _instance.traceConnection(_delegate.toString(), _engine, _incoming);
        }
        return SocketOperation.None;
    }

    @Override
    public int closing(boolean initiator, LocalException ex) {
        // If we are initiating the connection closure, wait for the peer
        // to close the TCP/IP connection. Otherwise, close immediately.
        return initiator ? SocketOperation.Read : SocketOperation.None;
    }

    @Override
    public void close() {
        if (_engine != null) {
            try {
                //
                // Send the close_notify message.
                //
                _engine.closeOutbound();
                // Cast to java.nio.Buffer to avoid incompatible covariant
                // return type used in Java 9 java.nio.ByteBuffer
                ((java.nio.Buffer) _netOutput.b).clear();
                while (!_engine.isOutboundDone()) {
                    _engine.wrap(_emptyBuffer, _netOutput.b);
                    try {
                        //
                        // Note: we can't block to send the close_notify message. In some cases, the
                        // close_notify message might therefore not be received by the peer. This is
                        // not a big issue since the Ice protocol isn't subject to truncation
                        // attacks.
                        //
                        flushNonBlocking();
                    } catch (LocalException ex) {
                        // Ignore.
                    }
                }
            } catch (SSLException ex) {
                // We can't throw in close.
            }

            try {
                _engine.closeInbound();
            } catch (SSLException ex) {
                //
                // SSLEngine always raises an exception with this message:
                //
                // Inbound closed before receiving peer's close_notify: possible truncation attack?
                //
                // We would probably need to wait for a response in shutdown() to avoid this.
                // For now, we'll ignore this exception.
                //
                // _instance.logger().error("SSL transport: error during close\n" +
                // ex.getMessage());
            }
        }

        _delegate.close();
    }

    @Override
    public EndpointI bind() {
        assert false;
        return null;
    }

    @Override
    public int write(Buffer buf) {
        if (!_isConnected) {
            return _delegate.write(buf);
        }

        int status = writeNonBlocking(buf.b);
        assert (status == SocketOperation.None || status == SocketOperation.Write);
        return status;
    }

    @Override
    public int read(Buffer buf) {
        if (!_isConnected) {
            return _delegate.read(buf);
        }

        _readyCallback.ready(SocketOperation.Read, false);

        //
        // Try to satisfy the request from data we've already decrypted.
        //
        fill(buf.b);

        //
        // Read and decrypt more data if necessary. Note that we might read
        // more data from the socket than is actually necessary to fill the
        // caller's stream.
        //
        try {
            while (buf.b.hasRemaining()) {
                _netInput.flip();
                SSLEngineResult result = _engine.unwrap(_netInput.b, _appInput);
                _netInput.b.compact();

                Status status = result.getStatus();
                assert status != Status.BUFFER_OVERFLOW;

                if (status == Status.CLOSED) {
                    throw new ConnectionLostException();
                } else if (status == Status.BUFFER_UNDERFLOW
                    || (_appInput.position() == 0 && _netInput.b.position() == 0)) {
                    int s = _delegate.read(_netInput);
                    if (s != SocketOperation.None && _netInput.b.position() == 0) {
                        return s;
                    }
                    continue;
                }

                fill(buf.b);
            }

            // If there is no more application data, do one further unwrap to ensure
            // that the SSLEngine has no buffered data (Android R21 and greater only).
            if (_appInput.position() == 0) {
                _netInput.flip();
                _engine.unwrap(_netInput.b, _appInput);
                _netInput.b.compact();

                // Don't check the status here since we may have already filled
                // the buffer with a complete request which must be processed.
            }
        } catch (SSLException ex) {
            throw new SecurityException("SSL transport: error during read", ex);
        }

        //
        // Indicate whether more data is available.
        //
        if (_netInput.b.position() > 0 || _appInput.position() > 0) {
            _readyCallback.ready(SocketOperation.Read, true);
        }

        return SocketOperation.None;
    }

    @Override
    public String protocol() {
        return _delegate.protocol();
    }

    @Override
    public String toString() {
        return _delegate.toString();
    }

    @Override
    public String toDetailedString() {
        return toString();
    }

    @Override
    public com.zeroc.Ice.ConnectionInfo getInfo(
            boolean incoming, String adapterName, String connectionId) {
        assert incoming == _incoming;
        // adapterName is the name of the object adapter currently associated with this connection,
        // while _adapterName represents the name of the object adapter that created this connection
        // (incoming only).

        return new ConnectionInfo(
            _delegate.getInfo(incoming, adapterName, connectionId), _cipher, _certs, _verified);
    }

    @Override
    public void setBufferSize(int rcvSize, int sndSize) {
        _delegate.setBufferSize(rcvSize, sndSize);
    }

    @Override
    public void checkSendSize(Buffer buf) {
        _delegate.checkSendSize(buf);
    }

    TransceiverI(
            Instance instance,
            Transceiver delegate,
            String hostOrAdapterName,
            boolean incoming,
            SSLEngineFactory sslEngineFactory) {
        _instance = instance;
        _delegate = delegate;
        _incoming = incoming;
        _sslEngineFactory = sslEngineFactory;
        if (_incoming) {
            _adapterName = hostOrAdapterName;
        } else {
            _host = hostOrAdapterName;
        }
    }

    private int handshakeNonBlocking() {
        try {
            HandshakeStatus status = _engine.getHandshakeStatus();
            while (!_engine.isOutboundDone() && !_engine.isInboundDone()) {
                SSLEngineResult result = null;
                switch (status) {
                    case FINISHED, NOT_HANDSHAKING -> {
                        return SocketOperation.None;
                    }
                    case NEED_TASK -> {
                        Runnable task;
                        while ((task = _engine.getDelegatedTask()) != null) {
                            task.run();
                        }
                        status = _engine.getHandshakeStatus();
                    }
                    case NEED_UNWRAP -> {
                        if (_netInput.b.position() == 0) {
                            int s = _delegate.read(_netInput);
                            if (s != SocketOperation.None && _netInput.b.position() == 0) {
                                return s;
                            }
                        }

                        //
                        // The engine needs more data. We might already have enough data in
                        // the _netInput buffer to satisfy the engine. If not, the engine
                        // responds with BUFFER_UNDERFLOW and we'll read from the socket.
                        //
                        _netInput.flip();
                        result = _engine.unwrap(_netInput.b, _appInput);
                        _netInput.b.compact();
                        //
                        // FINISHED is only returned from wrap or unwrap, not from
                        // engine.getHandshakeResult().
                        //
                        status = result.getHandshakeStatus();
                        switch (result.getStatus()) {
                            case BUFFER_OVERFLOW -> throw new AssertionError();
                            case BUFFER_UNDERFLOW -> {
                                assert (status == SSLEngineResult.HandshakeStatus.NEED_UNWRAP);
                                int position = _netInput.b.position();
                                int s = _delegate.read(_netInput);
                                if (s != SocketOperation.None && _netInput.b.position() == position) {
                                    return s;
                                }
                            }
                            case CLOSED -> throw new ConnectionLostException();
                            case OK -> {}

                            // 1.9 introduced NEEDS_UNWRAP_AGAIN for DTLS
                            default -> throw new AssertionError();
                        }
                    }
                    case NEED_WRAP -> {
                        // The engine needs to send a message.
                        result = _engine.wrap(_emptyBuffer, _netOutput.b);
                        if (result.bytesProduced() > 0) {
                            int s = flushNonBlocking();
                            if (s != SocketOperation.None) {
                                return s;
                            }
                        }

                        // FINISHED is only returned from wrap or unwrap, not from engine.getHandshakeResult().
                        status = result.getHandshakeStatus();
                    }
                }

                if (result != null) {
                    switch (result.getStatus()) {
                        case BUFFER_OVERFLOW:
                            assert false;
                            break;
                        case BUFFER_UNDERFLOW:
                            // Need to read again.
                            assert (status == HandshakeStatus.NEED_UNWRAP);
                            break;
                        case CLOSED:
                            throw new ConnectionLostException();
                        case OK:
                            break;
                    }
                }
            }
        } catch (SSLException ex) {
            throw new SecurityException("SSL transport: handshake error", ex);
        }
        return SocketOperation.None;
    }

    private int writeNonBlocking(ByteBuffer buf) {
        //
        // This method has two purposes: encrypt the application's message buffer into our
        // _netOutput buffer, and write the contents of _netOutput to the socket without
        // blocking.
        //
        try {
            while (buf.hasRemaining() || _netOutput.b.position() > 0) {
                if (buf.hasRemaining()) {
                    //
                    // Encrypt the buffer.
                    //
                    SSLEngineResult result = _engine.wrap(buf, _netOutput.b);
                    switch (result.getStatus()) {
                        case BUFFER_OVERFLOW:
                            //
                            // Need to make room in _netOutput.b.
                            //
                            break;
                        case BUFFER_UNDERFLOW:
                            assert false;
                            break;
                        case CLOSED:
                            throw new ConnectionLostException();
                        case OK:
                            break;
                    }
                }

                //
                // Write the encrypted data to the socket. We continue writing until we've written
                // all of _netOutput, or until flushNonBlocking indicates that it cannot write
                // (i.e., by returning SocketOperation.Write).
                //
                if (_netOutput.b.position() > 0) {
                    int s = flushNonBlocking();
                    if (s != SocketOperation.None) {
                        return s;
                    }
                }
            }
        } catch (SSLException ex) {
            throw new SecurityException(
                "SSL transport: error while encoding message", ex);
        }

        assert (_netOutput.b.position() == 0);
        return SocketOperation.None;
    }

    private int flushNonBlocking() {
        _netOutput.flip();

        try {
            int s = _delegate.write(_netOutput);
            if (s != SocketOperation.None) {
                _netOutput.b.compact();
                return s;
            }
        } catch (SocketException ex) {
            throw new ConnectionLostException(ex);
        }
        // Cast to java.nio.Buffer to avoid incompatible covariant
        // return type used in Java 9 java.nio.ByteBuffer
        ((java.nio.Buffer) _netOutput.b).clear();
        return SocketOperation.None;
    }

    private void fill(ByteBuffer buf) {
        ((java.nio.Buffer) _appInput).flip();
        if (_appInput.hasRemaining()) {
            int bytesAvailable = _appInput.remaining();
            int bytesNeeded = buf.remaining();
            if (bytesAvailable > bytesNeeded) {
                bytesAvailable = bytesNeeded;
            }
            if (buf.hasArray()) {
                //
                // Copy directly into the destination buffer's backing array.
                //
                byte[] arr = buf.array();
                _appInput.get(arr, buf.arrayOffset() + buf.position(), bytesAvailable);
                // Cast to java.nio.Buffer to avoid incompatible covariant
                // return type used in Java 9 java.nio.ByteBuffer
                ((java.nio.Buffer) buf).position(buf.position() + bytesAvailable);
            } else if (_appInput.hasArray()) {
                //
                // Copy directly from the source buffer's backing array.
                //
                byte[] arr = _appInput.array();
                buf.put(arr, _appInput.arrayOffset() + _appInput.position(), bytesAvailable);
                // Cast to java.nio.Buffer to avoid incompatible covariant
                // return type used in Java 9 java.nio.ByteBuffer
                ((java.nio.Buffer) _appInput).position(_appInput.position() + bytesAvailable);
            } else {
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

    private final Instance _instance;
    private final Transceiver _delegate;
    private SSLEngine _engine;
    private String _host = "";
    private String _adapterName = "";
    private final boolean _incoming;
    private ReadyCallback _readyCallback;
    private boolean _isConnected;

    private ByteBuffer _appInput; // Holds clear-text data to be read by the application.
    private Buffer _netInput; // Holds encrypted data read from the socket.
    private Buffer _netOutput; // Holds encrypted data to be written to the socket.
    private static final ByteBuffer _emptyBuffer = ByteBuffer.allocate(0); // Used during handshaking.

    private String _cipher;
    private Certificate[] _certs;
    private boolean _verified;
    private final SSLEngineFactory _sslEngineFactory;
}
