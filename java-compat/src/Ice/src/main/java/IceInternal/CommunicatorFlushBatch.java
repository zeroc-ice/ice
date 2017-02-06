// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

import java.util.concurrent.Callable;

public class CommunicatorFlushBatch extends IceInternal.AsyncResultI
{
    public static CommunicatorFlushBatch check(Ice.AsyncResult r, Ice.Communicator com, String operation)
    {
        check(r, operation);
        if(!(r instanceof CommunicatorFlushBatch))
        {
            throw new IllegalArgumentException("Incorrect AsyncResult object for end_" + operation + " method");
        }
        if(r.getCommunicator() != com)
        {
            throw new IllegalArgumentException("Communicator for call to end_" + operation +
                                               " does not match communicator that was used to call corresponding " +
                                               "begin_" + operation + " method");
        }
        return (CommunicatorFlushBatch)r;
    }

    public CommunicatorFlushBatch(Ice.Communicator communicator, Instance instance, String op, CallbackBase callback)
    {
        super(communicator, instance, op, callback);

        _observer = ObserverHelper.get(instance, op);

        //
        // _useCount is initialized to 1 to prevent premature callbacks.
        // The caller must invoke ready() after all flush requests have
        // been initiated.
        //
        _useCount = 1;
    }

    public void flushConnection(final Ice.ConnectionI con, final Ice.CompressBatch compressBatch)
    {
        class FlushBatch extends OutgoingAsyncBase
        {
            public FlushBatch()
            {
                super(CommunicatorFlushBatch.this.getCommunicator(),
                      CommunicatorFlushBatch.this._instance,
                      CommunicatorFlushBatch.this.getOperation(),
                      null);
            }

            @Override
            public boolean sent()
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
            public boolean completed(Ice.Exception ex)
            {
                if(_childObserver != null)
                {
                    _childObserver.failed(ex.ice_id());
                    _childObserver.detach();
                    _childObserver = null;
                }
                doCheck(false);
                return false;
            }

            @Override
            protected Ice.Instrumentation.InvocationObserver getObserver()
            {
                return CommunicatorFlushBatch.this._observer;
            }
        }

        synchronized(this)
        {
            ++_useCount;
        }

        try
        {
            final FlushBatch flushBatch = new FlushBatch();
            final Ice.BooleanHolder compress = new Ice.BooleanHolder();
            final int batchRequestNum = con.getBatchRequestQueue().swap(flushBatch.getOs(), compress);
            if(batchRequestNum == 0)
            {
                flushBatch.sent();
            }
            else if(_instance.queueRequests())
            {
                _instance.getQueueExecutor().executeNoThrow(new Callable<Void>()
                {
                    @Override
                    public Void call() throws RetryException
                    {
                        boolean comp;
                        if(compressBatch == Ice.CompressBatch.Yes)
                        {
                            comp = true;
                        }
                        else if(compressBatch == Ice.CompressBatch.No)
                        {
                            comp = false;
                        }
                        else
                        {
                            comp = compress.value;
                        }
                        con.sendAsyncRequest(flushBatch, comp, false, batchRequestNum);
                        return null;
                    }
                });
            }
            else
            {
                boolean comp;
                if(compressBatch == Ice.CompressBatch.Yes)
                {
                    comp = true;
                }
                else if(compressBatch == Ice.CompressBatch.No)
                {
                    comp = false;
                }
                else
                {
                    comp = compress.value;
                }
                con.sendAsyncRequest(flushBatch, comp, false, batchRequestNum);
            }
        }
        catch(RetryException ex)
        {
            doCheck(false);
            throw ex.get();
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
        synchronized(this)
        {
            assert(_useCount > 0);
            if(--_useCount > 0)
            {
                return;
            }
        }

        if(sent(true))
        {
            if(userThread)
            {
                _sentSynchronously = true;
                invokeSent();
            }
            else
            {
                invokeSentAsync();
            }
        }
    }

    private int _useCount;
}
