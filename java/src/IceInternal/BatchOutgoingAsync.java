// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class BatchOutgoingAsync extends Ice.AsyncResult implements OutgoingAsyncMessageCallback, TimerTask
{
    public BatchOutgoingAsync(Ice.Communicator communicator, Instance instance, String operation, CallbackBase callback)
    {
        super(communicator, instance, operation, callback);
    }

    public int
    __send(Ice.ConnectionI connection, boolean compress, boolean response)
    {
        _cachedConnection = connection;
        return connection.flushAsyncBatchRequests(this);
    }

    public int
    __invokeCollocated(CollocatedRequestHandler handler)
    {
        return handler.invokeAsyncBatchRequests(this);
    }

    public boolean 
    __sent()
    {
        synchronized(_monitor)
        {
            _state |= Done | OK | Sent;
            //_os.resize(0, false); // Don't clear the buffer now, it's needed for the collocation optimization
            if(_childObserver != null)
            {
                _childObserver.detach();
                _childObserver = null;
            }
            if(_timeoutRequestHandler != null)
            {
                _instance.timer().cancel(this);
                _timeoutRequestHandler = null;
            }
            _monitor.notifyAll();
            return true;
        }
    }

    public void 
    __invokeSent()
    {
        __invokeSentInternal();
    }
    
    public void 
    __finished(Ice.Exception exc, boolean sent)
    {
        if(_childObserver != null)
        {
            _childObserver.failed(exc.ice_name());
            _childObserver.detach();
            _childObserver = null;
        }
        if(_timeoutRequestHandler != null)
        {
            _instance.timer().cancel(this);
            _timeoutRequestHandler = null;
        }
        __invokeException(exc);
    }

    public void 
    runTimerTask()
    {
        __runTimerTask();
    }

}
