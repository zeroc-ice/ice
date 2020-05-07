//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;

using System;
using System.Collections.Generic;
using System.Net.Sockets;

namespace IceInternal
{
    public interface ITransceiver
    {
        Socket? Fd();

        /// <summary>
        /// Initialize the transport, this method returns SocketOperation.None once the transport has been initialized,
        /// if the initialization needs to write data it must write the data to the write buffer and return
        /// SocketOperation.Write, likewise if the initialization needs to read data it must resize the read buffer to
        /// the appropriate size and returns SocketOperation.Read.
        /// </summary>
        /// <param name="readBuffer">An empty buffer used for read operations</param>
        /// <param name="writeBuffer">An empty buffer used for write operations</param>
        /// <returns>Returns SocketOperation.Write, SocketOperation.Read or SocketOperation.None indicating
        /// whenever the operation needs to write more data, read more data or it is done.</returns>
        int Initialize(ref ArraySegment<byte> readBuffer, IList<ArraySegment<byte>> writeBuffer);
        int Closing(bool initiator, System.Exception? ex);
        void Close();
        void Destroy();

        Endpoint Bind();

        /// <summary>Write the buffer data to the socket starting at the given offset,
        /// the offset is incremented with the number of bytes written, returns
        /// SocketOperation.None if all the data was wrote otherwise returns
        /// SocketOperation.Write.</summary>
        /// <param name="buffer">The data to write to the socket as a list of byte array segments.</param>
        /// <param name="offset">The zero based byte offset into the buffer. The offset is increase by
        /// the amount of bytes written.</param>
        /// <returns>A constant indicating if all data was wrote to the socket, SocketOperation.None
        /// indicate there is no more data to write, SocketOperation.Write indicates there is still
        /// data to write in the buffer.</returns>
        int Write(IList<ArraySegment<byte>> buffer, ref int offset);
        int Read(ref ArraySegment<byte> buffer, ref int offset);

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
        bool StartRead(ref ArraySegment<byte> buffer, ref int offset, AsyncCallback callback, object state);
        void FinishRead(ref ArraySegment<byte> buffer, ref int offset);

        //
        // Write data asynchronously.
        //
        // The I/O request may complete synchronously, in which case finishWrite
        // will be invoked in the same thread as startWrite. The request
        // will be canceled upon the termination of the thread that calls startWrite.
        //

        /// <summary>Starts an asynchronous write operation of the buffer data to the transport
        /// starting at the given offset, completed is set to true if the write operation
        /// account for the remaining of the buffer data or false otherwise, returns whenever
        /// the asynchronous operation completed synchronously or not.</summary>
        /// <param name="buffer">The data to write to the socket as a list of byte array segments.</param>
        /// <param name="offset">The zero based byte offset into the buffer at what start writing.</param>
        /// <param name="callback">The asynchronous completion callback.</param>
        /// <param name="state">A state object that is associated with the asynchronous operation.</param>
        /// <param name="completed">True if the write operation accounts for the buffer remaining data, from
        /// offset to the end of the buffer.</param>
        /// <returns>True if the asynchronous operation completed synchronously otherwise false.</returns>
        bool StartWrite(IList<ArraySegment<byte>> buffer, int offset, AsyncCallback callback, object state, out bool completed);

        /// <summary>Finish an asynchronous write operation, the offset is increase with the
        /// number of bytes wrote to the transport.</summary>
        /// <param name="buffer">The buffer of data to write to the socket.</param>
        /// <param name="offset">The offset at what the write operation starts, the offset is increase
        /// with the number of bytes successfully wrote to the socket.</param>
        void FinishWrite(IList<ArraySegment<byte>> buffer, ref int offset);

        string Transport();
        string ToDetailedString();
        Ice.ConnectionInfo GetInfo();
        void CheckSendSize(int size);
        void SetBufferSize(int rcvSize, int sndSize);
    }
}
