// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * This exception is raised if an operation call on a server raises an unknown exception, that is,
 * any exception which does not derive from {@link LocalException} or {@link UserException}.
 */
public class UnknownException extends LocalException {
    public UnknownException(Throwable cause) {
        super(null, cause);
    }

    public UnknownException(String message) {
        super(message);
    }

    public UnknownException(String message, Throwable cause) {
        super(message, cause);
    }

    public String ice_id() {
        return "::Ice::UnknownException";
    }

    private static final long serialVersionUID = 4845487294380422868L;
}
