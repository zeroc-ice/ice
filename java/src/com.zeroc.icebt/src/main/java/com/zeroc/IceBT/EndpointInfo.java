// Copyright (c) ZeroC, Inc.

package com.zeroc.IceBT;

/** Provides access to Bluetooth endpoint information. */
public final class EndpointInfo extends com.zeroc.Ice.EndpointInfo {
    /** The address configured with the endpoint. */
    public final String addr;

    /** The UUID configured with the endpoint. */
    public final String uuid;

    private final short _type;
    private final boolean _secure;

    @Override
    public short type() {
        return _type;
    }

    @Override
    public boolean secure() {
        return _secure;
    }

    EndpointInfo(
            int timeout, boolean compress, String addr, String uuid, short type, boolean secure) {
        super(timeout, compress);
        this.addr = addr;
        this.uuid = uuid;
        this._type = type;
        this._secure = secure;
    }
}
