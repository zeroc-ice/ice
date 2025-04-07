// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.net.Socket;
import java.nio.channels.SelectableChannel;

final class TcpTransceiver implements Transceiver {
    @Override
    public SelectableChannel fd() {
        assert (_stream != null);
        return _stream.fd();
    }

    @Override
    public void setReadyCallback(ReadyCallback callback) {
        // No need of the callback
    }

    @Override
    public int initialize(Buffer readBuffer, Buffer writeBuffer) {
        return _stream.connect(readBuffer, writeBuffer);
    }

    @Override
    public int closing(boolean initiator, LocalException ex) {
        // If we are initiating the connection closure, wait for the peer to close the TCP/IP
        // connection. Otherwise, close immediately.
        return initiator ? SocketOperation.Read : SocketOperation.None;
    }

    @Override
    public void close() {
        _stream.close();
    }

    @Override
    public EndpointI bind() {
        assert false;
        return null;
    }

    @Override
    public int write(Buffer buf) {
        return _stream.write(buf);
    }

    @Override
    public int read(Buffer buf) {
        return _stream.read(buf);
    }

    @Override
    public String protocol() {
        return _instance.protocol();
    }

    @Override
    public String toString() {
        return _stream.toString();
    }

    @Override
    public String toDetailedString() {
        return toString();
    }

    @Override
    public ConnectionInfo getInfo(boolean incoming, String adapterName, String connectionId) {
        if (_stream.fd() == null) {
            return new TCPConnectionInfo(incoming, adapterName, connectionId);
        } else {
            Socket socket = _stream.fd().socket();

            String remoteAddress = "";
            int remotePort = -1;
            if (socket.getInetAddress() != null) {
                remoteAddress = socket.getInetAddress().getHostAddress();
                remotePort = socket.getPort();
            }

            int rcvSize = 0;
            int sndSize = 0;
            if (!socket.isClosed()) {
                rcvSize = Network.getRecvBufferSize(_stream.fd());
                sndSize = Network.getSendBufferSize(_stream.fd());
            }

            return new TCPConnectionInfo(
                    incoming,
                    adapterName,
                    connectionId,
                    socket.getLocalAddress().getHostAddress(),
                    socket.getLocalPort(),
                    remoteAddress,
                    remotePort,
                    rcvSize,
                    sndSize);
        }
    }

    @Override
    public void checkSendSize(Buffer buf) {}

    @Override
    public void setBufferSize(int rcvSize, int sndSize) {
        _stream.setBufferSize(rcvSize, sndSize);
    }

    TcpTransceiver(ProtocolInstance instance, StreamSocket stream) {
        _instance = instance;
        _stream = stream;
    }

    private final ProtocolInstance _instance;
    private final StreamSocket _stream;
}
