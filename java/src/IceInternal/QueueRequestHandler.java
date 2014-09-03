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
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Future;
import java.util.concurrent.RejectedExecutionException;

import Ice.CommunicatorDestroyedException;
import Ice.ConnectionI;

public class QueueRequestHandler implements RequestHandler
{
    public
    QueueRequestHandler(Instance instance, RequestHandler delegate) 
    {
        _executor = instance.getQueueExecutor();
        assert(delegate != null);
        _delegate = delegate;
    }

    @Override
    public void
    prepareBatchRequest(final BasicStream out) throws RetryException
    {
        try
        {
            Future<Void> future = _executor.submit(new Callable<Void>() 
            {
                @Override
                public Void call() throws RetryException
                {
                    _delegate.prepareBatchRequest(out);
                    return null;
                }
            });

            future.get();
        }
        catch(RejectedExecutionException e)
        {
            throw new CommunicatorDestroyedException();
        }
        catch(InterruptedException e)
        {
            throw new Ice.OperationInterruptedException();
        }
        catch(ExecutionException e)
        {
            try
            {
                throw e.getCause();
            }
            catch(RetryException ex)
            {
                throw ex;
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

    @Override
    public void
    finishBatchRequest(final BasicStream out)
    {
        try
        {
            Future<Void> future = _executor.submit(new Callable<Void>() 
            {
                @Override
                public Void call()
                {
                    _delegate.finishBatchRequest(out);
                    return null;
                }
            });
            future.get();
        }
        catch(RejectedExecutionException e)
        {
            throw new CommunicatorDestroyedException();
        }
        catch(InterruptedException e)
        {
            throw new Ice.OperationInterruptedException();
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

    @Override
    public void
    abortBatchRequest()
    {
        try
        {
            Future<Void> future = _executor.submit(new Callable<Void>()
            {
                @Override
                public Void call()
                {
                    _delegate.abortBatchRequest();
                    return null;
                }
            });
            future.get();
        }
        catch(RejectedExecutionException e)
        {
            throw new CommunicatorDestroyedException();
        }
        catch(InterruptedException e)
        {
            throw new Ice.OperationInterruptedException();
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

    @Override
    public int
    sendAsyncRequest(final OutgoingAsyncMessageCallback out) throws RetryException
    {
        try
        {
            Future<Integer> future = _executor.submit(new Callable<Integer>()
            {
                @Override
                public Integer call() throws RetryException
                {
                    return _delegate.sendAsyncRequest(out);
                }
            });
            return future.get();
        }
        catch(RejectedExecutionException e)
        {
            throw new CommunicatorDestroyedException();
        }
        catch(InterruptedException e)
        {
            // If the request cannot be canceled (or is itself interrupted) then
            // restore the interrupt state.
            try
            {
                if(!asyncRequestCanceled(out,  new Ice.OperationInterruptedException()))
                {
                    Thread.currentThread().interrupt();
                }
            }
            catch(Ice.OperationInterruptedException ex)
            {
                Thread.currentThread().interrupt();
            }
        }
        catch(ExecutionException e)
        {
            try
            {
                throw e.getCause();
            }
            catch(RetryException ex)
            {
                throw ex;
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
        return 0;
    }

    @Override
    public boolean
    asyncRequestCanceled(final OutgoingAsyncMessageCallback outAsync, final Ice.LocalException ex)
    {
        try
        {
            Future<Boolean> future = _executor.submit(new Callable<Boolean>()
            {
                @Override
                public Boolean call()
                {
                    return _delegate.asyncRequestCanceled(outAsync, ex);
                }
            });
            return future.get();
        }
        catch(RejectedExecutionException e)
        {
            throw new CommunicatorDestroyedException();
        }
        catch(InterruptedException e)
        {
            throw new Ice.OperationInterruptedException();
        }
        catch(ExecutionException e)
        {
            try
            {
                throw e.getCause();
            }
            catch(RuntimeException exc)
            {
                throw exc;
            }
            catch(Throwable exc)
            {
                assert(false);
            }
        }
        return false;
    }

    @Override
    public Reference
    getReference()
    {
        return _delegate.getReference();
    }

    @Override
    public ConnectionI
    getConnection()
    {
        return _delegate.getConnection();
    }

    @Override
    public ConnectionI
    waitForConnection() throws InterruptedException, RetryException
    {
        return _delegate.waitForConnection();
    }

    private final RequestHandler _delegate;
    private final ExecutorService _executor;
}
