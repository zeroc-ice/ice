// Copyright (c) ZeroC, Inc.

using System.Net.Sockets;

namespace Ice.Internal;

public interface Transceiver
{
    Socket fd();

    int initialize(Buffer readBuffer, Buffer writeBuffer, ref bool hasMoreData);

    int closing(bool initiator, Ice.LocalException ex);

    void close();

    void destroy();

    EndpointI bind();

    int write(Buffer buf);

    int read(Buffer buf, ref bool hasMoreData);

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
    bool startRead(Buffer buf, AsyncCallback callback, object state);

    void finishRead(Buffer buf);

    //
    // Write data asynchronously.
    //
    // The I/O request may complete synchronously, in which case finishWrite
    // will be invoked in the same thread as startWrite. The request
    // will be canceled upon the termination of the thread that calls startWrite.
    //
    bool startWrite(Buffer buf, AsyncCallback callback, object state, out bool completed);

    void finishWrite(Buffer buf);

    string protocol();

    string toDetailedString();

    /// <summary>Creates a connection info object for this connection.</summary>
    /// <param name="incoming"><see langword="true"/> for an incoming connection, <see langword="true"/> for an outgoing
    /// connection.</param>
    /// <param name="adapterName">The name of the object adapter currently associated with this connection.</param>
    /// <param name="connectionId">The connection ID of this connection.</param>
    /// <returns>The new connection info.</returns>
    ConnectionInfo getInfo(bool incoming, string adapterName, string connectionId);

    void checkSendSize(Buffer buf);

    void setBufferSize(int rcvSize, int sndSize);
}
