// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.servantLocator;

import java.util.concurrent.CompletionStage;
import java.util.concurrent.CompletableFuture;

import test.Ice.servantLocator.AMD.Test.TestImpossibleException;
import test.Ice.servantLocator.AMD.Test.TestIntfUserException;
import test.Ice.servantLocator.AMD.Test.TestIntf;

public final class AMDTestI implements TestIntf
{
    @Override
    public CompletionStage<Void> requestFailedExceptionAsync(com.zeroc.Ice.Current current)
    {
        return CompletableFuture.completedFuture((Void)null);
    }

    @Override
    public CompletionStage<Void> unknownUserExceptionAsync(com.zeroc.Ice.Current current)
    {
        return CompletableFuture.completedFuture((Void)null);
    }

    @Override
    public CompletionStage<Void> unknownLocalExceptionAsync(com.zeroc.Ice.Current current)
    {
        return CompletableFuture.completedFuture((Void)null);
    }

    @Override
    public CompletionStage<Void> unknownExceptionAsync(com.zeroc.Ice.Current current)
    {
        return CompletableFuture.completedFuture((Void)null);
    }

    @Override
    public CompletionStage<Void> localExceptionAsync(com.zeroc.Ice.Current current)
    {
        return CompletableFuture.completedFuture((Void)null);
    }

//     public CompletionStage<Void> userExceptionAsync(com.zeroc.Ice.Current current)
//     {
//        return CompletableFuture.completedFuture((Void)null);
//     }

    @Override
    public CompletionStage<Void> javaExceptionAsync(com.zeroc.Ice.Current current)
    {
        return CompletableFuture.completedFuture((Void)null);
    }

    @Override
    public CompletionStage<Void> unknownExceptionWithServantExceptionAsync(com.zeroc.Ice.Current current)
    {
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(new com.zeroc.Ice.ObjectNotExistException());
        return f;
    }

    @Override
    public CompletionStage<String> impossibleExceptionAsync(boolean _throw, com.zeroc.Ice.Current current)
    {
        if(_throw)
        {
            CompletableFuture<String> f = new CompletableFuture<>();
            f.completeExceptionally(new TestImpossibleException());
            return f;
        }
        else
        {
            //
            // Return a value so we can be sure that the stream position
            // is reset correctly if finished() throws.
            //
            return CompletableFuture.completedFuture("Hello");
        }
    }

    @Override
    public CompletionStage<String> intfUserExceptionAsync(boolean _throw, com.zeroc.Ice.Current current)
    {
        if(_throw)
        {
            CompletableFuture<String> f = new CompletableFuture<>();
            f.completeExceptionally(new TestIntfUserException());
            return f;
        }
        else
        {
            //
            // Return a value so we can be sure that the stream position
            // is reset correctly if finished() throws.
            //
            return CompletableFuture.completedFuture("Hello");
        }
    }

    @Override
    public CompletionStage<Void> asyncResponseAsync(com.zeroc.Ice.Current current)
    {
        //
        // The Java 8 mapping does not support this test.
        //
        //return CompletableFuture.completedFuture((Void)null);
        throw new com.zeroc.Ice.ObjectNotExistException();
    }

    @Override
    public CompletionStage<Void> asyncExceptionAsync(com.zeroc.Ice.Current current)
    {
        //
        // The Java 8 mapping does not support this test.
        //
        //CompletableFuture<Void> f = new CompletableFuture<>();
        //f.completeExceptionally(new TestIntfUserException());
        //return f;
        throw new com.zeroc.Ice.ObjectNotExistException();
    }
    
    @Override
    public CompletionStage<Void> shutdownAsync(com.zeroc.Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        return CompletableFuture.completedFuture((Void)null);
    }
}
