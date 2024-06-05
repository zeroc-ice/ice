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

/** This exception indicates a request was interrupted. */
public class OperationInterruptedException extends LocalException {
  public OperationInterruptedException() {}

  public OperationInterruptedException(Throwable cause) {
    super(cause);
  }

  public String ice_id() {
    return "::Ice::OperationInterruptedException";
  }

  /**
   * @hidden
   */
  public static final long serialVersionUID = -1099536335133286580L;
}