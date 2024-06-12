// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

public enum ReplyStatus {
  Ok(0),
  UserException(1),
  ObjectNotExist(2),
  FacetNotExist(3),
  OperationNotExist(4),
  UnknownLocalException(5),
  UnknownUserException(6),
  UnknownException(7);

  private final byte value;

  private ReplyStatus(int value) {
    this.value = (byte) value;
  }

  public byte getValue() {
    return value;
  }
}
