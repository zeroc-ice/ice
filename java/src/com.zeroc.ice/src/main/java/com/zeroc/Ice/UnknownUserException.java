// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * The dispatch returned a {@link UserException} that was not declared in the operation's exception
 * specification (the <code>throws</code> clause). This is necessary in order to not violate the
 * contract established by an operation's signature: Only local exceptions and user exceptions
 * declared in the <code>throws</code> clause can be raised.
 */
public final class UnknownUserException extends UnknownException {
    public UnknownUserException(String message) {
        super(ReplyStatus.UnknownUserException, message);
    }

    public String ice_id() {
        return "::Ice::UnknownUserException";
    }

    public static UnknownUserException fromTypeId(String typeId) {
        return new UnknownUserException(
                "The reply carries a user exception that does not conform to the operation's exception specification: " +
                        typeId);
    }

    private static final long serialVersionUID = -6046568406824082586L;
}
