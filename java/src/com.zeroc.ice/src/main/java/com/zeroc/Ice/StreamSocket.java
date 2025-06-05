// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;

class StreamSocket {
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
            _fd = null; // Necessary for the finalizer
            throw ex;
        }

        _desc = Network.fdToString(_fd, _proxy, _addr);
    }

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
            _fd = null; // Necessary for the finalizer
            throw ex;
        }

        _desc = Network.fdToString(_fd);
    }

    @SuppressWarnings({"nofinalizer", "deprecation"})
    @Override
    protected synchronized void finalize() throws Throwable {
        try {
            Assert.FinalizerAssert(_fd == null);
        } catch (Exception ex) {} finally {
            super.finalize();
        }
    }

    public void setBufferSize(int rcvSize, int sndSize) {
        Network.setTcpBufSize(_fd, rcvSize, sndSize, _instance);
    }

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

    public boolean isConnected() {
        return _state == StateConnected;
    }

    public SocketChannel fd() {
        return _fd;
    }

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

    public int write(ByteBuffer buf) {
        assert (_fd != null);

        int sent = 0;
        while (buf.hasRemaining()) {
            try {
                int ret;
                if (_maxSendPacketSize > 0 && buf.remaining() > _maxSendPacketSize) {
                    int previous = buf.limit();
                    // Cast to java.nio.Buffer to avoid incompatible covariant
                    // return type used in Java 9 java.nio.ByteBuffer
                    ((java.nio.Buffer) buf).limit(buf.position() + _maxSendPacketSize);
                    ret = _fd.write(buf);
                    ((java.nio.Buffer) buf).limit(previous);
                } else {
                    ret = _fd.write(buf);
                }

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

    public void close() {
        assert (_fd != null);
        try {
            _fd.close();
        } catch (IOException ex) {
            throw new SocketException(ex);
        } finally {
            _fd = null;
        }
    }

    @Override
    public String toString() {
        return _desc;
    }

    private void init() {
        Network.setBlock(_fd, false);
        Network.setTcpBufSize(_fd, _instance);

        if (System.getProperty("os.name").startsWith("Windows")) {
            //
            // On Windows, limiting the buffer size is important to prevent poor throughput
            // performances when sending large amount of data. See Microsoft KB article KB823764.
            //
            _maxSendPacketSize = java.lang.Math.max(512, Network.getSendBufferSize(_fd) / 2);
        } else {
            _maxSendPacketSize = 0;
        }
    }

    private int toState(int operation) {
        switch (operation) {
            case SocketOperation.Read:
                return StateProxyRead;
            case SocketOperation.Write:
                return StateProxyWrite;
            default:
                return StateProxyConnected;
        }
    }

    private final ProtocolInstance _instance;

    private final NetworkProxy _proxy;
    private final InetSocketAddress _addr;

    private SocketChannel _fd;
    private int _maxSendPacketSize;
    private int _state;
    private String _desc;

    private static final int StateNeedConnect = 0;
    private static final int StateConnectPending = 1;
    private static final int StateProxyRead = 2;
    private static final int StateProxyWrite = 3;
    private static final int StateProxyConnected = 4;
    private static final int StateConnected = 5;
}
