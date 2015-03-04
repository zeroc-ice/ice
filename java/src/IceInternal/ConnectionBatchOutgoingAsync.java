// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class ConnectionBatchOutgoingAsync extends BatchOutgoingAsync
{
    public ConnectionBatchOutgoingAsync(Ice.ConnectionI con, Ice.Communicator communicator, Instance instance,
                                        String operation, CallbackBase callback)
    {
        super(communicator, instance, operation, callback);
        _connection = con;
    }

    public void __send()
    {
        int status = _connection.flushAsyncBatchRequests(this);
        if((status & AsyncStatus.Sent) > 0)
        {
            _sentSynchronously = true;
            if((status & AsyncStatus.InvokeSentCallback) > 0)
            {
                __sent();
            }
        }
    }

    @Override
    public Ice.Connection getConnection()
    {
        return _connection;
    }

    private Ice.ConnectionI _connection;
}
