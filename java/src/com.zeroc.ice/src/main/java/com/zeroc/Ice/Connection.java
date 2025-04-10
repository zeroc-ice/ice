// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.concurrent.CompletableFuture;

/** The user-level interface to a connection. */
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
     * Create a special proxy that always uses this connection. This can be used for callbacks from
     * a server to a client if the server cannot directly establish a connection to the client, for
     * example because of firewalls. In this case, the server would create a proxy using an already
     * established connection from the client.
     *
     * @param id The identity for which a proxy is to be created.
     * @return A proxy that matches the given identity and uses this connection.
     * @see #setAdapter
     */
    ObjectPrx createProxy(Identity id);

    /**
     * Associates an object adapter with this connection. When a connection receives a request, it
     * dispatches this request using its associated object adapter. If the associated object adapter
     * is null, the connection rejects any incoming request with an {@link ObjectNotExistException}.
     * The default object adapter of an incoming connection is the object adapter that created this
     * connection; the default object adapter of an outgoing connection is the communicator's
     * default object adapter.
     *
     * @param adapter The object adapter to associate with the connection.
     * @see #createProxy
     * @see #getAdapter
     * @see Communicator#setDefaultObjectAdapter
     */
    void setAdapter(ObjectAdapter adapter);

    /**
     * Gets the object adapter associated with this connection.
     *
     * @return The object adapter associated with this connection.
     * @see #setAdapter
     * @see Communicator#getDefaultObjectAdapter
     */
    ObjectAdapter getAdapter();

    /**
     * Get the endpoint from which the connection was created.
     *
     * @return The endpoint from which the connection was created.
     */
    Endpoint getEndpoint();

    /**
     * Flush any pending batch requests for this connection. This means all batch requests invoked
     * on fixed proxies associated with the connection.
     *
     * @param compress Specifies whether or not the queued batch requests should be compressed
     *     before being sent over the wire.
     */
    void flushBatchRequests(CompressBatch compress);

    /**
     * Flush any pending batch requests for this connection. This means all batch requests invoked
     * on fixed proxies associated with the connection.
     *
     * @param compress Specifies whether or not the queued batch requests should be compressed
     *     before being sent over the wire.
     * @return A future that will be completed when the invocation completes.
     */
    CompletableFuture<Void> flushBatchRequestsAsync(CompressBatch compress);

    /**
     * Set a close callback on the connection. The callback is called by the connection when it's
     * closed. The callback is called from the Ice thread pool associated with the connection. If
     * the callback needs more information about the closure, it can call {@link
     * Connection#throwException}.
     *
     * @param callback The close callback object.
     */
    void setCloseCallback(CloseCallback callback);

    /**
     * Return the connection type. This corresponds to the endpoint type, i.e., "tcp", "udp", etc.
     *
     * @return The type of the connection.
     */
    String type();

    /**
     * Return a description of the connection as human readable text, suitable for logging or error
     * messages.
     *
     * @return The description of the connection as human readable text.
     */
    String _toString();

    /**
     * Returns the connection information.
     *
     * @return The connection information.
     */
    ConnectionInfo getInfo();

    /**
     * Set the connection buffer receive/send size.
     *
     * @param rcvSize The connection receive buffer size.
     * @param sndSize The connection send buffer size.
     */
    void setBufferSize(int rcvSize, int sndSize);

    /**
     * Throw an exception indicating the reason for connection closure. For example, {@link
     * CloseConnectionException} is raised if the connection was closed gracefully, whereas {@link
     * ConnectionAbortedException}/{@link ConnectionClosedException} is raised if the connection was
     * manually closed by the application. This operation does nothing if the connection is not yet
     * closed.
     */
    void throwException();
}
