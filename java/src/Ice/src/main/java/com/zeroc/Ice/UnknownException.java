// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * This exception is raised if an operation call on a server raises an unknown exception, that is,
 * any exception which does not derive from {@link LocalException} or {@link UserException}.
 */
public class UnknownException extends LocalException {
  public UnknownException(Throwable cause) {
    super(cause);
    this.unknown = "";
  }

  public UnknownException(String message) {
    super(message);
    this.unknown = "";
  }

  public UnknownException(String unknown, Throwable cause) {
    super("unknown exception with type ID '" + unknown + "'", cause);
    this.unknown = unknown;
  }

  public String ice_id() {
    return "::Ice::UnknownException";
  }

  /** This field is set to the textual representation of the unknown exception if available. */
  public final String unknown;

  private static final long serialVersionUID = 4845487294380422868L;
}
