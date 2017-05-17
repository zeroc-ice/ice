// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["ice-prefix", "cpp:header-ext:h", "cpp:dll-export:ICE_API", "objc:header-dir:objc", "objc:dll-export:ICE_API", "js:ice-build"]]

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

/**
 *
 * Base class providing access to the connection details. *
 **/
["php:internal"]
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
    void closed(Connection con);
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
    void heartbeat(Connection con);
}

local enum ACMClose
{
    CloseOff,
    CloseOnIdle,
    CloseOnInvocation,
    CloseOnInvocationAndIdle,
    CloseOnIdleForceful
}

local enum ACMHeartbeat
{
    HeartbeatOff,
    HeartbeatOnDispatch,
    HeartbeatOnIdle,
    HeartbeatAlways
}

local struct ACM
{
    int timeout;
    ACMClose close;
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

/**
 *
 * The user-level interface to a connection.
 *
 **/
["php:internal"]
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
    void close(ConnectionClose mode);

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
    ["cpp:const"] Object* createProxy(Identity id);

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
     * automatically removed from the connection.
     *
     * @see #createProxy
     * @see #setAdapter
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
    ["cpp:const"] ObjectAdapter getAdapter();

    /**
     *
     * Get the endpoint from which the connection was created.
     *
     * @return The endpoint from which the connection was created.
     *
     **/
    ["cpp:const"] Endpoint getEndpoint();

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
    void setHeartbeatCallback(HeartbeatCallback callback);

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
     * @param timeout The timeout value in milliseconds.
     *
     * @param close The close condition
     *
     * @param heartbeat The hertbeat condition
     *
     **/
    ["java:optional"]
    void setACM(optional(1) int timeout, optional(2) ACMClose close, optional(3) ACMHeartbeat heartbeat);

    /**
     *
     * Get the ACM parameters.
     *
     * @return The ACM parameters.
     *
     **/
    ACM getACM();

    /**
     *
     * Return the connection type. This corresponds to the endpoint
     * type, i.e., "tcp", "udp", etc.
     *
     * @return The type of the connection.
     *
     **/
    ["cpp:const"] string type();

    /**
     *
     * Get the timeout for the connection.
     *
     * @return The connection's timeout.
     *
     **/
    ["cpp:const"] int timeout();

    /**
     *
     * Return a description of the connection as human readable text,
     * suitable for logging or error messages.
     *
     * @return The description of the connection as human readable
     * text.
     *
     **/
    ["cpp:const"] string toString();

    /**
     *
     * Returns the connection information.
     *
     * @return The connection information.
     *
     **/
    ["cpp:const"] ConnectionInfo getInfo();

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
["php:internal"]
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
["php:internal"]
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
["php:internal"]
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

dictionary<string, string> HeaderDict;

/**
 *
 * Provides access to the connection details of a WebSocket connection
 *
 **/
["php:internal"]
local class WSConnectionInfo extends ConnectionInfo
{
    /** The headers from the HTTP upgrade request. */
    HeaderDict headers;
}

}
