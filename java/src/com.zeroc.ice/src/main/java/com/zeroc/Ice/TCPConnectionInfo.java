// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Provides access to the connection details of a TCP connection */
public final class TCPConnectionInfo extends IPConnectionInfo {
    /** The connection buffer receive size. */
    public int rcvSize;

    /** The connection buffer send size. */
    public int sndSize;
}
