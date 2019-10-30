//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * An application can implement this interface to receive notifications when
 * a connection receives a heartbeat message.
 *
 * @see Connection#setHeartbeatCallback
 **/
@FunctionalInterface
public interface HeartbeatCallback
{
    /**
     * This method is called by the the connection when a heartbeat is
     * received from the peer.
     * @param con The connection on which a heartbeat was received.
     **/
    void heartbeat(Connection con);
}
