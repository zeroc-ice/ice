// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

final class SOCKSNetworkProxy implements NetworkProxy {
    public SOCKSNetworkProxy(String host, int port) {
        _host = host;
        _port = port;
    }

    private SOCKSNetworkProxy(java.net.InetSocketAddress address) {
        _address = address;
    }

    @Override
    public void beginWrite(java.net.InetSocketAddress endpoint, Buffer buf) {
        final java.net.InetAddress addr = endpoint.getAddress();
        if (addr == null) {
            throw new FeatureNotSupportedException("SOCKS4 does not support domain names");
        } else if (!(addr instanceof java.net.Inet4Address)) {
            throw new FeatureNotSupportedException("SOCKS4 only supports IPv4 addresses");
        }

        //
        // SOCKS connect request
        //
        buf.resize(9, false);
        final java.nio.ByteOrder order = buf.b.order();
        buf.b.order(java.nio.ByteOrder.BIG_ENDIAN); // Network byte order.
        buf.position(0);
        buf.b.put((byte) 0x04); // SOCKS version 4.
        buf.b.put((byte) 0x01); // Command, establish a TCP/IP stream connection
        buf.b.putShort((short) endpoint.getPort()); // Port
        buf.b.put(addr.getAddress()); // IPv4 address
        buf.b.put((byte) 0x00); // User ID.
        buf.position(0);
        buf.limit(buf.size());
        buf.b.order(order);
    }

    @Override
    public int endWrite(Buffer buf) {
        // Once the request is sent, read the response
        return buf.b.hasRemaining() ? SocketOperation.Write : SocketOperation.Read;
    }

    @Override
    public void beginRead(Buffer buf) {
        //
        // Read the SOCKS4 response whose size is 8 bytes.
        //
        if (!buf.b.hasRemaining()) {
            buf.resize(8, true);
            buf.position(0);
        }
    }

    @Override
    public int endRead(Buffer buf) {
        // We're done once we read the response
        return buf.b.hasRemaining() ? SocketOperation.Read : SocketOperation.None;
    }

    @Override
    public void finish(Buffer readBuffer, Buffer writeBuffer) {
        readBuffer.position(0);
        byte b1 = readBuffer.b.get();
        byte b2 = readBuffer.b.get();
        if (b1 != 0x00 || b2 != 0x5a) {
            throw new ConnectFailedException();
        }
    }

    @Override
    public NetworkProxy resolveHost(int protocolSupport) {
        assert (_host != null);
        return new SOCKSNetworkProxy(
                Network.getAddresses(_host, _port, protocolSupport, false, true).get(0));
    }

    @Override
    public java.net.InetSocketAddress getAddress() {
        assert (_address != null); // Host must be resolved.
        return _address;
    }

    @Override
    public String getName() {
        return "SOCKS";
    }

    @Override
    public int getProtocolSupport() {
        return Network.EnableIPv4;
    }

    private String _host;
    private int _port;
    private java.net.InetSocketAddress _address;
}
