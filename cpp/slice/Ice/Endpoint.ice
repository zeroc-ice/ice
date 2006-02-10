// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_ENDPOINT_ICE
#define ICE_ENDPOINT_ICE

module Ice
{

enum EndpointSelectionType
{
    Random,
    Ordered
};

/**
 *
 * The user-level interface to an endpoint.
 *
 **/
local interface Endpoint
{
    /**
     *
     * Return a string representation of the endpoint.
     *
     * @return The string representation of the endpoint.
     *
     **/
    nonmutating string toString();
};
local sequence<Endpoint> EndpointSeq;

};

#endif
