// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_TRANSPORT_INFO_ICE
#define ICE_TRANSPORT_INFO_ICE

#include <Ice/Identity.ice>

module Ice
{

/**
 *
 * The user-level interface to a connection.
 *
 **/
local interface TransportInfo
{
    /**
     *
     * Flush any pending batch requests for this connection. This
     * causes all batch requests that were sent via proxies that use
     * this connection to be sent to the server.
     *
     **/
    void flushBatchRequests();

    /**
     *
     * Create a proxy that always uses this connection. This is
     * typically used for reverse communications using connections
     * that have been established from a client to an object adapter.
     *
     * <note><para> This operation is intended to be used by special
     * services, such as [Router] implementations. Regular user code
     * should not attempt to use this operation. </para></note>
     *
     * @param id The identity for which a proxy is to be created.
     *
     * @return A proxy that matches the given identity and uses this
     * connection.
     *
     * @see Identity
     *
     **/
    nonmutating Object* createProxy(Identity id);

    /**
     *
     * Return the connection type. This corresponds to the endpoint
     * type, i.e., "tcp", "udp", etc.
     *
     * @return The type of the connection.
     *
     **/
    nonmutating string type();

    /**
     *
     * Return a description of the connection as human readable text,
     * suitable for logging or error messages.
     *
     * @return The description of the connection as human readable
     * text.
     *
     **/
    nonmutating string toString();
};

};

#endif
