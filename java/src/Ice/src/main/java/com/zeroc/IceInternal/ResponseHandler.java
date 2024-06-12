//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

public interface ResponseHandler {
  void sendResponse(int requestId, com.zeroc.Ice.OutputStream os, byte status, boolean amd);

  void sendNoResponse();

  void dispatchException(
      int requestId, com.zeroc.Ice.LocalException ex, int requestCount, boolean amd);
}
