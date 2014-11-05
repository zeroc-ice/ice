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
[["cpp:include:IceGrid/Config.h"]]

#include <IceGrid/Locator.ice>

module IceGrid
{

/**
 *
 * The IceGrid lookup reply interface provides must be implemented by
 * clients which are searching for IceGrid locators. IceGrid
 * registries invoke on this interface to provide their locator proxy.
 *
 * @see Locator
 * @see Lookup
 * 
 **/
interface LookupReply
{
    /**
     *
     * This method is called by the implementation of the Lookup
     * interface to reply to a findLocator request.
     *
     * @param prx The proxy of the locator.
     *
     **/
    void foundLocator(Locator* prx);
};

/**
 *
 * The IceGrid lookup interface is implemented by IceGrid registries
 * and can be used by clients to find available IceGrid locators on
 * the network. 
 * 
 * IceGrid registries provide a well-known `IceGrid/Lookup' object
 * accessible through UDP multicast. Clients typically make a
 * multicast findLocator request to find the locator proxy.
 *
 * @see Locator
 * @see LookupReply
 * 
 **/
interface Lookup
{
    /**
     *
     * Find a locator proxy with the given instance name.
     *
     * @param instanceName Restrict the search to IceGrid registries
     * configured with the given instance name. If empty, all the
     * available registries will reply.
     *
     * @param reply The reply object to use to send the reply.
     *
     **/
    idempotent void findLocator(string instanceName, LookupReply* reply);
};

};
