// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.interceptor;

import java.util.concurrent.CompletionStage;
import java.util.concurrent.CompletableFuture;

import test.Ice.interceptor.Test.InvalidInputException;
import test.Ice.interceptor.Test.RetryException;
import test.Ice.interceptor.Test.MyObject;

class MyObjectI implements MyObject
{
    @Override
    public int add(int x, int y, com.zeroc.Ice.Current current)
    {
        return x + y;
    }

    @Override
    public int addWithRetry(int x, int y, com.zeroc.Ice.Current current)
    {
        String val = current.ctx.get("retry");

        if(val == null || !val.equals("no"))
        {
            throw new RetryException();
        }
        return x + y;
    }

    @Override
    public int badAdd(int x, int y, com.zeroc.Ice.Current current)
        throws InvalidInputException
    {
        throw new InvalidInputException();
    }

    @Override
    public int notExistAdd(int x, int y, com.zeroc.Ice.Current current)
    {
        throw new com.zeroc.Ice.ObjectNotExistException();
    }

    @Override
    public int badSystemAdd(int x, int y, com.zeroc.Ice.Current current)
    {
        throw new MySystemException();
    }

    //
    // AMD
    //

    @Override
    public CompletionStage<Integer> amdAddAsync(final int x, final int y, com.zeroc.Ice.Current current)
    {
        CompletableFuture<Integer> r = new CompletableFuture<>();
        Thread thread = new Thread(() ->
            {
                try
                {
                    Thread.sleep(10);
                }
                catch(InterruptedException e)
                {
                }
                r.complete(x + y);
            });

        thread.setDaemon(true);
        thread.start();
        return r;
    }

    @Override
    public CompletionStage<Integer> amdAddWithRetryAsync(final int x, final int y, com.zeroc.Ice.Current current)
    {
        String val = current.ctx.get("retry");

        if(val != null && val.equals("no"))
        {
            try
            {
                Thread.sleep(10);
            }
            catch(InterruptedException e)
            {
            }
            return CompletableFuture.completedFuture(x + y);
        }
        else
        {
            throw new RetryException();
        }
    }

    @Override
    public CompletionStage<Integer> amdBadAddAsync(int x, int y, com.zeroc.Ice.Current current)
    {
        CompletableFuture<Integer> r = new CompletableFuture<>();
        Thread thread = new Thread(() ->
            {
                try
                {
                    Thread.sleep(10);
                }
                catch(InterruptedException e)
                {
                }
                r.completeExceptionally(new InvalidInputException());
            });
        thread.setDaemon(true);
        thread.start();
        return r;
    }

    @Override
    public CompletionStage<Integer> amdNotExistAddAsync(int x, int y, com.zeroc.Ice.Current current)
    {
        CompletableFuture<Integer> r = new CompletableFuture<>();
        Thread thread = new Thread(() ->
            {
                try
                {
                    Thread.sleep(10);
                }
                catch(InterruptedException e)
                {
                }
                r.completeExceptionally(new com.zeroc.Ice.ObjectNotExistException());
            });
        thread.setDaemon(true);
        thread.start();
        return r;
    }

    @Override
    public CompletionStage<Integer> amdBadSystemAddAsync(int x, int y, com.zeroc.Ice.Current current)
    {
        CompletableFuture<Integer> r = new CompletableFuture<>();
        Thread thread = new Thread(() ->
            {
                try
                {
                    Thread.sleep(10);
                }
                catch(InterruptedException e)
                {
                }
                r.completeExceptionally(new MySystemException());
            });
        thread.setDaemon(true);
        thread.start();
        return r;
    }
}
