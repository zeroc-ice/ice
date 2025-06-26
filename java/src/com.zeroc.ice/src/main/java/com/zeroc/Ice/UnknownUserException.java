// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * The exception that is thrown when a client receives a {@link UserException} that was not declared in the 
 * operation's exception specification.
 */
public final class UnknownUserException extends UnknownException {
    /**
     * Constructs an UnknownUserException with a message.
     *
     * @param message the detail message
     */
    public UnknownUserException(String message) {
        super(ReplyStatus.UnknownUserException, message);
    }

    @Override
    public String ice_id() {
        return "::Ice::UnknownUserException";
    }

    /**
     * Creates an UnknownUserException from a type ID.
     *
     * @param typeId the type ID of the user exception
     * @return an UnknownUserException
     */
    public static UnknownUserException fromTypeId(String typeId) {
        return new UnknownUserException(
            "The reply carries a user exception that does not conform to the operation's exception specification: "
                + typeId);
    }

    private static final long serialVersionUID = -6046568406824082586L;
}
