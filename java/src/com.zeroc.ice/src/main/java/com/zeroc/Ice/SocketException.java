// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception indicates socket errors. */
public class SocketException extends SyscallException {
    /**
     * Constructs a SocketException with {@code null} as its detail message.
     */
    public SocketException() {
        super((String) null);
    }

     /**
     * Constructs a {@code SocketException} with the specified cause.
     *
     * @param cause the cause of this exception
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
