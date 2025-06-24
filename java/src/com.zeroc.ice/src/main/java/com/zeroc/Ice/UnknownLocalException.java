// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * This exception is raised if an operation call on a server raises a local exception. Because local
 * exceptions are not transmitted by the Ice protocol, the client receives all local exceptions
 * raised by the server as {@link UnknownLocalException}. The only exception to this rule are all
 * exceptions derived from {@link DispatchException}, which are transmitted by the Ice protocol even
 * though they are declared <code>local</code>.
 */
public final class UnknownLocalException extends UnknownException {
    /**
     * Constructs an UnknownLocalException with a message.
     *
     * @param message the detail message for this exception
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
