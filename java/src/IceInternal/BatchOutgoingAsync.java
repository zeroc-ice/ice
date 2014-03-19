// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class BatchOutgoingAsync extends Ice.AsyncResult implements OutgoingAsyncMessageCallback
{
    public BatchOutgoingAsync(Ice.Communicator communicator, Instance instance, String operation, CallbackBase callback)
    {
        super(communicator, instance, operation, callback);
    }

    public boolean __sent(Ice.ConnectionI connection)
    {
        synchronized(_monitor)
        {
            _state |= Done | OK | Sent;
            _os.resize(0, false); // Clear buffer now, instead of waiting for AsyncResult deallocation
            if(_remoteObserver != null)
            {
                _remoteObserver.detach();
                _remoteObserver = null;
            }
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
        if(_remoteObserver != null)
        {
            _remoteObserver.failed(exc.ice_name());
            _remoteObserver.detach();
            _remoteObserver = null;
        }
        __exception(exc);
    }
}
