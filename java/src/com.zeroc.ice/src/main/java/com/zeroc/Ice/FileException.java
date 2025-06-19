// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception indicates a file error occurred. */
public final class FileException extends SyscallException {
    /**
     * Constructs a {@code FileException} with a message.
     *
     * @param message the detail message for this exception
     */
    public FileException(String message) {
        super(message);
    }

    /**
     * Constructs a {@code FileException} with a message and a cause.
     *
     * @param message the detail message for this exception
     * @param cause   the cause of this exception
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
