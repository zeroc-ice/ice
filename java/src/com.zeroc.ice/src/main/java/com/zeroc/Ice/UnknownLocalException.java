// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * The exception that is thrown when a dispatch failed with a {@link LocalException} that is not a DispatchException.
 */
public final class UnknownLocalException extends UnknownException {
    /**
     * Constructs an UnknownLocalException with a message.
     *
     * @param message the detail message
     */
    public UnknownLocalException(String message) {
        super(ReplyStatus.UnknownLocalException, message);
    }

    @Override
    public String ice_id() {
        return "::Ice::UnknownLocalException";
    }

    private static final long serialVersionUID = 1374449481624773050L;
}
