//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

public interface ResponseHandler
{
    void sendResponse(int requestId, com.zeroc.Ice.OutputStream os, byte status, boolean amd);
    void sendNoResponse();
    boolean systemException(int requestId, com.zeroc.Ice.SystemException ex, boolean amd);
    void invokeException(int requestId, com.zeroc.Ice.LocalException ex, int invokeNum, boolean amd);
}
