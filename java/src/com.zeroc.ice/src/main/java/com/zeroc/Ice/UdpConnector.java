//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

final class UdpConnector implements Connector {
    @Override
    public Transceiver connect() {
        if (Util.isAndroid() && _addr.getAddress().isMulticastAddress()) {
            return new UdpMulticastClientTransceiver(_instance, _addr, _mcastInterface, _mcastTtl);
        } else {
            return new UdpTransceiver(_instance, _addr, _sourceAddr, _mcastInterface, _mcastTtl);
        }
    }

    public java.nio.channels.SelectableChannel fd() {
        assert (false); // Shouldn't be called, startConnect always completes immediately.
        return null;
    }

    @Override
    public short type() {
        return _instance.type();
    }

    @Override
    public String toString() {
        return Network.addrToString(_addr);
    }

    @Override
    public int hashCode() {
        int h = 5381;
        h = HashUtil.hashAdd(h, _addr.getAddress().getHostAddress());
        h = HashUtil.hashAdd(h, _addr.getPort());
        if (_sourceAddr != null) {
            h = HashUtil.hashAdd(h, _sourceAddr.getAddress().getHostAddress());
        }
        h = HashUtil.hashAdd(h, _mcastInterface);
        h = HashUtil.hashAdd(h, _mcastTtl);
        h = HashUtil.hashAdd(h, _connectionId);
        return h;
    }

    //
    // Only for use by UdpEndpointI
    //
    UdpConnector(
            ProtocolInstance instance,
            java.net.InetSocketAddress addr,
            java.net.InetSocketAddress sourceAddr,
            String mcastInterface,
            int mcastTtl,
            String connectionId) {
        _instance = instance;
        _addr = addr;
        _sourceAddr = sourceAddr;
        _mcastInterface = mcastInterface;
        _mcastTtl = mcastTtl;
        _connectionId = connectionId;
    }

    @Override
    public boolean equals(java.lang.Object obj) {
        if (!(obj instanceof UdpConnector)) {
            return false;
        }

        if (this == obj) {
            return true;
        }

        UdpConnector p = (UdpConnector) obj;
        if (!_connectionId.equals(p._connectionId)) {
            return false;
        }

        if (_mcastTtl != p._mcastTtl) {
            return false;
        }

        if (_mcastInterface.compareTo(p._mcastInterface) != 0) {
            return false;
        }

        if (!java.util.Objects.equals(_sourceAddr, p._sourceAddr)) {
            return false;
        }

        return java.util.Objects.equals(_addr, p._addr);
    }

    private final ProtocolInstance _instance;
    private final java.net.InetSocketAddress _addr;
    private final java.net.InetSocketAddress _sourceAddr;
    private final String _mcastInterface;
    private final int _mcastTtl;
    private final String _connectionId;
}
