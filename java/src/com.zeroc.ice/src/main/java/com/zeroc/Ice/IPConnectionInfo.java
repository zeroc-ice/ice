// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Provides access to the connection details of an IP connection */
public class IPConnectionInfo extends ConnectionInfo {
    /** The local address. */
    public String localAddress = "";

    /** The local port. */
    public int localPort = -1;

    /** The remote address. */
    public String remoteAddress = "";

    /** The remote port. */
    public int remotePort = -1;
}
