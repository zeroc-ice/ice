// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class BatchOutgoingAsync extends Ice.AsyncResult implements OutgoingAsyncMessageCallback
{
    public BatchOutgoingAsync(Instance instance, String operation, CallbackBase callback)
    {
        super(instance, operation, callback);
    }

    public boolean __sent(Ice.ConnectionI connection)
    {
        synchronized(_monitor)
        {
            _state |= Done | OK | Sent;
            _monitor.notifyAll();
            return true;
        }
    }

    public void __sent()
    {
        __sentInternal();
    }
    
    public void __finished(Ice.LocalException exc, boolean sent)
    {
        __exception(exc);
    }
}
