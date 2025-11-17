// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * @hidden Public because it's used by IceBT, SSL, and the 'Ice/background' test.
 */
public interface ReadyCallback {
    /**
     * Sets the transceiver read or write readiness status. This method is used by a transceiver
     * implementation to notify the thread pool whether it's ready (or not) to write or read data.
     *
     * @param op specifies whether this notification is for read or write readiness.
     *     Must be one of {@link SocketOperation#Read} or {@link SocketOperation#Write}.
     * @param value {@code true} if the specified operation is ready, {@code false} otherwise.
     */
    void ready(int op, boolean value);
}
