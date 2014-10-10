// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import java.util.concurrent.RejectedExecutionException;

import Ice.CommunicatorDestroyedException;

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

    public void flushConnection(final Ice.ConnectionI con)
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
                    _childObserver.failed(ex.ice_name());
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
            if(_instance.queueRequests())
            {
                Future<Integer> future = _instance.getQueueExecutor().submit(new Callable<Integer>()
                {
                    @Override
                    public Integer call() throws RetryException
                    {
                        return con.flushAsyncBatchRequests(new FlushBatch());
                    }
                });

                boolean interrupted = false;
                while(true)
                {
                    try 
                    {
                        future.get();
                        if(interrupted)
                        {
                            Thread.currentThread().interrupt();
                        }
                        break;
                    }
                    catch(InterruptedException ex)
                    {
                        interrupted = true;
                    }
                    catch(RejectedExecutionException e)
                    {
                        throw new CommunicatorDestroyedException();
                    }
                    catch(ExecutionException e)
                    {
                        try
                        {
                            throw e.getCause();
                        }
                        catch(RuntimeException ex)
                        {
                            throw ex;
                        }
                        catch(Throwable ex)
                        {
                            assert(false);
                        }
                    }
                }
            }
            else
            {
                con.flushAsyncBatchRequests(new FlushBatch());
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
