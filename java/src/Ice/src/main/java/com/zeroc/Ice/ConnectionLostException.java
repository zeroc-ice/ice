//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/** This exception indicates a lost connection. */
public class ConnectionLostException extends SocketException {
  public ConnectionLostException() {
    super();
  }

  public ConnectionLostException(Throwable cause) {
    super(cause);
  }

  public String ice_id() {
    return "::Ice::ConnectionLostException";
  }

  private static final long serialVersionUID = -7871526265585148499L;
}
