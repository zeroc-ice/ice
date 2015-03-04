// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class CommunicatorBatchOutgoingAsync extends Ice.AsyncResult
{
    public CommunicatorBatchOutgoingAsync(Ice.Communicator communicator, Instance instance, String operation,
                                          CallbackBase callback)
    {
        super(communicator, instance, operation, callback);

        //
        // _useCount is initialized to 1 to prevent premature callbacks.
        // The caller must invoke ready() after all flush requests have
        // been initiated.
        //
        _useCount = 1;

        //
        // Assume all connections are flushed synchronously.
        //
        _sentSynchronously = true;

        //
        // Attach observer
        //
        _observer = ObserverHelper.get(instance, operation);
    }

    public void flushConnection(Ice.ConnectionI con)
    {
        class BatchOutgoingAsyncI extends BatchOutgoingAsync
        {
            public 
            BatchOutgoingAsyncI()
            {
                super(CommunicatorBatchOutgoingAsync.this._communicator, 
                      CommunicatorBatchOutgoingAsync.this._instance, 
                      CommunicatorBatchOutgoingAsync.this._operation, 
                      null);
            }

            public boolean 
            __sent(Ice.ConnectionI con)
            {
                if(_remoteObserver != null)
                {
                    _remoteObserver.detach();
                    _remoteObserver = null;
                }
                check(false);
                return false;
            }

            public void 
            __finished(Ice.LocalException ex, boolean sent)
            {
                if(_remoteObserver != null)
                {
                    _remoteObserver.failed(ex.ice_name());
                    _remoteObserver.detach();
                    _remoteObserver = null;
                }
                check(false);
            }

            public void
            __attachRemoteObserver(Ice.ConnectionInfo info, Ice.Endpoint endpt, int requestId, int size)
            {
                if(CommunicatorBatchOutgoingAsync.this._observer != null)
                {
                    _remoteObserver = CommunicatorBatchOutgoingAsync.this._observer.getRemoteObserver(info, endpt,
                                                                                                      requestId, size);
                    if(_remoteObserver != null)
                    {
                        _remoteObserver.attach();
                    }
                }
            }
        };

        synchronized(_monitor)
        {
            ++_useCount;
        }
        
        try
        {
            int status = con.flushAsyncBatchRequests(new BatchOutgoingAsyncI());
            if((status & AsyncStatus.Sent) > 0)
            {
                _sentSynchronously = false;
            }
        }
        catch(Ice.LocalException ex)
        {
            check(false);
            throw ex;
        }
    }

    public void ready()
    {
        check(true);
    }

    private void check(boolean userThread)
    {
        synchronized(_monitor)
        {
            assert(_useCount > 0);
            if(--_useCount > 0)
            {
                return;
            }
            _state |= Done | OK | Sent;
            _os.resize(0, false); // Clear buffer now, instead of waiting for AsyncResult deallocation
            _monitor.notifyAll();
        }

        //
        // sentSynchronously_ is immutable here.
        //
        if(!_sentSynchronously || !userThread)
        {
            __sentAsync();
        }
        else
        {
            __sentInternal();
        }
    }

    private int _useCount;
}
