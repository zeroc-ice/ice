//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * Base class for Ice local exceptions. Those exceptions are not checked so we inherit from
 * java.lang.RuntimeException. User exceptions are checked exceptions and therefore inherit directly
 * from java.lang.Exception.
 */
public abstract class Exception extends RuntimeException implements Cloneable {
  public Exception(String message) {
    super(message);
  }

  public Exception(String message, Throwable cause) {
    super(message, cause);
  }

  /**
   * Creates a copy of this exception.
   *
   * @return The copy of this exception.
   */
  @Override
  public Exception clone() {
    Exception c = null;

    try {
      c = (Exception) super.clone();
    } catch (CloneNotSupportedException ex) {
      assert false;
    }
    return c;
  }

  /**
   * Returns the type ID of this exception.
   *
   * @return The type ID of this exception.
   */
  public abstract String ice_id();

  private static final long serialVersionUID = 0L;
}
