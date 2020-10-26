// Copyright (c) ZeroC, Inc. All rights reserved.

#pragma once

// TODO: these definitions moved to Ice/Discovery.ice. Remove this file when all languages mappings are updated.

[[cpp:doxygen:include(IceDiscovery/IceDiscovery.h)]]
[[cpp:header-ext(h)]]

[[suppress-warning(reserved-identifier)]]
[[js:module(ice)]]

[[python:pkgdir(IceDiscovery)]]

#include <Ice/Identity.ice>

/// The IceDiscovery plug-in implements the {@link Ice::Locator} interface to locate (or discover) objects and object
/// adapters using UDP multicast. It also implements the {@link Ice::LocatorDiscovery} interface to allow servers to
/// respond to such multicast discovery requests.
[cs:namespace(ZeroC)]
[java:package(com.zeroc)]
module IceDiscovery
{
    interface LookupReply;

    /// The IceDiscovery.Multicast object adapter of a server application hosts a Lookup object that receives discovery
    /// requests from Discovery clients.
    interface Lookup
    {
        /// Finds an ice1 object adapter hosted by the target object's server.
        /// @param domainId The IceDiscovery domain ID. An IceDiscovery server only replies to requests that include a
        /// domain ID that matches the server's configured domain ID.
        /// @param id The adapter ID.
        /// @param reply A proxy to a LookupReply object created by the caller. The server calls foundAdapterById on
        /// this object when it hosts an ice1 object adapter that has the requested adapter ID (or replica group ID).
        idempotent void findAdapterById(string domainId, string id, LookupReply reply);

        /// Finds an object hosted by an ice1 object adapter of the target object's server
        /// @param domainId The IceDiscovery domain ID. An IceDiscovery server only replies to requests that include a
        /// domain ID that matches the server's configured domain ID.
        /// @param id The object identity.
        /// @param reply A proxy to a LookupReply object created by the caller. The server calls foundObjectById on this
        /// object when it hosts an object with the requested identity in an ice1 object adapter.
        idempotent void findObjectById(string domainId, Ice::Identity id, LookupReply reply);
    }

    /// Handles the reply or replies to findAdapterById and findObjectById calls on {@link Lookup}.
    interface LookupReply
    {
        /// Provides the endpoints for an object adapter in response to a findAdapterById call on a Lookup object.
        /// @param id The adapter or replica group ID, as specified in the findAdapterById call.
        /// @param proxy A dummy proxy that carries the endpoints of the object adapter.
        /// @param isReplicaGroup True if `id` corresponds to a replica group ID and false otherwise.
        void foundAdapterById(string id, Object proxy, bool isReplicaGroup);

        /// Provides the adapter ID or endpoints for an object in response to a findObjectById call on a Lookup object.
        /// @param id The identity of the object, as specified in the findObjectById call.
        /// @param proxy A dummy proxy that carries the adapter ID or endpoints for the well-known object.
        void foundObjectById(Ice::Identity id, Object proxy);
    }
}
