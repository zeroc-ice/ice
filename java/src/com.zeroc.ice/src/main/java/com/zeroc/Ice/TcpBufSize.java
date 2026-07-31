// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * The TCP send and receive buffer sizes, as configured by the Ice.TCP.SndSize and Ice.TCP.RcvSize
 * properties.
 */
final class TcpBufSize {
    // By default, on Windows we use a 128KB buffer size. On Unix platforms, we use the system
    // defaults.
    private static final int dfltBufSize =
        System.getProperty("os.name").startsWith("Windows") ? 128 * 1024 : 0;

    private final int _rcvSize;
    private final int _sndSize;

    /**
     * Reads the Ice.TCP.RcvSize and Ice.TCP.SndSize properties. Throws PropertyException if a value
     * is not a valid integer.
     */
    TcpBufSize(Properties properties) {
        _rcvSize = properties.getPropertyAsIntWithDefault("Ice.TCP.RcvSize", dfltBufSize);
        _sndSize = properties.getPropertyAsIntWithDefault("Ice.TCP.SndSize", dfltBufSize);
    }

    int rcvSize() {
        return _rcvSize;
    }

    int sndSize() {
        return _sndSize;
    }
}
