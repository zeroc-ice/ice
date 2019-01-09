// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package IceInternal;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Callable;
import java.util.concurrent.Future;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.RejectedExecutionException;

public final class QueueExecutorService
{
    QueueExecutorService(ExecutorService executor)
    {
        _executor = executor;
        _thread = executeNoThrow(new Callable<Thread>()
        {
            @Override
            public Thread call()
            {
                return Thread.currentThread();
            }
        });
    }

    public <T> T executeNoThrow(Callable<T> callable)
    {
        try
        {
            return execute(callable);
        }
        catch(RetryException ex)
        {
            assert(false);
            return null;
        }
    }

    public <T> T execute(Callable<T> callable)
        throws RetryException
    {
        if(_thread == Thread.currentThread())
        {
            try
            {
                return callable.call();
            }
            catch(RuntimeException ex)
            {
                throw ex;
            }
            catch(Exception ex)
            {
                // RetryException is the only checked exception that
                // can be raised by Ice internals.
                assert(ex instanceof RetryException);
                throw (RetryException)ex;
            }
        }

        boolean interrupted = false;
        try
        {
            Future<T> future = _executor.submit(callable);
            while(true)
            {
                try
                {
                    T value = future.get();
                    return value;
                }
                catch(InterruptedException ex)
                {
                    interrupted = true;
                }
            }
        }
        catch(RejectedExecutionException e)
        {
            throw new Ice.CommunicatorDestroyedException();
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
                // RetryException is the only checked exception that
                // can be raised by Ice internals.
                assert(ex instanceof RetryException);
                throw (RetryException)ex;
            }
        }
        finally
        {
            if(interrupted)
            {
                Thread.currentThread().interrupt();
            }
        }
    }

    final ExecutorService _executor;
    final Thread _thread;
}
