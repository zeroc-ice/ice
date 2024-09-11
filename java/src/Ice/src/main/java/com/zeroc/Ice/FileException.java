// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception indicates a file error occurred. */
public final class FileException extends SyscallException {
    public FileException(String message) {
        super(message);
    }

    public FileException(String message, Throwable cause) {
        super(message, cause);
    }

    public String ice_id() {
        return "::Ice::FileException";
    }

    private static final long serialVersionUID = 8755315548941623583L;
}
