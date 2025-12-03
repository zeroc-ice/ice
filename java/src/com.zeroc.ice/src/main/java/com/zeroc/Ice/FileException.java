// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** The exception that is thrown to report a file system error. */
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
     * @param cause the cause
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
