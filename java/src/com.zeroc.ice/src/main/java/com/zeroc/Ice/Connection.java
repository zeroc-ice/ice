// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.concurrent.CompletableFuture;

/** Represents a connection that uses the Ice protocol. */
public interface Connection {
    /** Aborts this connection. */
    void abort();

    /**
     * Closes the connection gracefully after waiting for all outstanding invocations to complete.
     * This method blocks until the connection has been closed, or if it takes longer than the
     * configured close timeout, the connection is aborted with a {@link CloseTimeoutException}.
     */
    void close();

    /**
     * Creates a special proxy (a "fixed proxy") that always uses this connection.
     *
     * @param id the identity of the target object
     * @return a fixed proxy with the provided identity
     * @see #setAdapter
     */
    ObjectPrx createProxy(Identity id);

    /**
     * Associates an object adapter with this connection. When a connection receives a request, it dispatches this
     * request using its associated object adapter. If the associated object adapter is null, the connection rejects
     * any incoming request with an {@link ObjectNotExistException}.
     * The default object adapter of an incoming connection is the object adapter that created this connection;
     * the default object adapter of an outgoing connection is the communicator's default object adapter.
     *
     * @param adapter the object adapter to associate with the connection
     * @see #createProxy
     * @see #getAdapter
     * @see Communicator#setDefaultObjectAdapter
     */
    void setAdapter(ObjectAdapter adapter);

    /**
     * Gets the object adapter associated with this connection.
     *
     * @return the object adapter associated with this connection
     * @see #setAdapter
     * @see Communicator#getDefaultObjectAdapter
     */
    ObjectAdapter getAdapter();

    /**
     * Gets the endpoint from which the connection was created.
     *
     * @return the endpoint from which the connection was created
     */
    Endpoint getEndpoint();

    /**
     * Flushes any pending batch requests for this connection.
     * This corresponds to all batch requests invoked on fixed proxies associated with the connection.
     *
     * @param compress Specifies whether or not the queued batch requests should be compressed before
     *     being sent over the wire.
     */
    void flushBatchRequests(CompressBatch compress);

    /**
     * Flushes any pending batch requests for this connection.
     * This corresponds to all batch requests invoked on fixed proxies associated with the connection.
     *
     * @param compress Specifies whether or not the queued batch requests should be compressed before
     *     being sent over the wire.
     * @return a future that becomes available when the flush completes
     */
    CompletableFuture<Void> flushBatchRequestsAsync(CompressBatch compress);

    /**
     * Sets a close callback on the connection. The callback is called by the connection when it's closed.
     * The callback is called from the Ice thread pool associated with the connection.
     *
     * @param callback the close callback object
     */
    void setCloseCallback(CloseCallback callback);

    /**
     * Disables the inactivity check on this connection.
     */
    void disableInactivityCheck();

    /**
     * Gets the connection type. This corresponds to the endpoint type, such as "tcp", "udp", etc.
     *
     * @return the type of the connection
     */
    String type();

    /**
     * Gets a description of the connection as human readable text, suitable for logging or error messages.
     * This method remains usable after the connection is closed or aborted.
     *
     * @return the description of the connection as human readable text
     */
    String _toString();

    /**
     * Gets the connection information.
     *
     * @return the connection information
     */
    ConnectionInfo getInfo();

    /**
     * Sets the size of the receive and send buffers.
     *
     * @param rcvSize the size of the receive buffer
     * @param sndSize the size of the send buffer
     */
    void setBufferSize(int rcvSize, int sndSize);

    /**
     * Throws an exception that provides the reason for the closure of this connection. For example,
     * this method throws {@link CloseConnectionException} when the connection was closed gracefully by the peer;
     * it throws {@link ConnectionAbortedException} when the connection is aborted with {@link #abort}.
     * This method does nothing if the connection is not yet closed.
     */
    void throwException();
}
