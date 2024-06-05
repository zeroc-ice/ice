//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
//
// Ice version 3.7.10
//
// <auto-generated>
//
// Generated from file `LocalException.ice'
//
// Warning: do not edit this file.
//
// </auto-generated>
//

package com.zeroc.Ice;

/** This exception indicates that an invocation failed because it timed out. */
public class InvocationTimeoutException extends TimeoutException {
  public InvocationTimeoutException() {
    super();
  }

  public InvocationTimeoutException(Throwable cause) {
    super(cause);
  }

  public String ice_id() {
    return "::Ice::InvocationTimeoutException";
  }

  /**
   * @hidden
   */
  public static final long serialVersionUID = -4956443780705036860L;
}