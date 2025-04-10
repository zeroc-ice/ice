// Copyright (c) ZeroC, Inc.

package test.Ice.background;

import com.zeroc.Ice.Buffer;
import com.zeroc.Ice.ConnectionInfo;
import com.zeroc.Ice.EndpointI;
import com.zeroc.Ice.LocalException;
import com.zeroc.Ice.ReadyCallback;
import com.zeroc.Ice.SocketOperation;

import java.nio.channels.SelectableChannel;

final class Transceiver implements com.zeroc.Ice.Transceiver {
    @Override
    public SelectableChannel fd() {
        return _transceiver.fd();
    }

    @Override
    public void setReadyCallback(ReadyCallback callback) {
        _transceiver.setReadyCallback(callback);
        _readyCallback = callback;
    }

    @Override
    public int initialize(Buffer readBuffer, Buffer writeBuffer) {
        int status = _configuration.initializeSocketStatus();
        if (status == SocketOperation.Connect) {
            return status;
        } else if (status == SocketOperation.Write) {
            if (!_initialized) {
                status = _transceiver.initialize(readBuffer, writeBuffer);
                if (status != SocketOperation.None) {
                    return status;
                }
                _initialized = true;
            }
            return SocketOperation.Write;
        } else if (status == SocketOperation.Read) {
            return status;
        }

        _configuration.checkInitializeException();
        if (!_initialized) {
            status = _transceiver.initialize(readBuffer, writeBuffer);
            if (status != SocketOperation.None) {
                return status;
            }
            _initialized = true;
        }
        return SocketOperation.None;
    }

    @Override
    public int closing(boolean initiator, LocalException ex) {
        return _transceiver.closing(initiator, ex);
    }

    @Override
    public void close() {
        _transceiver.close();
    }

    @Override
    public EndpointI bind() {
        return _transceiver.bind();
    }

    @Override
    public int write(Buffer buf) {
        if (!_configuration.writeReady() && buf.b.hasRemaining()) {
            return SocketOperation.Write;
        }

        _configuration.checkWriteException();
        return _transceiver.write(buf);
    }

    @Override
    public int read(Buffer buf) {
        if (!_configuration.readReady() && buf.b.hasRemaining()) {
            return SocketOperation.Read;
        }

        _configuration.checkReadException();

        if (_buffered) {
            while (buf.b.hasRemaining()) {
                if (_readBufferPos == _readBuffer.b.position()) {
                    _readBufferPos = 0;
                    _readBuffer.position(0);
                    _transceiver.read(_readBuffer);
                    if (_readBufferPos == _readBuffer.b.position()) {
                        _readyCallback.ready(SocketOperation.Read, false);
                        return SocketOperation.Read;
                    }
                }
                final int pos = _readBuffer.b.position();
                assert (pos > _readBufferPos);
                final int requested = buf.b.remaining();
                int available = pos - _readBufferPos;
                assert (available > 0);
                if (available >= requested) {
                    available = requested;
                }

                byte[] arr = new byte[available];
                _readBuffer.position(_readBufferPos);
                _readBuffer.b.get(arr);
                buf.b.put(arr);
                _readBufferPos += available;
                _readBuffer.position(pos);
            }

            _readyCallback.ready(SocketOperation.Read, _readBufferPos < _readBuffer.b.position());
            return SocketOperation.None;
        } else {
            return _transceiver.read(buf);
        }
    }

    @Override
    public String protocol() {
        return "test-" + _transceiver.protocol();
    }

    @Override
    public String toString() {
        return _transceiver.toString();
    }

    @Override
    public String toDetailedString() {
        return _transceiver.toDetailedString();
    }

    @Override
    public ConnectionInfo getInfo(
            boolean incoming, String adapterName, String connectionId) {
        return _transceiver.getInfo(incoming, adapterName, connectionId);
    }

    @Override
    public void checkSendSize(Buffer buf) {
        _transceiver.checkSendSize(buf);
    }

    @Override
    public void setBufferSize(int rcvSize, int sndSize) {
        _transceiver.setBufferSize(rcvSize, sndSize);
    }

    public com.zeroc.Ice.Transceiver delegate() {
        return _transceiver;
    }

    //
    // Only for use by Connector, Acceptor
    //
    Transceiver(Configuration configuration, com.zeroc.Ice.Transceiver transceiver) {
        _transceiver = transceiver;
        _configuration = configuration;
        _initialized = false;
        _buffered = _configuration.buffered();
        _readBuffer = new Buffer(false);
        _readBuffer.resize(1024 * 8, true); // 8KB buffer
        _readBufferPos = 0;
    }

    private final com.zeroc.Ice.Transceiver _transceiver;
    private final Configuration _configuration;
    private ReadyCallback _readyCallback;
    private boolean _initialized;
    private final boolean _buffered;
    private final Buffer _readBuffer;
    private int _readBufferPos;
}
