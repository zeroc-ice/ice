// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:header-ext:h"]]

[["js:module:@zeroc/ice"]]
[["python:pkgdir:IceLocatorDiscovery"]]

#include "Ice/Locator.ice"

/// IceLocatorDiscovery is an Ice plug-in that enables the discovery of Ice locators (such as IceGrid) via UDP
/// multicast.
["java:identifier:com.zeroc.IceLocatorDiscovery"]
module IceLocatorDiscovery
{
    /// Represents a callback object implemented by IceLocatorDiscovery clients. It allows IceLocatorDiscovery clients
    /// to receive replies to Lookup requests.
    /// @see Lookup
    interface LookupReply
    {
        /// Provides a reply to a {@link Lookup#findLocator} request.
        /// @param prx The proxy of the locator.
        void foundLocator(Ice::Locator* prx);
    }

    /// Looks for a locator using UDP multicast.
    /// @remark This interface is implemented by Ice locator implementations and can be used by clients to find
    /// available Ice locators on the network.
    /// Ice locator implementations provide a well-known 'Ice/LocatorLookup' object accessible through UDP multicast.
    /// Clients typically make a multicast findLocator request to find the locator proxy.
    /// @see LookupReply
    interface Lookup
    {
        /// Finds a locator proxy with the given instance name.
        /// @param instanceName Restrict the search to Ice registries configured with the given instance name. If
        /// empty, all the available registries will reply.
        /// @param reply The reply object to use to send the reply.
        idempotent void findLocator(string instanceName, LookupReply* reply);
    }
}
