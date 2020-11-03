// Copyright (c) ZeroC, Inc. All rights reserved.

#pragma once

#ifdef __SLICE2CS__ // C# only for now

[[cpp:doxygen:include(Ice/LocatorDiscovery.h)]]
[[cpp:header-ext(h)]]

[[suppress-warning(reserved-identifier)]]
[[js:module(ice)]]

[[python:pkgdir(IceLocatorDiscovery)]]

#include <Ice/Locator.ice>

/// Ice Locator Discovery is built-in {@see Ice::Locator} implementation that discovers IceGrid or custom locators via
/// UDP multicast. You enable this built-in locator implementation by setting the configuration property
/// Ice.Default.Locator to LocatorDiscovery.
[cs:namespace(ZeroC)]
[java:package(com.zeroc)]
module Ice::LocatorDiscovery
{
    /// The Ice.LocatorDiscovery.Reply object adapter of a client application hosts a LookupReply object that processes
    /// replies to locator discovery requests.
    interface LookupReply
    {
        /// Provides a locator proxy in response to a findLocator call on a Lookup object.
        /// @param proxy The proxy to the locator object.
        [oneway] void foundLocator(Ice::Locator proxy);
    }

    /// A locator implementation such as the IceGrid registry hosts a Lookup object that receives discovery requests
    /// from clients. This Lookup object is a well-known object with identity `IceLocatorDiscovery/Lookup'.
    interface Lookup
    {
        /// Finds a locator with the given instance name.
        /// @param instanceName Restricts the search to locator implementations configured with the given instance name.
        /// If empty, all available locator implementations will reply.
        /// @param reply A proxy to the client's LookupReply object. The locator implementation calls foundLocator on
        /// this object.
        [oneway] idempotent void findLocator(string instanceName, LookupReply reply);
    }
}

#endif
