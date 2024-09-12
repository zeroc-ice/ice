//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice.SSL;

/** Provides access to an SSL endpoint information. */
public abstract class EndpointInfo extends com.zeroc.Ice.EndpointInfo {
    public EndpointInfo() {
        super();
    }

    public EndpointInfo(com.zeroc.Ice.EndpointInfo underlying, int timeout, boolean compress) {
        super(underlying, timeout, compress);
    }

    public EndpointInfo clone() {
        return (EndpointInfo) super.clone();
    }
}
