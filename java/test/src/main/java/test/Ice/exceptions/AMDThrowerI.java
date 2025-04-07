// Copyright (c) ZeroC, Inc.

package test.Ice.exceptions;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.DispatchException;
import com.zeroc.Ice.TimeoutException;
import com.zeroc.Ice.UserException;

import test.Ice.exceptions.AMD.Test.A;
import test.Ice.exceptions.AMD.Test.B;
import test.Ice.exceptions.AMD.Test.C;
import test.Ice.exceptions.AMD.Test.D;
import test.Ice.exceptions.AMD.Test.Thrower;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

public final class AMDThrowerI implements Thrower {
    public AMDThrowerI() {}

    @Override
    public CompletionStage<Void> shutdownAsync(Current current) {
        current.adapter.getCommunicator().shutdown();
        return CompletableFuture.completedFuture((Void) null);
    }

    @Override
    public CompletionStage<Boolean> supportsUndeclaredExceptionsAsync(
            Current current) {
        return CompletableFuture.completedFuture(true);
    }

    @Override
    public CompletionStage<Boolean> supportsAssertExceptionAsync(Current current) {
        return CompletableFuture.completedFuture(true);
    }

    @Override
    public CompletionStage<Void> throwAasAAsync(int a, Current current) throws A {
        CompletableFuture<Void> r = new CompletableFuture<>();
        A ex = new A();
        ex.aMem = a;
        r.completeExceptionally(ex);
        return r;
    }

    @Override
    public CompletionStage<Void> throwAorDasAorDAsync(int a, Current current)
            throws A, D {
        CompletableFuture<Void> r = new CompletableFuture<>();
        if (a > 0) {
            A ex = new A();
            ex.aMem = a;
            r.completeExceptionally(ex);
        } else {
            D ex = new D();
            ex.dMem = a;
            r.completeExceptionally(ex);
        }
        return r;
    }

    @Override
    public CompletionStage<Void> throwBasAAsync(int a, int b, Current current)
            throws A {
        CompletableFuture<Void> r = new CompletableFuture<>();
        B ex = new B();
        ex.aMem = a;
        ex.bMem = b;
        throw ex;
        // r.completeExceptionally(ex);
    }

    @Override
    public CompletionStage<Void> throwBasBAsync(int a, int b, Current current)
            throws B {
        CompletableFuture<Void> r = new CompletableFuture<>();
        B ex = new B();
        ex.aMem = a;
        ex.bMem = b;
        throw ex;
        // r.completeExceptionally(ex);
    }

    @Override
    public CompletionStage<Void> throwCasAAsync(int a, int b, int c, Current current)
            throws A {
        CompletableFuture<Void> r = new CompletableFuture<>();
        C ex = new C();
        ex.aMem = a;
        ex.bMem = b;
        ex.cMem = c;
        r.completeExceptionally(ex);
        return r;
    }

    @Override
    public CompletionStage<Void> throwCasBAsync(int a, int b, int c, Current current)
            throws B {
        CompletableFuture<Void> r = new CompletableFuture<>();
        C ex = new C();
        ex.aMem = a;
        ex.bMem = b;
        ex.cMem = c;
        r.completeExceptionally(ex);
        return r;
    }

    @Override
    public CompletionStage<Void> throwCasCAsync(int a, int b, int c, Current current)
            throws C {
        CompletableFuture<Void> r = new CompletableFuture<>();
        C ex = new C();
        ex.aMem = a;
        ex.bMem = b;
        ex.cMem = c;
        r.completeExceptionally(ex);
        return r;
    }

    @Override
    public CompletionStage<Void> throwUndeclaredAAsync(int a, Current current) {
        CompletableFuture<Void> r = new CompletableFuture<>();
        A ex = new A();
        ex.aMem = a;
        r.completeExceptionally(ex);
        return r;
    }

    @Override
    public CompletionStage<Void> throwUndeclaredBAsync(
            int a, int b, Current current) {
        CompletableFuture<Void> r = new CompletableFuture<>();
        B ex = new B();
        ex.aMem = a;
        ex.bMem = b;
        r.completeExceptionally(ex);
        return r;
    }

    @Override
    public CompletionStage<Void> throwUndeclaredCAsync(
            int a, int b, int c, Current current) throws UserException {
        CompletableFuture<Void> r = new CompletableFuture<>();
        C ex = new C();
        ex.aMem = a;
        ex.bMem = b;
        ex.cMem = c;
        throw ex;
        // r.completeExceptionally(ex);
        // return r;
    }

    @Override
    public CompletionStage<Void> throwLocalExceptionAsync(Current current) {
        CompletableFuture<Void> r = new CompletableFuture<>();
        r.completeExceptionally(new TimeoutException());
        return r;
    }

    @Override
    public CompletionStage<Void> throwNonIceExceptionAsync(Current current) {
        throw new RuntimeException();
    }

    @Override
    public CompletionStage<Void> throwAssertExceptionAsync(Current current) {
        throw new AssertionError();
    }

    @Override
    public CompletionStage<byte[]> throwMemoryLimitExceptionAsync(
            byte[] seq, Current current) {
        //
        // 20KB is over the configured 10KB message size max.
        //
        return CompletableFuture.completedFuture(new byte[1024 * 20]);
    }

    @Override
    public CompletionStage<Void> throwLocalExceptionIdempotentAsync(Current current) {
        CompletableFuture<Void> r = new CompletableFuture<>();
        r.completeExceptionally(new TimeoutException());
        return r;
    }

    @Override
    public CompletionStage<Void> throwDispatchExceptionAsync(
            byte replyStatus, Current current) {
        CompletableFuture<Void> r = new CompletableFuture<>();
        // We convert the signed byte into a positive int.
        r.completeExceptionally(new DispatchException(replyStatus & 0xFF));
        return r;
    }

    @Override
    public CompletionStage<Void> throwAfterResponseAsync(Current current) {
        // The Java 8 mapping doesn't support completing a request and continuing to use the
        // dispatch thread.

        return CompletableFuture.completedFuture((Void) null);
    }

    @Override
    public CompletionStage<Void> throwAfterExceptionAsync(Current current) throws A {
        // The Java 8 mapping doesn't support completing a request and continuing to use the
        // dispatch thread.

        CompletableFuture<Void> r = new CompletableFuture<>();
        r.completeExceptionally(new A());
        return r;
    }
}
