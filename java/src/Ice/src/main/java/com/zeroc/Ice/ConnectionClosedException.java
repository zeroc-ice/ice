// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception indicates that a connection was closed gracefully. */
public class ConnectionClosedException extends LocalException {
  public String ice_id() {
    return "::Ice::ConnectionClosedException";
  }

  private static final long serialVersionUID = 0L;
}
