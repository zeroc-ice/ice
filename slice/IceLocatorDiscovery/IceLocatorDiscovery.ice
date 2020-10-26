// Copyright (c) ZeroC, Inc. All rights reserved.

#pragma once

// TODO: these definitions moved to Ice/LocatorDiscovery.ice. Remove this file when all languages mappings are updated.

[[cpp:doxygen:include(IceLocatorDiscovery/IceLocatorDiscovery.h)]]
[[cpp:header-ext(h)]]

[[suppress-warning(reserved-identifier)]]
[[js:module(ice)]]

[[python:pkgdir(IceLocatorDiscovery)]]

#include <Ice/Locator.ice>

/// LocatorDiscovery is an Ice plug-in that enables the discovery of IceGrid and custom locators via UDP multicast.
/// This plug-in is usually named IceLocatorDiscovery in Ice configuration. The LocatorDiscovery plug-in implements the
/// {@link Ice::Locator} interface to locate (or discover) locators such as the IceGrid registry or custom IceGrid-like
/// locator implementations using UDP multicast.
[cs:namespace(ZeroC)]
[java:package(com.zeroc)]
module IceLocatorDiscovery
{
    /// The {plug-in name}.Reply object adapter of a client application hosts a LookupReply object that processes
    /// replies to locator discovery requests.
    interface LookupReply
    {
        /// Provides a locator proxy in response to a findLocator call on a Lookup object.
        /// @param proxy The proxy to the locator object.
        void foundLocator(Ice::Locator proxy);
    }

    /// A locator implementation such as the IceGrid registry hosts a Lookup object that receives discovery requests
    /// from clients. This Lookup object is a well-known object with identity `IceLocatorDiscovery/Lookup'.
    interface Lookup
    {
        /// Finds a locator with the given instance name.
        /// @param instanceName Restrict the search to locator implementations configured with the given instance name.
        /// If empty, all available locator implementations will reply.
        /// @param reply A proxy to the client's LookupReply object. The locator implementation calls foundLocator on
        /// this object.
        idempotent void findLocator(string instanceName, LookupReply reply);
    }
}
