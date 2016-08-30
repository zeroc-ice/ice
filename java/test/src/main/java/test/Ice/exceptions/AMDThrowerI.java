// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.exceptions;

import java.util.concurrent.CompletionStage;
import java.util.concurrent.CompletableFuture;

import test.Ice.exceptions.AMD.Test.A;
import test.Ice.exceptions.AMD.Test.B;
import test.Ice.exceptions.AMD.Test.C;
import test.Ice.exceptions.AMD.Test.D;
import test.Ice.exceptions.AMD.Test.Thrower;

public final class AMDThrowerI implements Thrower
{
    public AMDThrowerI()
    {
    }

    @Override
    public CompletionStage<Void> shutdownAsync(com.zeroc.Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        return CompletableFuture.completedFuture((Void)null);
    }

    @Override
    public CompletionStage<Boolean> supportsUndeclaredExceptionsAsync(com.zeroc.Ice.Current current)
    {
        return CompletableFuture.completedFuture(true);
    }

    @Override
    public CompletionStage<Boolean> supportsAssertExceptionAsync(com.zeroc.Ice.Current current)
    {
        return CompletableFuture.completedFuture(true);
    }

    @Override
    public CompletionStage<Void> throwAasAAsync(int a, com.zeroc.Ice.Current current)
        throws A
    {
        CompletableFuture<Void> r = new CompletableFuture<>();
        A ex = new A();
        ex.aMem = a;
        r.completeExceptionally(ex);
        return r;
    }

    @Override
    public CompletionStage<Void> throwAorDasAorDAsync(int a, com.zeroc.Ice.Current current)
        throws A,
               D
    {
        CompletableFuture<Void> r = new CompletableFuture<>();
        if(a > 0)
        {
            A ex = new A();
            ex.aMem = a;
            r.completeExceptionally(ex);
        }
        else
        {
            D ex = new D();
            ex.dMem = a;
            r.completeExceptionally(ex);
        }
        return r;
    }

    @Override
    public CompletionStage<Void> throwBasAAsync(int a, int b, com.zeroc.Ice.Current current)
        throws A
    {
        CompletableFuture<Void> r = new CompletableFuture<>();
        B ex = new B();
        ex.aMem = a;
        ex.bMem = b;
        throw ex;
        //r.completeExceptionally(ex);
    }

    @Override
    public CompletionStage<Void> throwBasBAsync(int a, int b, com.zeroc.Ice.Current current)
        throws B
    {
        CompletableFuture<Void> r = new CompletableFuture<>();
        B ex = new B();
        ex.aMem = a;
        ex.bMem = b;
        throw ex;
        //r.completeExceptionally(ex);
    }

    @Override
    public CompletionStage<Void> throwCasAAsync(int a, int b, int c, com.zeroc.Ice.Current current)
        throws A
    {
        CompletableFuture<Void> r = new CompletableFuture<>();
        C ex = new C();
        ex.aMem = a;
        ex.bMem = b;
        ex.cMem = c;
        r.completeExceptionally(ex);
        return r;
    }

    @Override
    public CompletionStage<Void> throwCasBAsync(int a, int b, int c, com.zeroc.Ice.Current current)
        throws B
    {
        CompletableFuture<Void> r = new CompletableFuture<>();
        C ex = new C();
        ex.aMem = a;
        ex.bMem = b;
        ex.cMem = c;
        r.completeExceptionally(ex);
        return r;
    }

    @Override
    public CompletionStage<Void> throwCasCAsync(int a, int b, int c, com.zeroc.Ice.Current current)
        throws C
    {
        CompletableFuture<Void> r = new CompletableFuture<>();
        C ex = new C();
        ex.aMem = a;
        ex.bMem = b;
        ex.cMem = c;
        r.completeExceptionally(ex);
        return r;
    }

    @Override
    public CompletionStage<Void> throwUndeclaredAAsync(int a, com.zeroc.Ice.Current current)
    {
        CompletableFuture<Void> r = new CompletableFuture<>();
        A ex = new A();
        ex.aMem = a;
        r.completeExceptionally(ex);
        return r;
    }

    @Override
    public CompletionStage<Void> throwUndeclaredBAsync(int a, int b, com.zeroc.Ice.Current current)
    {
        CompletableFuture<Void> r = new CompletableFuture<>();
        B ex = new B();
        ex.aMem = a;
        ex.bMem = b;
        r.completeExceptionally(ex);
        return r;
    }

    @Override
    public CompletionStage<Void> throwUndeclaredCAsync(int a, int b, int c, com.zeroc.Ice.Current current)
    {
        CompletableFuture<Void> r = new CompletableFuture<>();
        C ex = new C();
        ex.aMem = a;
        ex.bMem = b;
        ex.cMem = c;
        r.completeExceptionally(ex);
        return r;
    }

    @Override
    public CompletionStage<Void> throwLocalExceptionAsync(com.zeroc.Ice.Current current)
    {
        CompletableFuture<Void> r = new CompletableFuture<>();
        r.completeExceptionally(new com.zeroc.Ice.TimeoutException());
        return r;
    }

    @Override
    public CompletionStage<Void> throwNonIceExceptionAsync(com.zeroc.Ice.Current current)
    {
        throw new RuntimeException();
    }

    @Override
    public CompletionStage<Void> throwAssertExceptionAsync(com.zeroc.Ice.Current current)
    {
        throw new java.lang.AssertionError();
    }

    @Override
    public CompletionStage<byte[]> throwMemoryLimitExceptionAsync(byte[] seq, com.zeroc.Ice.Current current)
    {
        //
        // 20KB is over the configured 10KB message size max.
        //
        return CompletableFuture.completedFuture(new byte[1024 * 20]);
    }

    @Override
    public CompletionStage<Void> throwLocalExceptionIdempotentAsync(com.zeroc.Ice.Current current)
    {
        CompletableFuture<Void> r = new CompletableFuture<>();
        r.completeExceptionally(new com.zeroc.Ice.TimeoutException());
        return r;
    }

    @Override
    public CompletionStage<Void> throwAfterResponseAsync(com.zeroc.Ice.Current current)
    {
        // The Java 8 mapping doesn't support completing a request and continuing to use the dispatch thread.

        // throw new RuntimeException();
    }

    @Override
    public CompletionStage<Void> throwAfterExceptionAsync(com.zeroc.Ice.Current current)
        throws A
    {
        // The Java 8 mapping doesn't support completing a request and continuing to use the dispatch thread.

        // throw new RuntimeException();
    }
}
