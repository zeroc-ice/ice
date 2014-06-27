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
    int __send(Ice.ConnectionI conection, boolean compress, boolean response)
        throws RetryException;

    int __invokeCollocated(CollocatedRequestHandler handler);

    //
    // Called by the connection when the message is confirmed sent. The connection is locked
    // when this is called so this method can call the sent callback. Instead, this method
    // returns true if there's a sent callback and false otherwise. If true is returned, the
    // connection will call the __invokeSent() method bellow (which in turn should call the 
    // sent callback).
    //
    public abstract boolean __sent();

    //
    // Called by the connection to call the user sent callback.
    //
    public abstract void __invokeSent();
    
    //
    // Called by the connection when the request failed. The boolean indicates whether or
    // not the message was possibly sent (this is useful for retry to figure out whether
    // or not the request can't be retried without breaking at-most-once semantics.)
    //
    public abstract void __finished(Ice.Exception ex, boolean sent);
}
