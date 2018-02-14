// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections.Generic;
using Ice.Instrumentation;

namespace IceInternal
{
    public interface ResponseHandler
    {
        void sendResponse(int requestId, BasicStream os, byte status, bool amd);
        void sendNoResponse();
        bool systemException(int requestId, Ice.SystemException ex, bool amd);
        void invokeException(int requestId, Ice.LocalException ex, int invokeNum, bool amd);
    };
}
