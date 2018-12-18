// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.metrics;

import java.util.concurrent.CompletionStage;
import java.util.concurrent.CompletableFuture;

import test.Ice.metrics.AMD.Test.*;

public final class AMDMetricsI implements Metrics
{
    public AMDMetricsI()
    {
    }

    @Override
    public CompletionStage<Void> opAsync(com.zeroc.Ice.Current current)
    {
        return CompletableFuture.completedFuture((Void)null);
    }

    @Override
    public CompletionStage<Void> failAsync(com.zeroc.Ice.Current current)
    {
        current.con.close(com.zeroc.Ice.ConnectionClose.Forcefully);
        return CompletableFuture.completedFuture((Void)null);
    }

    @Override
    public CompletionStage<Void> opWithUserExceptionAsync(com.zeroc.Ice.Current current)
        throws UserEx
    {
        CompletableFuture<Void> r = new CompletableFuture<>();
        r.completeExceptionally(new UserEx());
        return r;
    }

    @Override
    public CompletionStage<Void> opWithRequestFailedExceptionAsync(com.zeroc.Ice.Current current)
    {
        CompletableFuture<Void> r = new CompletableFuture<>();
        r.completeExceptionally(new com.zeroc.Ice.ObjectNotExistException());
        return r;
    }

    @Override
    public CompletionStage<Void> opWithLocalExceptionAsync(com.zeroc.Ice.Current current)
    {
        CompletableFuture<Void> r = new CompletableFuture<>();
        r.completeExceptionally(new com.zeroc.Ice.SyscallException());
        return r;
    }

    @Override
    public CompletionStage<Void> opWithUnknownExceptionAsync(com.zeroc.Ice.Current current)
    {
        CompletableFuture<Void> r = new CompletableFuture<>();
        r.completeExceptionally(new IllegalArgumentException());
        return r;
    }

    @Override
    public CompletionStage<Void> opByteSAsync(byte[] bs, com.zeroc.Ice.Current current)
    {
        return CompletableFuture.completedFuture((Void)null);
    }

    @Override
    public com.zeroc.Ice.ObjectPrx getAdmin(com.zeroc.Ice.Current current)
    {
        return current.adapter.getCommunicator().getAdmin();
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
