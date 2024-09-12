// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception indicates socket errors. */
public class SocketException extends SyscallException {
    public SocketException() {
        super((String) null);
    }

    public SocketException(Throwable cause) {
        super(cause);
    }

    public String ice_id() {
        return "::Ice::SocketException";
    }

    private static final long serialVersionUID = -7634050967564791782L;
}
