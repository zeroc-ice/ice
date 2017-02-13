// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public interface ResponseHandler
{
    void sendResponse(int requestId, BasicStream os, byte status, boolean amd);
    void sendNoResponse();
    boolean systemException(int requestId, Ice.SystemException ex, boolean amd);
    void invokeException(int requestId, Ice.LocalException ex, int invokeNum, boolean amd);
}
