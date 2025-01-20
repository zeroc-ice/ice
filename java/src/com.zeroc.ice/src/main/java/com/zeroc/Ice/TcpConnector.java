// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

final class TcpConnector implements Connector {
    @Override
    public Transceiver connect() {
        return new TcpTransceiver(
                _instance, new StreamSocket(_instance, _proxy, _addr, _sourceAddr));
    }

    @Override
    public short type() {
        return _instance.type();
    }

    @Override
    public String toString() {
        return Network.addrToString(_proxy == null ? _addr : _proxy.getAddress());
    }

    @Override
    public int hashCode() {
        int h = 5381;
        h = HashUtil.hashAdd(h, _addr.getAddress().getHostAddress());
        h = HashUtil.hashAdd(h, _addr.getPort());
        if (_sourceAddr != null) {
            h = HashUtil.hashAdd(h, _sourceAddr.getAddress().getHostAddress());
        }
        h = HashUtil.hashAdd(h, _timeout);
        h = HashUtil.hashAdd(h, _connectionId);
        return h;
    }

    //
    // Only for use by TcpEndpoint
    //
    TcpConnector(
            ProtocolInstance instance,
            java.net.InetSocketAddress addr,
            NetworkProxy proxy,
            java.net.InetSocketAddress sourceAddr,
            int timeout,
            String connectionId) {
        _instance = instance;
        _addr = addr;
        _proxy = proxy;
        _sourceAddr = sourceAddr;
        _timeout = timeout;
        _connectionId = connectionId;
    }

    @Override
    public boolean equals(java.lang.Object obj) {
        if (!(obj instanceof TcpConnector)) {
            return false;
        }

        if (this == obj) {
            return true;
        }

        TcpConnector p = (TcpConnector) obj;
        if (_timeout != p._timeout) {
            return false;
        }

        if (!_connectionId.equals(p._connectionId)) {
            return false;
        }

        if (!java.util.Objects.equals(_sourceAddr, p._sourceAddr)) {
            return false;
        }

        return java.util.Objects.equals(_addr, p._addr);
    }

    private final ProtocolInstance _instance;
    private java.net.InetSocketAddress _addr;
    private NetworkProxy _proxy;
    private java.net.InetSocketAddress _sourceAddr;
    private int _timeout;
    private String _connectionId = "";
}
