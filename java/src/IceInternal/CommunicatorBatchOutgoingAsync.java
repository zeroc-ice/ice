// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class CommunicatorBatchOutgoingAsync extends OutgoingAsyncBase
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

            @Override
            public boolean
            sent()
            {
                if(_childObserver != null)
                {
                    _childObserver.detach();
                    _childObserver = null;
                }
                doCheck(false);
                return false;
            }

            // TODO: MJN: This is missing a test.
            @Override
            public void
            finished(Ice.Exception ex)
            {
                if(_childObserver != null)
                {
                    _childObserver.failed(ex.ice_name());
                    _childObserver.detach();
                    _childObserver = null;
                }
                doCheck(false);
            }

            @Override
            public void
            attachRemoteObserver(Ice.ConnectionInfo info, Ice.Endpoint endpt, int requestId, int size)
            {
                if(CommunicatorBatchOutgoingAsync.this._observer != null)
                {
                    _childObserver = CommunicatorBatchOutgoingAsync.this._observer.getRemoteObserver(info, endpt,
                                                                                                      requestId, size);
                    if(_childObserver != null)
                    {
                        _childObserver.attach();
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
            doCheck(false);
            throw ex;
        }
    }

    public void ready()
    {
        doCheck(true);
    }

    private void doCheck(boolean userThread)
    {
        synchronized(_monitor)
        {
            assert(_useCount > 0);
            if(--_useCount > 0)
            {
                return;
            }
            _state |= StateDone | StateOK | StateSent;
            _os.resize(0, false); // Clear buffer now, instead of waiting for AsyncResult deallocation
            _monitor.notifyAll();
        }

        if(_callback == null || !_callback.__hasSentCallback())
        {
            if(_observer != null)
            {
                _observer.detach();
                _observer = null;
            }
        }
        else
        {
            //
            // sentSynchronously_ is immutable here.
            //
            if(!_sentSynchronously || !userThread)
            {
                invokeSentAsync();
            }
            else
            {
                invokeSentInternal();
            }
        }
    }

    private int _useCount;
}
