// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * This exception is raised if an operation call on a server raises an unknown exception, that is,
 * any exception which does not derive from {@link LocalException} or {@link UserException}.
 */
public class UnknownException extends DispatchException {
    public UnknownException(String message, Throwable cause) {
        super(ReplyStatus.UnknownException.value(), message, cause);
    }

    public UnknownException(String message) {
        this(message, null);
    }

    public UnknownException(Throwable cause) {
        this(null, cause);
    }

    @Override
    public String ice_id() {
        return "::Ice::UnknownException";
    }

    // Logically protected too.
    UnknownException(ReplyStatus replyStatus, String message) {
        super(replyStatus.value(), message);
    }

    private static final long serialVersionUID = 4845487294380422868L;
}
