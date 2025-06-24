// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception is raised if a system error occurred in the server or client process. */
public class SyscallException extends LocalException {
    /**
     * Constructs a SyscallException with a {@code null} detail message and the specified cause.
     *
     * @param cause the cause
     */
    public SyscallException(Throwable cause) {
        super(null, cause);
    }

    /**
     * Constructs a SyscallException with the specified detail message.
     *
     * @param message the detail message
     */
    protected SyscallException(String message) {
        super(message);
    }

    /**
     * Constructs a SyscallException with the specified detail message and cause.
     *
     * @param message the detail message
     * @param cause   the cause
     */
    protected SyscallException(String message, Throwable cause) {
        super(message, cause);
    }

    @Override
    public String ice_id() {
        return "::Ice::SyscallException";
    }

    private static final long serialVersionUID = -2440066513892919497L;
}
