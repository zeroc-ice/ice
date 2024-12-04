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

    /// <summary>Writes data asynchronously. The I/O request may complete synchronously, in which case
    /// <see cref="finishWrite" /> will be invoked in the same thread as <see cref="startWrite" />. The operation will
    /// be canceled upon the termination of the thread that calls <see cref="startWrite" />.</summary>
    /// <param name="buf">The buffer to write.</param>
    /// <param name="callback">The callback that this operation calls when it completes asynchronously.</param>
    /// <param name="state">The state to pass to the callback.</param>
    /// <param name="messageFullyWritten">When <see langword="true" />, the operation has written or is the process of
    /// writing the remainder of the message; otherwise, <see langword="false"/>.</param>
    /// <returns><see langword="true"/> when no I/O is pending, or <see langword="false" /> when I/O is pending. The
    /// provided callback is called only when I/O is pending, upon asynchronous completion of this operation.</returns>
    bool startWrite(Buffer buf, AsyncCallback callback, object state, out bool messageFullyWritten);

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
