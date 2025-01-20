// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice.SSL;

/** A factory for javax.net.ssl.SSLEngine objects. */
@FunctionalInterface
public interface SSLEngineFactory {
    /**
     * Creates a new SSLEngine instance used to configure the ssl transport.
     *
     * @param peerHost The hostname or IP address of the peer.
     * @param peerPort The port number of the peer. For outgoing connections, this is always set to
     *     -1.
     * @return The new SSLEngine instance.
     */
    javax.net.ssl.SSLEngine createSSLEngine(String peerHost, int peerPort);
}
