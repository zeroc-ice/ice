//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

import com.zeroc.Ice.Annotations.*;

/**
 * The user-level interface to a connection.
 **/
public interface Connection
{
    /**
     * Manually close the connection using the specified closure mode.
     * @param mode Determines how the connection will be closed.
     *
     * @see ConnectionClose
     **/
    void close(ConnectionClose mode);

    /**
     * Create a special proxy that always uses this connection. This
     * can be used for callbacks from a server to a client if the
     * server cannot directly establish a connection to the client,
     * for example because of firewalls. In this case, the server
     * would create a proxy using an already established connection
     * from the client.
     * @param id The identity for which a proxy is to be created.
     * @return A proxy that matches the given identity and uses this
     * connection.
     *
     * @see #setAdapter
     **/
    ObjectPrx createProxy(Identity id);

    /**
     * Explicitly set an object adapter that dispatches requests that
     * are received over this connection. A client can invoke an
     * operation on a server using a proxy, and then set an object
     * adapter for the outgoing connection that is used by the proxy
     * in order to receive callbacks. This is useful if the server
     * cannot establish a connection back to the client, for example
     * because of firewalls.
     * @param adapter The object adapter that should be used by this
     * connection to dispatch requests. The object adapter must be
     * activated. When the object adapter is deactivated, it is
     * automatically removed from the connection. Attempts to use a
     * deactivated object adapter raise {@link ObjectAdapterDeactivatedException}
     *
     * @see #createProxy
     * @see #getAdapter
     **/
    void setAdapter(ObjectAdapter adapter);

    /**
     * Get the object adapter that dispatches requests for this
     * connection.
     * @return The object adapter that dispatches requests for the
     * connection, or null if no adapter is set.
     *
     * @see #setAdapter
     **/
    ObjectAdapter getAdapter();

    /**
     * Get the endpoint from which the connection was created.
     * @return The endpoint from which the connection was created.
     **/
    Endpoint getEndpoint();

    /**
     * Flush any pending batch requests for this connection.
     * This means all batch requests invoked on fixed proxies
     * associated with the connection.
     * @param compress Specifies whether or not the queued batch requests
     * should be compressed before being sent over the wire.
     **/
    void flushBatchRequests(CompressBatch compress);

    /**
     * Flush any pending batch requests for this connection.
     * This means all batch requests invoked on fixed proxies
     * associated with the connection.
     * @param compress Specifies whether or not the queued batch requests
     * should be compressed before being sent over the wire.
     * @return A future that will be completed when the invocation completes.
     **/
    java.util.concurrent.CompletableFuture<Void> flushBatchRequestsAsync(CompressBatch compress);

    /**
     * Set a close callback on the connection. The callback is called by the
     * connection when it's closed. The callback is called from the
     * Ice thread pool associated with the connection. If the callback needs
     * more information about the closure, it can call {@link Connection#throwException}.
     * @param callback The close callback object.
     **/
    void setCloseCallback(CloseCallback callback);

    /**
     * Set a heartbeat callback on the connection. The callback is called by the
     * connection when a heartbeat is received. The callback is called
     * from the Ice thread pool associated with the connection.
     * @param callback The heartbeat callback object.
     **/
    void setHeartbeatCallback(HeartbeatCallback callback);

    /**
     * Send a heartbeat message.
     **/
    void heartbeat();

    /**
     * Send a heartbeat message.
     * @return A future that will be completed when the invocation completes.
     **/
    java.util.concurrent.CompletableFuture<Void> heartbeatAsync();

    /**
     * Set the active connection management parameters.
     * @param timeout The timeout value in seconds, must be &gt;= 0.
     * @param close The close condition
     * @param heartbeat The hertbeat condition
     **/
    void setACM(@Nullable Integer timeout, @Nullable ACMClose close, @Nullable ACMHeartbeat heartbeat);

    /**
     * Get the ACM parameters.
     * @return The ACM parameters.
     **/
    ACM getACM();

    /**
     * Return the connection type. This corresponds to the endpoint
     * type, i.e., "tcp", "udp", etc.
     * @return The type of the connection.
     **/
    String type();

    /**
     * Get the timeout for the connection.
     * @return The connection's timeout.
     **/
    int timeout();

    /**
     * Return a description of the connection as human readable text,
     * suitable for logging or error messages.
     * @return The description of the connection as human readable
     * text.
     **/
    String _toString();

    /**
     * Returns the connection information.
     * @return The connection information.
     **/
    ConnectionInfo getInfo();

    /**
     * Set the connection buffer receive/send size.
     * @param rcvSize The connection receive buffer size.
     * @param sndSize The connection send buffer size.
     **/
    void setBufferSize(int rcvSize, int sndSize);

    /**
     * Throw an exception indicating the reason for connection closure. For example,
     * {@link CloseConnectionException} is raised if the connection was closed gracefully,
     * whereas {@link ConnectionManuallyClosedException} is raised if the connection was
     * manually closed by the application. This operation does nothing if the connection is
     * not yet closed.
     **/
    void throwException();
}
