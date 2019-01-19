//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package Ice;

/**
 * An AsyncResult object is the return value of an asynchronous invocation.
 * With this object, an application can obtain several attributes of the
 * invocation and discover its outcome.
 **/
public interface AsyncResult
{
    /**
     * If not completed, cancels the request. This is a local
     * operation, it won't cancel the request on the server side.
     * Calling <code>cancel</code> prevents a queued request from
     * being sent or ignores a reply if the request has already
     * been sent.
     **/
    public void cancel();

    /**
     * Returns the communicator that sent the invocation.
     *
     * @return The communicator.
     **/
    public Communicator getCommunicator();

    /**
     * Returns the connection that was used to call the <code>begin_</code> method, or nil
     * if this AsyncResult object was not obtained via an asynchronous connection invocation
     * (such as <code>begin_flushBatchRequests</code>).
     *
     * @return The connection.
     **/
    public Connection getConnection();

    /**
     * Returns the proxy that was used to call the <code>begin_</code> method, or nil
     * if this AsyncResult object was not obtained via an asynchronous proxy invocation.
     *
     * @return The proxy.
     **/
    public ObjectPrx getProxy();

    /**
     * Indicates whether the result of an invocation is available.
     *
     * @return True if the result is available, which means a call to the <code>end_</code>
     * method will not block. The method returns false if the result is not yet available.
     **/
    public boolean isCompleted();

    /**
     * Blocks the caller until the result of the invocation is available.
     **/
    public void waitForCompleted();

    /**
     * When you call the <code>begin_</code> method, the Ice run time attempts to
     * write the corresponding request to the client-side transport. If the
     * transport cannot accept the request, the Ice run time queues the request
     * for later transmission. This method returns true if, at the time it is called,
     * the request has been written to the local transport (whether it was initially
     * queued or not). Otherwise, if the request is still queued, this method returns
     * false.
     *
     * @return True if the request has been sent, or false if the request is queued.
     **/
    public boolean isSent();

    /**
     * Blocks the caller until the request has been written to the client-side transport.
     **/
    public void waitForSent();

    /**
     * If the invocation failed with a local exception, throws the local exception.
     **/
    public void throwLocalException();

    /**
     * This method returns true if a request was written to the client-side
     * transport without first being queued. If the request was initially
     * queued, this method returns false (independent of whether the request
     * is still in the queue or has since been written to the client-side transport).
     *
     * @return True if the request was sent without being queued, or false
     * otherwise.
     **/
    public boolean sentSynchronously();

    /**
     * Returns the name of the operation.
     *
     * @return The operation name.
     **/
    public String getOperation();
}
