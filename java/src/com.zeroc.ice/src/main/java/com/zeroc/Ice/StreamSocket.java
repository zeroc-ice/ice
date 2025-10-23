// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;

class StreamSocket {
    /**
     * Constructs a StreamSocket and initiates a connection to the specified address.
     *
     * @param instance The protocol instance.
     * @param proxy The network proxy, or null if not using a proxy.
     * @param addr The remote address to connect to.
     * @param sourceAddr The local address to bind to, or null for default.
     * @throws LocalException if the socket cannot be created or connected.
     */
    public StreamSocket(
            ProtocolInstance instance,
            NetworkProxy proxy,
            InetSocketAddress addr,
            InetSocketAddress sourceAddr) {
        _instance = instance;
        _proxy = proxy;
        _addr = addr;
        _fd = Network.createTcpSocket();
        _state = StateNeedConnect;

        try {
            init();
            if (Network.doConnect(_fd, _proxy != null ? _proxy.getAddress() : _addr, sourceAddr)) {
                _state = _proxy != null ? StateProxyWrite : StateConnected;
            }
        } catch (LocalException ex) {
            assert (!_fd.isOpen());
            throw ex;
        }

        _desc = Network.fdToString(_fd, _proxy, _addr);
    }

    /**
     * Constructs a StreamSocket from an existing SocketChannel.
     *
     * @param instance The protocol instance.
     * @param fd The connected SocketChannel.
     * @throws LocalException if initialization fails.
     */
    public StreamSocket(ProtocolInstance instance, SocketChannel fd) {
        _instance = instance;
        _proxy = null;
        _addr = null;
        _fd = fd;
        _state = StateConnected;

        try {
            init();
        } catch (LocalException ex) {
            assert (!_fd.isOpen());
            throw ex;
        }

        _desc = Network.fdToString(_fd);
    }

    /**
     * Sets the receive and send buffer sizes for the socket.
     *
     * @param rcvSize The receive buffer size in bytes.
     * @param sndSize The send buffer size in bytes.
     */
    public void setBufferSize(int rcvSize, int sndSize) {
        Network.setTcpBufSize(_fd, rcvSize, sndSize, _instance);
    }

    /**
     * Initiates or completes a connection, including proxy handshake if needed.
     *
     * @param readBuffer The buffer for reading data.
     * @param writeBuffer The buffer for writing data.
     * @return The next SocketOperation to perform, or SocketOperation.None if connected.
     */
    public int connect(Buffer readBuffer, Buffer writeBuffer) {
        if (_state == StateNeedConnect) {
            _state = StateConnectPending;
            return SocketOperation.Connect;
        } else if (_state <= StateConnectPending) {
            Network.doFinishConnect(_fd);
            _desc = Network.fdToString(_fd, _proxy, _addr);
            _state = _proxy != null ? StateProxyWrite : StateConnected;
        }

        if (_state == StateProxyWrite) {
            _proxy.beginWrite(_addr, writeBuffer);
            return SocketOperation.Write;
        } else if (_state == StateProxyRead) {
            _proxy.beginRead(readBuffer);
            return SocketOperation.Read;
        } else if (_state == StateProxyConnected) {
            _proxy.finish(readBuffer, writeBuffer);

            readBuffer.clear();
            writeBuffer.clear();

            _state = StateConnected;
        }

        assert (_state == StateConnected);
        return SocketOperation.None;
    }

    /**
     * Returns true if the socket is connected.
     *
     * @return true if connected, false otherwise.
     */
    public boolean isConnected() {
        return _state == StateConnected;
    }

    /**
     * Returns the underlying SocketChannel.
     *
     * @return The SocketChannel for this socket.
     */
    public SocketChannel fd() {
        return _fd;
    }

    /**
     * Reads data into the provided buffer.
     *
     * @param buf The buffer to read data into.
     * @return The next SocketOperation to perform, or SocketOperation.None if done.
     */
    public int read(Buffer buf) {
        if (_state == StateProxyRead) {
            while (true) {
                int ret = read(buf.b);
                if (ret == 0) {
                    return SocketOperation.Read;
                }
                _state = toState(_proxy.endRead(buf));
                if (_state != StateProxyRead) {
                    return SocketOperation.None;
                }
            }
        }
        read(buf.b);
        return buf.b.hasRemaining() ? SocketOperation.Read : SocketOperation.None;
    }

    /**
     * Writes data from the provided buffer.
     *
     * @param buf The buffer containing data to write.
     * @return The next SocketOperation to perform, or SocketOperation.None if done.
     */
    public int write(Buffer buf) {
        if (_state == StateProxyWrite) {
            while (true) {
                int ret = write(buf.b);
                if (ret == 0) {
                    return SocketOperation.Write;
                }
                _state = toState(_proxy.endWrite(buf));
                if (_state != StateProxyWrite) {
                    return SocketOperation.None;
                }
            }
        }
        write(buf.b);
        return buf.b.hasRemaining() ? SocketOperation.Write : SocketOperation.None;
    }

    /**
     * Reads bytes from the socket into the given ByteBuffer.
     *
     * @param buf The ByteBuffer to read data into.
     * @return The number of bytes read.
     * @throws ConnectionLostException if the connection is closed.
     */
    public int read(ByteBuffer buf) {
        assert (_fd != null);

        int read = 0;

        while (buf.hasRemaining()) {
            try {
                int ret = _fd.read(buf);
                if (ret == -1) {
                    throw new ConnectionLostException();
                } else if (ret == 0) {
                    return read;
                }

                read += ret;
            } catch (IOException ex) {
                throw new ConnectionLostException(ex);
            }
        }
        return read;
    }

    /**
     * Writes bytes from the given ByteBuffer to the socket.
     *
     * @param buf The ByteBuffer containing data to write.
     * @return The number of bytes written.
     * @throws ConnectionLostException or SocketException if the connection is closed or an error occurs.
     */
    public int write(ByteBuffer buf) {
        assert (_fd != null);

        int sent = 0;
        while (buf.hasRemaining()) {
            try {
                int ret = _fd.write(buf);
                if (ret == -1) {
                    throw new ConnectionLostException();
                } else if (ret == 0) {
                    return sent;
                }
                sent += ret;
            } catch (IOException ex) {
                throw new SocketException(ex);
            }
        }
        return sent;
    }

    /**
     * Closes the socket.
     *
     * @throws SocketException if an error occurs while closing the socket.
     */
    public void close() {
        assert (_fd != null);
        try {
            _fd.close();
        } catch (IOException ex) {
            throw new SocketException(ex);
        }
    }

    /**
     * Returns a string representation of the socket.
     *
     * @return A string describing the socket.
     */
    @Override
    public String toString() {
        return _desc;
    }

    private void init() {
        Network.setBlock(_fd, false);
        Network.setTcpBufSize(_fd, _instance);
    }

    private int toState(int operation) {
        return switch (operation) {
            case SocketOperation.Read -> StateProxyRead;
            case SocketOperation.Write -> StateProxyWrite;
            default -> StateProxyConnected;
        };
    }

    private final ProtocolInstance _instance;

    private final NetworkProxy _proxy;
    private final InetSocketAddress _addr;
    private final SocketChannel _fd;

    private int _state;
    private String _desc;

    private static final int StateNeedConnect = 0;
    private static final int StateConnectPending = 1;
    private static final int StateProxyRead = 2;
    private static final int StateProxyWrite = 3;
    private static final int StateProxyConnected = 4;
    private static final int StateConnected = 5;
}
