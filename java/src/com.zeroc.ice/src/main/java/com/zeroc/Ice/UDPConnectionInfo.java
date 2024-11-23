// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Provides access to the connection details of a UDP connection */
public final class UDPConnectionInfo extends IPConnectionInfo {
    /** The multicast address. */
    public String mcastAddress = "";

    /** The multicast port. */
    public int mcastPort = -1;

    /** The connection buffer receive size. */
    public int rcvSize;

    /** The connection buffer send size. */
    public int sndSize;
}
