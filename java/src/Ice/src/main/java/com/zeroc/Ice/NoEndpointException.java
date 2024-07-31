//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/** This exception is raised if no suitable endpoint is available. */
public class NoEndpointException extends LocalException {
  public NoEndpointException(String proxy) {
    this.proxy = proxy;
  }

  public String ice_id() {
    return "::Ice::NoEndpointException";
  }

  /** The stringified proxy for which no suitable endpoint is available. */
  public String proxy;

  private static final long serialVersionUID = -5026638954785808518L;
}
