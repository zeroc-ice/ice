// Copyright (c) ZeroC, Inc.

package com.zeroc.IceBT;

/** Provides access to the details of a Bluetooth connection. */
public final class ConnectionInfo extends com.zeroc.Ice.ConnectionInfo {
    /** The local Bluetooth address. */
    public String localAddress = "";

    /** The local RFCOMM channel. */
    public int localChannel = -1;

    /** The remote Bluetooth address. */
    public String remoteAddress = "";

    /** The remote RFCOMM channel. */
    public int remoteChannel = -1;

    /** The UUID of the service being offered (in a server) or targeted (in a client). */
    public String uuid = "";

    /** The connection buffer receive size. */
    public int rcvSize;

    /** The connection buffer send size. */
    public int sndSize;
}
