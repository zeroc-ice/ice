// Copyright (c) ZeroC, Inc.

package com.zeroc.IceBT;

/** Provides access to Bluetooth endpoint information. */
public abstract class EndpointInfo extends com.zeroc.Ice.EndpointInfo {
    /** The address configured with the endpoint. */
    public String addr = "";

    /** The UUID configured with the endpoint. */
    public String uuid = "";
}
