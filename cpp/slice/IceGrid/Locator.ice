// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_LOCATOR_ICE
#define ICE_GRID_LOCATOR_ICE

#include <Ice/Locator.ice>

module IceGrid
{

interface Registry;
interface Query;

/**
 *
 * The IceGrid locator interface provides access to the [Query]
 * and [Registry] object of the IceGrid registry.
 *
 * @see Query
 * @see Registry
 * 
 **/
interface Locator extends Ice::Locator
{
    /**
     *
     * Get the proxy of the registry object hosted by this IceGrid
     * registry.
     *
     * @return The proxy of the registry object.
     *
     **/
    ["cpp:const"] idempotent Registry* getLocalRegistry();

    /**
     *
     * Get the proxy of the query object hosted by this IceGrid
     * registry.
     *
     * @return The proxy of the query object.
     *
     **/
    ["cpp:const"] idempotent Query* getLocalQuery();
};

};

#endif
