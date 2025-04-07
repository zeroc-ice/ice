// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.nio.channels.ServerSocketChannel;

/**
 * @hidden Public because it's used by IceBT and SSL.
 */
public interface Acceptor {
    ServerSocketChannel fd();

    void setReadyCallback(ReadyCallback callback);

    void close();

    EndpointI listen();

    Transceiver accept();

    String protocol();

    @Override
    String toString();

    String toDetailedString();
}
