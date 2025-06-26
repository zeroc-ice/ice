// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * The exception that is thrown when a dispatch failed with an exception that is not a {@link LocalException} or a
 * {@link UserException}.
 */
public class UnknownException extends DispatchException {
    /**
     * Constructs an UnknownException with a message and a cause.
     *
     * @param message the detail message
     * @param cause   the cause
     */
    public UnknownException(String message, Throwable cause) {
        super(ReplyStatus.UnknownException.value(), message, cause);
    }

    /**
     * Constructs an UnknownException with a message.
     *
     * @param message the detail message
     */
    public UnknownException(String message) {
        this(message, null);
    }

    /**
     * Constructs an UnknownException with a cause.
     *
     * @param cause the cause
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
