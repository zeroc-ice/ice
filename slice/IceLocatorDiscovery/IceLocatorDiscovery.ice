//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["cpp:doxygen:include:IceLocatorDiscovery/IceLocatorDiscovery.h"]]
[["cpp:header-ext:h"]]

[["ice-prefix"]]

[["js:module:ice"]]

[["python:pkgdir:IceLocatorDiscovery"]]

#include <Ice/Locator.ice>

#ifndef __SLICE2JAVA_COMPAT__
[["java:package:com.zeroc"]]
#endif

/**
 * IceLocatorDiscovery is an Ice plug-in that enables the discovery of IceGrid and custom locators via
 * UDP multicast.
 **/
module IceLocatorDiscovery
{

/**
 *
 * The Ice lookup reply interface must be implemented by clients which
 * are searching for Ice locators. Ice locator implementations invoke
 * on this interface to provide their locator proxy.
 *
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
    void foundLocator(Ice::Locator* prx);
}

/**
 *
 * The Ice lookup interface is implemented by Ice locator
 * implementations and can be used by clients to find available Ice
 * locators on the network.
 *
 * Ice locator implementations provide a well-known `Ice/LocatorLookup'
 * object accessible through UDP multicast. Clients typically make a
 * multicast findLocator request to find the locator proxy.
 *
 * @see LookupReply
 *
 **/
interface Lookup
{
    /**
     *
     * Find a locator proxy with the given instance name.
     *
     * @param instanceName Restrict the search to Ice registries
     * configured with the given instance name. If empty, all the
     * available registries will reply.
     *
     * @param reply The reply object to use to send the reply.
     *
     **/
    idempotent void findLocator(string instanceName, LookupReply* reply);
}

}
