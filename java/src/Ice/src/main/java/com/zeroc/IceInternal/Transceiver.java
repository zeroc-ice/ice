//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

public interface Transceiver {
  /**
   * Returns the selectable channel used by the thread pool's selector to wait for read or
   * write readiness.
   *
   * @return The selectable channel that will be registered with the thread pool's selector
   * or null if the transceiver doesn't use a selectable channel.
   */
  java.nio.channels.SelectableChannel fd();

  /**
   * Sets the transceiver ready callback. This method is called by the thread pool to provide
   * a ready callback object that the transceiver can use to notify the thread pool's
   * selector that more data is ready to be read or written by this transceiver. The
   * transceiver implementation typically uses this mechanism when it buffers data read from
   * the selectable channel define above or when there's no the transceiver doesn't provide
   * a selectable channel (the fd() method above returns null).
   *
   * @param callback The ready callback provided by the thread pool's selector.
   */
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
