// Copyright (c) ZeroC, Inc.

package com.zeroc.IceBT;

import com.zeroc.Ice.Properties;

/**
 * The Bluetooth send and receive buffer sizes, as configured by the IceBT.SndSize and IceBT.RcvSize
 * properties.
 */
final class BTBufSize {
    private static final int dfltBufSize = 128 * 1024;

    private final int _rcvSize;
    private final int _sndSize;

    /**
     * Reads the IceBT.RcvSize and IceBT.SndSize properties.
     *
     * @throws com.zeroc.Ice.PropertyException if a property value is not a valid integer
     */
    BTBufSize(Properties properties) {
        _rcvSize = properties.getPropertyAsIntWithDefault("IceBT.RcvSize", dfltBufSize);
        _sndSize = properties.getPropertyAsIntWithDefault("IceBT.SndSize", dfltBufSize);
    }

    int rcvSize() {
        return _rcvSize;
    }

    int sndSize() {
        return _sndSize;
    }
}
