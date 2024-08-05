// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception is raised if a system error occurred in the server or client process. */
public class SyscallException extends LocalException {
  public SyscallException() {
    super();
  }

  public SyscallException(Throwable cause) {
    super(cause);
  }

  public SyscallException(String message) {
    super(message);
  }

  public SyscallException(String message, Throwable cause) {
    super(message, cause);
  }

  public String ice_id() {
    return "::Ice::SyscallException";
  }

  private static final long serialVersionUID = -2440066513892919497L;
}
