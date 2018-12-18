// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

using System.Collections.Generic;
using Ice.Instrumentation;

namespace IceInternal
{
    public interface ResponseHandler
    {
        void sendResponse(int requestId, Ice.OutputStream os, byte status, bool amd);
        void sendNoResponse();
        bool systemException(int requestId, Ice.SystemException ex, bool amd);
        void invokeException(int requestId, Ice.LocalException ex, int invokeNum, bool amd);
    }
}
