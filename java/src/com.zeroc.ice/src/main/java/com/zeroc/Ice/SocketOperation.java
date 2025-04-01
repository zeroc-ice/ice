// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.nio.channels.SelectionKey;

/**
 * @hidden Public because it's used by IceBT, SSL, and the 'Ice/background' test.
 */
public class SocketOperation {
    public static final int None = 0;
    public static final int Read = SelectionKey.OP_READ;
    public static final int Write = SelectionKey.OP_WRITE;
    public static final int Connect = SelectionKey.OP_CONNECT;

    private SocketOperation() {
    }
}
