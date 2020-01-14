//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{
    public interface IResponseHandler
    {
        void SendResponse(int requestId, Ice.OutputStream os, byte status, bool amd);
        void SendNoResponse();
        bool SystemException(int requestId, Ice.SystemException ex, bool amd);
        void InvokeException(int requestId, Ice.LocalException ex, int invokeNum, bool amd);
    }
}
