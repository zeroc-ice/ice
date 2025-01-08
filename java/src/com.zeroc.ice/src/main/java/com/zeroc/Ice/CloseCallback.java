// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * An application can implement this interface to receive notifications when a connection closes.
 *
 * @see Connection#setCloseCallback
 */
@FunctionalInterface
public interface CloseCallback {
    /**
     * This method is called by the connection when the connection is closed. If the callback needs
     * more information about the closure, it can call {@link Connection#throwException}.
     *
     * @param con The connection that closed.
     */
    void closed(Connection con);
}
