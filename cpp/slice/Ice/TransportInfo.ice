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
