// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

//
// This interface is used by the connection to handle OutgoingAsync
// and BatchOutgoingAsync messages.
//
public interface OutgoingAsyncMessageCallback
{
    //
    // Called by the request handler to send the request over the connection.
    //
    int send(Ice.ConnectionI conection, boolean compress, boolean response)
        throws RetryException;

    //
    // Called by the collocated request handler to invoke the request.
    //
    int invokeCollocated(CollocatedRequestHandler handler);

    //
    // Called by the connection when the message is confirmed sent. The
    // connection is locked when this is called so this method can't call the
    // sent callback. Instead, this method returns true if there's a sent
    // callback and false otherwise. If true is returned, the connection will
    // call the __invokeSent() method bellow (which in turn should call the sent
    // callback).
    //
    boolean sent();

    //
    // Called by the connection to call the user sent callback.
    //
    void invokeSent();

    //
    // Called by the connection when the request failed.
    //
    void finished(Ice.Exception ex);

    //
    // Helper to dispatch the cancellation exception.
    //
    void dispatchInvocationCancel(Ice.LocalException ex, ThreadPool threadPool, Ice.Connection connection);
}
