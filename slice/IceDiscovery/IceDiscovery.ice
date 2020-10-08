//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[cpp:doxygen:include:IceDiscovery/IceDiscovery.h]]
[[cpp:header-ext:h]]

[[suppress-warning:reserved-identifier]]
[[js:module:ice]]

[[python:pkgdir:IceDiscovery]]

#include <Ice/Identity.ice>

[[java:package:com.zeroc]]
[cs:namespace:ZeroC]
/// The IceDiscovery plug-in implements the {@see Ice.Locator} interface to locate (or discover) objects and object
/// adapters using UDP multicast. It also implements the {@see Ice.LocatorDiscovery} interface to allow servers to
/// respond to such multicast discovery requests.
module IceDiscovery
{
    /// The IceDiscovery.Reply object adapter of a client application hosts a LookupReply object that processes
    /// replies to discovery requests.
    interface LookupReply
    {
        /// Provides the adapter ID or endpoints for an object in response to a {@see Lookup.FindObjectById}
        /// call on a Lookup object.
        /// @param id The identity of the object, as specified in the findObjectById call.
        /// @param proxy A dummy proxy that carries the adapter ID or endpoints for the well-known object.
        void foundObjectById(Ice::Identity id, Object proxy);

        /// Provides the endpoints for an object adapter in response to a {@see Lookup.FindAdapterById} call on a
        /// Lookup object.
        /// @param id The adapter or replica group ID, as specified in the findAdapterById call.
        /// @param proxy A dummy proxy that carries the endpoints of the object adapter.
        /// @param isReplicaGroup True if `id` corresponds to a replica group ID and false otherwise.
        void foundAdapterById(string id, Object proxy, bool isReplicaGroup);
    }

    /// The IceDiscovery.Multicast object adapter of a server application hosts a Lookup object that receives discovery
    /// requests from clients.
    interface Lookup
    {
        /// Finds an object hosted by this server.
        /// @param domainId The IceDiscovery domain ID. An IceDiscovery server only replies to requests that include
        /// a domain ID that matches the server's configured domain ID.
        /// @param id The object identity.
        /// @param reply A proxy to the client's LookupReply object. The server calls
        /// {@see LookupReply.FoundObjectById} on this object when it hosts an object with identity `id`, provided the
        /// domain IDs match.
        idempotent void findObjectById(string domainId, Ice::Identity id, LookupReply reply);

        /// Finds an object adapter hosted by this server.
        /// @param domainId The IceDiscovery domain ID. An IceDiscovery server only replies to requests that include
        /// a domain ID that matches the server's configured domain ID.
        /// @param id The adapter ID.
        /// @param reply A proxy to the client's LookupReply object. The server calls
        /// {@see LookupReply.FoundAdapterById} on this object when it hosts an object adapter with adapter ID `id`,
        /// provided the domain IDs match.
        idempotent void findAdapterById(string domainId, string id, LookupReply reply);
    }
}
