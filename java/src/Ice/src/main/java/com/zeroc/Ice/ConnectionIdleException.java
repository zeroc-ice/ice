// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception indicates that a connection was aborted by the idle check. */
public class ConnectionIdleException extends LocalException {
  public ConnectionIdleException() {
    super();
  }

  public ConnectionIdleException(Throwable cause) {
    super(cause);
  }

  public String ice_id() {
    return "::Ice::ConnectIdleException";
  }

  private static final long serialVersionUID = -1271371420507272518L;
}
