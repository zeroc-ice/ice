// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** The exception that is thrown to report a socket error. */
public class SocketException extends SyscallException {
    /**
     * Constructs a SocketException.
     */
    public SocketException() {
        super((String) null);
    }

    /**
     * Constructs a SocketException with a cause.
     *
     * @param cause the cause
     */
    public SocketException(Throwable cause) {
        super(cause);
    }

    @Override
    public String ice_id() {
        return "::Ice::SocketException";
    }

    private static final long serialVersionUID = -7634050967564791782L;
}
