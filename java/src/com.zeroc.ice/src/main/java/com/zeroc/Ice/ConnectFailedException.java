// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception indicates connection failures. */
public class ConnectFailedException extends SocketException {
    public ConnectFailedException() {
        super();
    }

    public ConnectFailedException(Throwable cause) {
        super(cause);
    }

    public String ice_id() {
        return "::Ice::ConnectFailedException";
    }

    private static final long serialVersionUID = 3367444083227777090L;
}
