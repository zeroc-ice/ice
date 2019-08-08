
// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h", "objc:header-dir:objc", "js:ice-build"]]

#include <Ice/ObjectAdapterF.ice>
#include <Ice/Identity.ice>
#include <Ice/Endpoint.ice>

["objc:prefix:ICE"]
module Ice
{

/**
 *
 * Base class providing access to the connection details. *
 **/
local class ConnectionInfo
{
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

    /**
     *
     * The connection buffer receive size.
     *
     **/
    int rcvSize;

    /**
     *
     * The connection buffer send size.
     *
     **/
    int sndSize;
};

local interface Connection;

/**
 *
 * An application can implement this interface to receive notifications when
 * a connection closes or receives a heartbeat message.
 *
 * @see Connection#setCallback
 *
 **/
local interface ConnectionCallback
{
    /**
     *
     * This method is called by the the connection when a heartbeat is
     * received from the peer.
     *
     * @param con The connection on which a heartbeat was received.
     *
     **/
    void heartbeat(Connection con);

    /**
     *
     * This method is called by the the connection when the connection
     * is closed.
     *
     * @param con The connection that closed.
     *
     **/
    void closed(Connection con);
};

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
};

/**
 * Specifies the heartbeat semantics for Active Connection Management.
 */
local enum ACMHeartbeat
{
    /** Disables heartbeats. */
    HeartbeatOff,
    /** Send a heartbeat at regular intervals if the connection is idle and only if there are pending dispatch. */
    HeartbeatOnInvocation,
    /** Send a heartbeat at regular intervals when the connection is idle. */
    HeartbeatOnIdle,
    /** Send a heartbeat at regular intervals until the connection is closed. */
    HeartbeatAlways
};

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
};

/**
 *
 * The user-level interface to a connection.
 *
 **/
local interface Connection
{
    /**
     *
     * Close a connection, either gracefully or forcefully. If a
     * connection is closed forcefully, it closes immediately, without
     * sending the relevant close connection protocol messages to the
     * peer and waiting for the peer to acknowledge these protocol
     * messages.
     *
     * @param force If true, close forcefully. Otherwise the
     * connection is closed gracefully.
     *
     **/
    void close(bool force);

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
     **/
    ["async"] void flushBatchRequests();

    /**
     *
     * Set callback on the connection. The callback is called by the
     * connection when it's closed. The callback is called from the
     * Ice thread pool associated with the connection.
     *
     * @param callback The connection callback object.
     *
     **/
    void setCallback(ConnectionCallback callback);

    /**
     *
     * Set the active connection management parameters.
     *
     * @param timeout The timeout value in seconds.
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
     * Set the connectiion buffer receive/send size.
     *
     * @param rcvSize The connection receive buffer size.
     * @param sndSize The connection send buffer size.
     *
     **/
    void setBufferSize(int rcvSize, int sndSize);
};

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
};

/**
 *
 * Provides access to the connection details of a TCP connection
 *
 **/
local class TCPConnectionInfo extends IPConnectionInfo
{
};

/**
 *
 * Provides access to the connection details of a UDP connection
 *
 **/
local class UDPConnectionInfo extends IPConnectionInfo
{
    /** The multicast address. */
    string mcastAddress;

    /** The multicast port. */
    int mcastPort = -1;
};

/** A collection of HTTP headers. */
dictionary<string, string> HeaderDict;

/**
 *
 * Provides access to the connection details of a WebSocket connection
 *
 **/
local class WSConnectionInfo extends TCPConnectionInfo
{
    /** The headers from the HTTP upgrade request. */
    HeaderDict headers;
};

};
