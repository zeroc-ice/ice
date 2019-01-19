//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

public interface ResponseHandler
{
    void sendResponse(int requestId, Ice.OutputStream os, byte status, boolean amd);
    void sendNoResponse();
    boolean systemException(int requestId, Ice.SystemException ex, boolean amd);
    void invokeException(int requestId, Ice.LocalException ex, int invokeNum, boolean amd);
}
