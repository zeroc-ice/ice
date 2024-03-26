//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceSSL;

/**
 * A factory for javax.net.ssl.SSLEngine objects.
 **/
@FunctionalInterface
public interface SSLEngineFactory
{
    /**
     * Creates a new SSLEngine instance for SSL transport. The IceSSL SSL transport transceiver class
     * uses this method to obtain an SSLEngine object when creating a connection.
     * @param peerHost The hostname or IP address of the peer.
     * @param peerPort The port number of the peer. For outgoing connections, this is always set to -1.
     * @return The new SSLEngine instance.
     */
    javax.net.ssl.SSLEngine create(String peerHost, int peerPort);
}
