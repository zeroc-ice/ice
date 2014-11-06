// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Callable;
import java.util.concurrent.Future;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.RejectedExecutionException;

final class QueueExecutorService
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

    <T> T executeNoThrow(Callable<T> callable)
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

    <T> T execute(Callable<T> callable)
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

        try
        {
            Future<T> future = _executor.submit(callable);
            boolean interrupted = false;
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
                finally
                {
                    if(interrupted)
                    {
                        Thread.currentThread().interrupt();
                    }
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
    }

    final ExecutorService _executor;
    final Thread _thread;
}
