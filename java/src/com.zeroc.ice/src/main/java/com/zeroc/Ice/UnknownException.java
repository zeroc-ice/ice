// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * This exception is raised if an operation call on a server raises an unknown exception, that is,
 * any exception which does not derive from {@link LocalException} or {@link UserException}.
 */
public class UnknownException extends DispatchException {
    /**
     * Constructs an UnknownException with a message and a cause.
     *
     * @param message the detail message for this exception
     * @param cause   the cause of this exception
     */
    public UnknownException(String message, Throwable cause) {
        super(ReplyStatus.UnknownException.value(), message, cause);
    }

    /**
     * Constructs an UnknownException with a message.
     *
     * @param message the detail message for this exception
     */
    public UnknownException(String message) {
        this(message, null);
    }

    /**
     * Constructs an UnknownException with a cause.
     *
     * @param cause the cause of this exception
     */
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
