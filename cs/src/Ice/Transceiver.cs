// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    using System;
    using System.Net.Sockets;

    public interface Transceiver
    {
        //
        // Initialize the transceiver using asynchronous I/O. This method never blocks. Returns true
        // if initialization is complete, or false if an I/O request is pending. In the latter case,
        // the callback must invoke initialize again and repeat this process until it returns true.
        //
        int initialize();

        void close();
        void destroy();

        //
        // Write data.
        //
        // Returns true if all the data was written, false otherwise.
        //
        bool write(Buffer buf);

        //
        // Read data.
        //
        // Returns true if all the requested data was read, false otherwise.
        //
        bool read(Buffer buf);

        //
        // Read data asynchronously.
        //
        // The I/O request may complete synchronously, in which case endRead
        // will be invoked in the same thread as beginRead. The return value
        // from beginRead must be passed to endRead, along with the same buffer
        // object. The caller must check the buffer after endRead completes to
        // determine whether all of the requested data has been read.
        //
        // The read request is canceled upon the termination of the thread that
        // calls beginRead, or when the socket is closed. In this case endRead
        // raises ReadAbortedException.
        //
        bool startRead(Buffer buf, AsyncCallback callback, object state);
        void finishRead(Buffer buf);

        //
        // Write data asynchronously.
        //
        // The I/O request may complete synchronously, in which case endWrite
        // will be invoked in the same thread as beginWrite. The request
        // will be canceled upon the termination of the thread that calls beginWrite.
        //
        bool startWrite(Buffer buf, AsyncCallback callback, object state, out bool completed);
        void finishWrite(Buffer buf);

        string type();
        Ice.ConnectionInfo getInfo();
        void checkSendSize(Buffer buf, int messageSizeMax);
    }

}
