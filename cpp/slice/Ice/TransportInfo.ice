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
 * The base interface for all transport info objects.
 *
 **/
local interface TransportInfo
{
    /**
     *
     * Return the transport type. This corresponds to the endpoint
     * type, i.e., "tcp", "udp", etc.
     *
     * @return The type of the transport.
     *
     **/
    nonmutating string type();

    /**
     *
     * Return a description of the transport as human readable text,
     * suitable for logging or error messages.
     *
     * @return The description of the transport as human readable
     * text.
     *
     **/
    nonmutating string toString();
};

};

#endif
