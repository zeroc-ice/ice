// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
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
                    _remoteObserver.detach();
                    _remoteObserver = null;
                }
                check(false);
            }

            public void
            __attachRemoteObserver(Ice.ConnectionInfo info, Ice.Endpoint endpt)
            {
                if(CommunicatorBatchOutgoingAsync.this._observer != null)
                {
                    _remoteObserver = CommunicatorBatchOutgoingAsync.this._observer.getRemoteObserver(info, endpt);
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
        
        int status = con.flushAsyncBatchRequests(new BatchOutgoingAsyncI());
        if((status & AsyncStatus.Sent) > 0)
        {
            _sentSynchronously = false;
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
            
            if(_observer != null)
            {
                _observer.detach();
                _observer = null;
            }

            _state |= Done | OK | Sent;
            _monitor.notifyAll();
        }

        //
        // sentSynchronously_ is immutable here.
        //
        if(!_sentSynchronously && userThread)
        {
            __sentAsync();
        }
        else
        {
            assert(_sentSynchronously == userThread); // sentSynchronously && !userThread is impossible.
            __sentInternal();
        }
    }

    private int _useCount;
}
