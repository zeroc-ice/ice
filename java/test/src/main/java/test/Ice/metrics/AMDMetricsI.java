// Copyright (c) ZeroC, Inc.

package test.Ice.metrics;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.ObjectNotExistException;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.SyscallException;

import test.Ice.metrics.Test.AsyncMetrics;
import test.Ice.metrics.Test.UserEx;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

public final class AMDMetricsI implements AsyncMetrics {
    public AMDMetricsI() {}

    @Override
    public CompletionStage<Void> opAsync(Current current) {
        return CompletableFuture.completedFuture((Void) null);
    }

    @Override
    public CompletionStage<Void> failAsync(Current current) {
        current.con.abort();
        return CompletableFuture.completedFuture((Void) null);
    }

    @Override
    public CompletionStage<Void> opWithUserExceptionAsync(Current current)
        throws UserEx {
        CompletableFuture<Void> r = new CompletableFuture<>();
        r.completeExceptionally(new UserEx());
        return r;
    }

    @Override
    public CompletionStage<Void> opWithRequestFailedExceptionAsync(Current current) {
        CompletableFuture<Void> r = new CompletableFuture<>();
        r.completeExceptionally(new ObjectNotExistException());
        return r;
    }

    @Override
    public CompletionStage<Void> opWithLocalExceptionAsync(Current current) {
        CompletableFuture<Void> r = new CompletableFuture<>();
        r.completeExceptionally(new SyscallException(null));
        return r;
    }

    @Override
    public CompletionStage<Void> opWithUnknownExceptionAsync(Current current) {
        CompletableFuture<Void> r = new CompletableFuture<>();
        r.completeExceptionally(new IllegalArgumentException());
        return r;
    }

    @Override
    public CompletionStage<Void> opByteSAsync(byte[] bs, Current current) {
        return CompletableFuture.completedFuture((Void) null);
    }

    @Override
    public CompletionStage<ObjectPrx> getAdminAsync(Current current) {
        return CompletableFuture.completedFuture(current.adapter.getCommunicator().getAdmin());
    }

    @Override
    public CompletionStage<Void> shutdownAsync(Current current) {
        current.adapter.getCommunicator().shutdown();
        return CompletableFuture.completedFuture(null);
    }
}
