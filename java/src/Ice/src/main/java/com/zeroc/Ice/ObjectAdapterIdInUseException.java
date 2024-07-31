//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception is raised if an {@link ObjectAdapter} cannot be activated. This happens if the
 * {@link Locator} detects another active {@link ObjectAdapter} with the same adapter id.
 */
public class ObjectAdapterIdInUseException extends LocalException {
  public ObjectAdapterIdInUseException(String id) {
    this.id = id;
  }

  public String ice_id() {
    return "::Ice::ObjectAdapterIdInUseException";
  }

  /** Adapter ID. */
  public String id;

  private static final long serialVersionUID = 609699148378869554L;
}
