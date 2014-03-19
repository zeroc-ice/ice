
// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h"]]

#include <Ice/ObjectAdapterF.ice>
#include <Ice/Identity.ice>
#include <Ice/Endpoint.ice>

module Ice
{

/**
 *
 * Base class providing access to the connection details.
 *
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
     * Flush any pending batch requests for this connection. This
     * causes all batch requests that were sent via proxies that use
     * this connection to be sent to the server.
     *
     **/
    ["async"] void flushBatchRequests();

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

};

