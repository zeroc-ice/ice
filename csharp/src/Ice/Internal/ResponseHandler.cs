// Copyright (c) ZeroC, Inc.

namespace Ice.Internal;

public interface ResponseHandler
{
    void sendResponse(int requestId, Ice.OutputStream os, byte status, bool amd);
    void sendNoResponse();
    void invokeException(int requestId, Ice.LocalException ex, int invokeNum, bool amd);
}
