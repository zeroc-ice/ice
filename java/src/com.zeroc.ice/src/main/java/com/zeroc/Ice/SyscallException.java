// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** The exception that is thrown to report the failure of a system call. */
public class SyscallException extends LocalException {
    /**
     * Constructs a SyscallException with a cause.
     *
     * @param cause the cause
     */
    public SyscallException(Throwable cause) {
        super(null, cause);
    }

    /**
     * Constructs a SyscallException with a message.
     *
     * @param message the detail message
     */
    protected SyscallException(String message) {
        super(message);
    }

    /**
     * Constructs a SyscallException with a message and a cause.
     *
     * @param message the detail message
     * @param cause the cause
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
