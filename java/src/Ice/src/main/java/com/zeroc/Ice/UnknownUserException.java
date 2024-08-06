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
    super(message);
  }

  public UnknownUserException(String message, Throwable cause) {
    super(message, cause);
  }

  public String ice_id() {
    return "::Ice::UnknownUserException";
  }

  private static final long serialVersionUID = -6046568406824082586L;
}
