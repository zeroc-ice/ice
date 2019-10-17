//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["ice-prefix"]]

[["js:module:ice"]]

[["objc:dll-export:ICE_API"]]
[["objc:header-dir:objc"]]

[["python:pkgdir:Ice"]]

#include <Ice/ObjectAdapterF.ice>
#include <Ice/Identity.ice>
#include <Ice/Endpoint.ice>

#ifndef __SLICE2JAVA_COMPAT__
[["java:package:com.zeroc"]]
#endif

["objc:prefix:ICE"]
module Ice
{

/**
 * The batch compression option when flushing queued batch requests.
 *
 **/
["cpp:scoped", "objc:scoped"]
local enum CompressBatch
{
    /**
     * Compress the batch requests.
     **/
    Yes,

    /**
     * Don't compress the batch requests.
     **/
    No,

    /**
     * Compress the batch requests if at least one request was
     * made on a compressed proxy.
     **/
    BasedOnProxy
}

#if !defined(__SLICE2PHP__) && !defined(__SLICE2MATLAB__)
/**
 *
 * Base class providing access to the connection details.
 *
 **/
local class ConnectionInfo
{
    /**
     *
     * The information of the underyling transport or null if there's
     * no underlying transport.
     *
     **/
    ConnectionInfo underlying;

    /**
     *
     * Whether or not the connection is an incoming or outgoing
     * connection.
     *
     **/
    bool incoming;

    /**
     *
     * The name of the adapter associated with the connection.
     *
     **/
    string adapterName;

    /**
     *
     * The connection id.
     *
     **/
    string connectionId;
}
#endif

local interface Connection;

/**
 *
 * An application can implement this interface to receive notifications when
 * a connection closes.
 *
 * @see Connection#setCloseCallback
 *
 **/
["delegate"]
local interface CloseCallback
{
    /**
     *
     * This method is called by the the connection when the connection
     * is closed. If the callback needs more information about the closure,
     * it can call {@link Connection#throwException}.
     *
     * @param con The connection that closed.
     **/
    ["swift:noexcept"] void closed(Connection con);
}

/**
 *
 * An application can implement this interface to receive notifications when
 * a connection receives a heartbeat message.
 *
 * @see Connection#setHeartbeatCallback
 *
 **/
["delegate"]
local interface HeartbeatCallback
{
    /**
     *
     * This method is called by the the connection when a heartbeat is
     * received from the peer.
     *
     * @param con The connection on which a heartbeat was received.
     **/
    ["swift:noexcept"] void heartbeat(Connection con);
}

/**
 * Specifies the close semantics for Active Connection Management.
 */
local enum ACMClose
{
    /** Disables automatic connection closure. */
    CloseOff,
    /** Gracefully closes a connection that has been idle for the configured timeout period. */
    CloseOnIdle,
    /**
     * Forcefully closes a connection that has been idle for the configured timeout period,
     * but only if the connection has pending invocations.
     */
    CloseOnInvocation,
    /** Combines the behaviors of CloseOnIdle and CloseOnInvocation. */
    CloseOnInvocationAndIdle,
    /**
     * Forcefully closes a connection that has been idle for the configured timeout period,
     * regardless of whether the connection has pending invocations or dispatch.
     */
    CloseOnIdleForceful
}

/**
 * Specifies the heartbeat semantics for Active Connection Management.
 */
local enum ACMHeartbeat
{
    /** Disables heartbeats. */
    HeartbeatOff,
    /** Send a heartbeat at regular intervals if the connection is idle and only if there are pending dispatch. */
    HeartbeatOnDispatch,
    /** Send a heartbeat at regular intervals when the connection is idle. */
    HeartbeatOnIdle,
    /** Send a heartbeat at regular intervals until the connection is closed. */
    HeartbeatAlways
}

/**
 * A collection of Active Connection Management configuration settings.
 */
local struct ACM
{
    /** A timeout value in seconds. */
    int timeout;
    /** The close semantics. */
    ACMClose close;
    /** The heartbeat semantics. */
    ACMHeartbeat heartbeat;
}

/**
 * Determines the behavior when manually closing a connection.
 **/
["cpp:scoped", "objc:scoped"]
local enum ConnectionClose
{
    /**
     * Close the connection immediately without sending a close connection protocol message to the peer
     * and waiting for the peer to acknowledge it.
     **/
    Forcefully,
    /**
     * Close the connection by notifying the peer but do not wait for pending outgoing invocations to complete.
     * On the server side, the connection will not be closed until all incoming invocations have completed.
     **/
    Gracefully,
    /**
     * Wait for all pending invocations to complete before closing the connection.
     **/
    GracefullyWithWait
}

#if !defined(__SLICE2PHP__) && !defined(__SLICE2MATLAB__)
/**
 *
 * The user-level interface to a connection.
 *
 **/
["swift:inherits:Swift.CustomStringConvertible"]
local interface Connection
{
    /**
     *
     * Manually close the connection using the specified closure mode.
     *
     * @param mode Determines how the connection will be closed.
     *
     * @see ConnectionClose
     **/
    ["cpp:noexcept", "js:async"] void close(ConnectionClose mode);

    /**
     *
     * Create a special proxy that always uses this connection. This
     * can be used for callbacks from a server to a client if the
     * server cannot directly establish a connection to the client,
     * for example because of firewalls. In this case, the server
     * would create a proxy using an already established connection
     * from the client.
     *
     * @param id The identity for which a proxy is to be created.
     *
     * @return A proxy that matches the given identity and uses this
     * connection.
     *
     * @see #setAdapter
     **/
    ["cpp:const", "swift:nonnull"] Object* createProxy(Identity id);

    /**
     *
     * Explicitly set an object adapter that dispatches requests that
     * are received over this connection. A client can invoke an
     * operation on a server using a proxy, and then set an object
     * adapter for the outgoing connection that is used by the proxy
     * in order to receive callbacks. This is useful if the server
     * cannot establish a connection back to the client, for example
     * because of firewalls.
     *
     * @param adapter The object adapter that should be used by this
     * connection to dispatch requests. The object adapter must be
     * activated. When the object adapter is deactivated, it is
     * automatically removed from the connection. Attempts to use a
     * deactivated object adapter raise {@link ObjectAdapterDeactivatedException}
     *
     * @see #createProxy
     * @see #getAdapter
     *
     **/
    void setAdapter(ObjectAdapter adapter);

    /**
     *
     * Get the object adapter that dispatches requests for this
     * connection.
     *
     * @return The object adapter that dispatches requests for the
     * connection, or null if no adapter is set.
     *
     * @see #setAdapter
     *
     **/
    ["cpp:const", "cpp:noexcept", "swift:noexcept"] ObjectAdapter getAdapter();

    /**
     *
     * Get the endpoint from which the connection was created.
     *
     * @return The endpoint from which the connection was created.
     *
     **/
    ["cpp:const", "cpp:noexcept", "swift:noexcept", "swift:nonnull"] Endpoint getEndpoint();

    /**
     *
     * Flush any pending batch requests for this connection.
     * This means all batch requests invoked on fixed proxies
     * associated with the connection.
     *
     * @param compress Specifies whether or not the queued batch requests
     * should be compressed before being sent over the wire.
     *
     **/
    ["async-oneway"] void flushBatchRequests(CompressBatch compress);

    /**
     *
     * Set a close callback on the connection. The callback is called by the
     * connection when it's closed. The callback is called from the
     * Ice thread pool associated with the connection. If the callback needs
     * more information about the closure, it can call {@link Connection#throwException}.
     *
     * @param callback The close callback object.
     *
     **/
    void setCloseCallback(CloseCallback callback);

    /**
     *
     * Set a heartbeat callback on the connection. The callback is called by the
     * connection when a heartbeat is received. The callback is called
     * from the Ice thread pool associated with the connection.
     *
     * @param callback The heartbeat callback object.
     *
     **/
    ["swift:noexcept"] void setHeartbeatCallback(HeartbeatCallback callback);

    /**
     *
     * Send a heartbeat message.
     *
     **/
    ["async-oneway"] void heartbeat();

    /**
     *
     * Set the active connection management parameters.
     *
     * @param timeout The timeout value in seconds, must be &gt;= 0.
     *
     * @param close The close condition
     *
     * @param heartbeat The hertbeat condition
     *
     **/
    ["swift:noexcept", "java:optional"]
    void setACM(optional(1) int timeout, optional(2) ACMClose close, optional(3) ACMHeartbeat heartbeat);

    /**
     *
     * Get the ACM parameters.
     *
     * @return The ACM parameters.
     *
     **/
    ["cpp:noexcept", "swift:noexcept"] ACM getACM();

    /**
     *
     * Return the connection type. This corresponds to the endpoint
     * type, i.e., "tcp", "udp", etc.
     *
     * @return The type of the connection.
     *
     **/
    ["cpp:const", "cpp:noexcept", "swift:noexcept"] string type();

    /**
     *
     * Get the timeout for the connection.
     *
     * @return The connection's timeout.
     *
     **/
    ["cpp:const", "cpp:noexcept", "swift:noexcept"] int timeout();

    /**
     *
     * Return a description of the connection as human readable text,
     * suitable for logging or error messages.
     *
     * @return The description of the connection as human readable
     * text.
     *
     **/
    ["cpp:const", "cpp:noexcept", "swift:noexcept"] string toString();

    /**
     *
     * Returns the connection information.
     *
     * @return The connection information.
     *
     **/
    ["cpp:const", "swift:nonnull"] ConnectionInfo getInfo();

    /**
     *
     * Set the connection buffer receive/send size.
     *
     * @param rcvSize The connection receive buffer size.
     * @param sndSize The connection send buffer size.
     *
     **/
    void setBufferSize(int rcvSize, int sndSize);

    /**
     *
     * Throw an exception indicating the reason for connection closure. For example,
     * {@link CloseConnectionException} is raised if the connection was closed gracefully,
     * whereas {@link ConnectionManuallyClosedException} is raised if the connection was
     * manually closed by the application. This operation does nothing if the connection is
     * not yet closed.
     *
     **/
    ["cpp:const"] void throwException();
}

/**
 *
 * Provides access to the connection details of an IP connection
 *
 **/
local class IPConnectionInfo extends ConnectionInfo
{
    /** The local address. */
    string localAddress = "";

    /** The local port. */
    int localPort = -1;

    /** The remote address. */
    string remoteAddress = "";

    /** The remote port. */
    int remotePort = -1;
}

/**
 *
 * Provides access to the connection details of a TCP connection
 *
 **/
local class TCPConnectionInfo extends IPConnectionInfo
{
    /**
     *
     * The connection buffer receive size.
     *
     **/
    int rcvSize = 0;

    /**
     *
     * The connection buffer send size.
     *
     **/
    int sndSize = 0;
}

/**
 *
 * Provides access to the connection details of a UDP connection
 *
 **/
local class UDPConnectionInfo extends IPConnectionInfo
{
    /**
     *
     * The multicast address.
     *
     **/
    string mcastAddress;

    /**
     *
     * The multicast port.
     *
     **/
    int mcastPort = -1;

    /**
     *
     * The connection buffer receive size.
     *
     **/
    int rcvSize = 0;

    /**
     *
     * The connection buffer send size.
     *
     **/
    int sndSize = 0;
}

/** A collection of HTTP headers. */
dictionary<string, string> HeaderDict;

/**
 *
 * Provides access to the connection details of a WebSocket connection
 *
 **/
local class WSConnectionInfo extends ConnectionInfo
{
    /** The headers from the HTTP upgrade request. */
    HeaderDict headers;
}

#endif

}
