// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.Ice;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.Executor;

/**
 * An instance of an InvocationFuture subclass is the return value of an asynchronous invocation.
 * With this object, an application can obtain several attributes of the invocation.
 **/
public abstract class InvocationFuture<T> extends CompletableFuture<T>
{
    /**
     * If not completed, cancels the request. This is a local
     * operation, it won't cancel the request on the server side.
     * Calling <code>cancel</code> prevents a queued request from
     * being sent or ignores a reply if the request has already
     * been sent.
     *
     * @return True if this task is now cancelled.
     **/
    public abstract boolean cancel();

    /**
     * Returns the communicator that sent the invocation.
     *
     * @return The communicator.
     **/
    public abstract Communicator getCommunicator();

    /**
     * Returns the connection that was used to start the invocation, or nil
     * if this future was not obtained via an asynchronous connection invocation
     * (such as <code>flushBatchRequestsAsync</code>).
     *
     * @return The connection.
     **/
    public abstract Connection getConnection();

    /**
     * Returns the proxy that was used to start the asynchronous invocation, or nil
     * if this object was not obtained via an asynchronous proxy invocation.
     *
     * @return The proxy.
     **/
    public abstract ObjectPrx getProxy();

    /**
     * Returns the name of the operation.
     *
     * @return The operation name.
     **/
    public abstract String getOperation();

    /**
     * Blocks the caller until the result of the invocation is available.
     **/
    public abstract void waitForCompleted();

    /**
     * When you start an asynchronous invocation, the Ice run time attempts to
     * write the corresponding request to the client-side transport. If the
     * transport cannot accept the request, the Ice run time queues the request
     * for later transmission. This method returns true if, at the time it is called,
     * the request has been written to the local transport (whether it was initially
     * queued or not). Otherwise, if the request is still queued, this method returns
     * false.
     *
     * @return True if the request has been sent, or false if the request is queued.
     **/
    public abstract boolean isSent();

    /**
     * Blocks the caller until the request has been written to the client-side transport.
     **/
    public abstract void waitForSent();

    /**
     * Returns true if a request was written to the client-side
     * transport without first being queued. If the request was initially
     * queued, this method returns false (independent of whether the request
     * is still in the queue or has since been written to the client-side transport).
     *
     * @return True if the request was sent without being queued, or false
     * otherwise.
     **/
    public abstract boolean sentSynchronously();

    /**
     * Returns a future that completes when the entire request message has been
     * accepted by the transport and executes the given action. The boolean value
     * indicates whether the message was sent synchronously.
     *
     * @param action Executed when the future is completed successfully or exceptionally.
     * @return A future that completes when the message has been handed off to the transport.
     **/
    public abstract CompletableFuture<Boolean> whenSent(
        java.util.function.BiConsumer<Boolean, ? super Throwable> action);

    /**
     * Returns a future that completes when the entire request message has been
     * accepted by the transport and executes the given action using the default executor. The boolean value
     * indicates whether the message was sent synchronously.
     *
     * @param action Executed when the future is completed successfully or exceptionally.
     * @return A future that completes when the message has been handed off to the transport.
     **/
    public abstract CompletableFuture<Boolean> whenSentAsync(
        java.util.function.BiConsumer<Boolean, ? super Throwable> action);

    /**
     * Returns a future that completes when the entire request message has been
     * accepted by the transport and executes the given action using the executor. The boolean value
     * indicates whether the message was sent synchronously.
     *
     * @param action Executed when the future is completed successfully or exceptionally.
     * @param executor The executor to use for asynchronous execution.
     * @return A future that completes when the message has been handed off to the transport.
     **/
    public abstract CompletableFuture<Boolean> whenSentAsync(
        java.util.function.BiConsumer<Boolean, ? super Throwable> action,
        Executor executor);
}
