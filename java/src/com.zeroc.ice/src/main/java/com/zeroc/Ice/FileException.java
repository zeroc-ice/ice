// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception indicates a file error occurred. */
public final class FileException extends SyscallException {
    /**
     * Constructs a FileException with a message.
     *
     * @param message the detail message
     */
    public FileException(String message) {
        super(message);
    }

    /**
     * Constructs a FileException with a message and a cause.
     *
     * @param message the detail message
     * @param cause   the cause
     */
    public FileException(String message, Throwable cause) {
        super(message, cause);
    }

    @Override
    public String ice_id() {
        return "::Ice::FileException";
    }

    private static final long serialVersionUID = 8755315548941623583L;
}
