// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * A datagram exceeds the configured size. This exception is raised if a datagram exceeds the
 * configured send or receive buffer size, or exceeds the maximum payload size of a UDP packet
 * (65507 bytes).
 */
public final class DatagramLimitException extends ProtocolException {
  public DatagramLimitException() {
    this("");
  }

  public DatagramLimitException(String reason) {
    super("Datagram limit exceed: " + reason);
  }

  public String ice_id() {
    return "::Ice::DatagramLimitException";
  }

  private static final long serialVersionUID = -783492847222783613L;
}
