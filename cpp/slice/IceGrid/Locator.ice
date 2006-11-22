// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
 * The IceGrid locator interface extends the [Ice::Locator] interface
 * to provide access to the IceGrid query and registry object of the
 * IceGrid registry.
 *
 * @see Registry
 * @see Query
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
