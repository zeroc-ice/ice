//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/** This exception indicates file errors. */
public class FileException extends SyscallException {
  public FileException(String path) {
    this.path = path;
  }

  public FileException(String path, Throwable cause) {
    super(cause);
    this.path = path;
  }

  public String ice_id() {
    return "::Ice::FileException";
  }

  /** The path of the file responsible for the error. */
  public String path;

  private static final long serialVersionUID = 8755315548941623583L;
}
