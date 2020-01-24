//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Net.Sockets;

namespace IceInternal
{
    public interface ITransceiver
    {
        Socket? Fd();
        int Initialize(Buffer readBuffer, Buffer writeBuffer, ref bool hasMoreData);
        int Closing(bool initiator, Ice.LocalException? ex);
        void Close();
        void Destroy();

        Endpoint Bind();
        int Write(Buffer buf);
        int Read(Buffer buf, ref bool hasMoreData);

        //
        // Read data asynchronously.
        //
        // The I/O request may complete synchronously, in which case finishRead
        // will be invoked in the same thread as startRead. The caller must check
        // the buffer after finishRead completes to determine whether all of the
        // requested data has been read.
        //
        // The read request is canceled upon the termination of the thread that
        // calls startRead, or when the socket is closed. In this case finishRead
        // raises ReadAbortedException.
        //
        bool StartRead(Buffer buf, AsyncCallback callback, object state);
        void FinishRead(Buffer buf);

        //
        // Write data asynchronously.
        //
        // The I/O request may complete synchronously, in which case finishWrite
        // will be invoked in the same thread as startWrite. The request
        // will be canceled upon the termination of the thread that calls startWrite.
        //
        bool StartWrite(Buffer buf, AsyncCallback callback, object state, out bool completed);
        void FinishWrite(Buffer buf);

        string Protocol();
        string ToDetailedString();
        Ice.ConnectionInfo GetInfo();
        void CheckSendSize(Buffer buf);
        void SetBufferSize(int rcvSize, int sndSize);
    }

}
