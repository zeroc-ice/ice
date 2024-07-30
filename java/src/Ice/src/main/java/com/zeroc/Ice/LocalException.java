//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/** Base class for all Ice run-time exceptions. */
public abstract class LocalException extends Exception {
  public LocalException() {}

  public LocalException(String message) {
    super(message);
  }

  public LocalException(Throwable cause) {
    super(cause);
  }

  public LocalException(String message, Throwable cause) {
    super(message, cause);
  }

  private static final long serialVersionUID = 0L;
}
