//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

public interface Transceiver {
  java.nio.channels.SelectableChannel fd();

  void setReadyCallback(ReadyCallback callback);

  int initialize(Buffer readBuffer, Buffer writeBuffer);

  int closing(boolean initiator, com.zeroc.Ice.LocalException ex);

  void close();

  EndpointI bind();

  int write(Buffer buf);

  int read(Buffer buf);

  /**
   * Checks if this transceiver is waiting to be read, typically because it has bytes readily
   * available for reading.
   *
   * @return true if this transceiver is waiting to be read, false otherwise.
   * @remark The caller must ensure the transceiver is not closed when calling this method.
   */
  boolean isWaitingToBeRead();

  String protocol();

  @Override
  String toString();

  String toDetailedString();

  com.zeroc.Ice.ConnectionInfo getInfo();

  void checkSendSize(Buffer buf);

  void setBufferSize(int rcvSize, int sndSize);
}
