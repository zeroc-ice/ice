// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

public interface ResponseHandler
{
    void sendResponse(int requestId, com.zeroc.Ice.OutputStream os, byte status, boolean amd);
    void sendNoResponse();
    boolean systemException(int requestId, com.zeroc.Ice.SystemException ex, boolean amd);
    void invokeException(int requestId, com.zeroc.Ice.LocalException ex, int invokeNum, boolean amd);
}
