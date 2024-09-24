// Copyright (c) ZeroC, Inc.

package com.zeroc.IceBT;

import com.zeroc.Ice.Connector;
import com.zeroc.Ice.HashUtil;
import com.zeroc.Ice.Transceiver;

final class ConnectorI implements Connector {
    @Override
    public Transceiver connect() {
        return new TransceiverI(_instance, _addr, _uuid, _connectionId);
    }

    @Override
    public short type() {
        return _instance.type();
    }

    @Override
    public String toString() {
        StringBuffer buf = new StringBuffer();
        if (!_addr.isEmpty()) {
            buf.append(_addr);
        }
        if (!_uuid.isEmpty()) {
            if (!_addr.isEmpty()) {
                buf.append(';');
            }
            buf.append(_uuid);
        }
        return buf.toString();
    }

    @Override
    public int hashCode() {
        int h = 5381;
        h = HashUtil.hashAdd(h, _addr);
        h = HashUtil.hashAdd(h, _uuid);
        h = HashUtil.hashAdd(h, _timeout);
        h = HashUtil.hashAdd(h, _connectionId);
        return h;
    }

    // Only for use by EndpointI.
    ConnectorI(Instance instance, String addr, String uuid, int timeout, String connectionId) {
        _instance = instance;
        _addr = addr;
        _uuid = uuid;
        _timeout = timeout;
        _connectionId = connectionId;
    }

    @Override
    public boolean equals(java.lang.Object obj) {
        if (!(obj instanceof ConnectorI)) {
            return false;
        }

        if (this == obj) {
            return true;
        }

        ConnectorI p = (ConnectorI) obj;
        if (!_uuid.equals(p._uuid)) {
            return false;
        }

        if (_timeout != p._timeout) {
            return false;
        }

        if (!_connectionId.equals(p._connectionId)) {
            return false;
        }

        return _addr.equals(p._addr);
    }

    private final Instance _instance;
    private final String _addr;
    private final String _uuid;
    private final int _timeout;
    private final String _connectionId;
}
