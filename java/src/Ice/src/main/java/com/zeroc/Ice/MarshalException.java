// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception is raised for errors during marshaling or unmarshaling. */
public final class MarshalException extends ProtocolException {
  public MarshalException(String reason) {
    super(reason);
  }

  public MarshalException(String reason, Throwable cause) {
    super(reason, cause);
  }

  public String ice_id() {
    return "::Ice::MarshalException";
  }

  private static final long serialVersionUID = -1332260000897066889L;
}
