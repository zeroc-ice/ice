// Copyright (c) ZeroC, Inc. All rights reserved.

#pragma once

[[cpp:doxygen:include(IceDiscovery/IceDiscovery.h)]]
[[cpp:header-ext(h)]]

[[suppress-warning(reserved-identifier)]]
[[js:module(ice)]]

[[python:pkgdir(IceDiscovery)]]

#ifdef __SLICE2CS__
#include <Ice/Endpoint.ice>
#endif

#include <Ice/Identity.ice>

/// The IceDiscovery plug-in implements the {@see Ice::Locator} interface to locate (or discover) objects and object
/// adapters using UDP multicast. It also implements the {@see Ice::LocatorDiscovery} interface to allow servers to
/// respond to such multicast discovery requests.
[cs:namespace(ZeroC)]
[java:package(com.zeroc)]
module IceDiscovery
{
    interface LookupReply;

#ifdef __SLICE2CS__
    interface ResolveAdapterIdReply;
    interface ResolveWellKnownProxyReply;
#endif

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

#ifdef __SLICE2CS__
        /// Finds an ice2 object adapter hosted by the target object's server.
        /// @param domainId The IceDiscovery domain ID. An IceDiscovery server only replies to requests that include a
        /// domain ID that matches the server's configured domain ID.
        /// @param adapterId The adapter ID.
        /// @param reply A proxy to a ResolveAdapterIdReply object created by the caller. The server calls found
        /// on this object when it hosts an ice2 object adapter that has the requested adapter ID (or replica group ID).
        void resolveAdapterId(string domainId, string adapterId, ResolveAdapterIdReply reply);

        /// Finds an object hosted by an ice2 object adapter of the target object's server.
        /// @param domainId The IceDiscovery domain ID. An IceDiscovery server only replies to requests that include a
        /// domain ID that matches the server's configured domain ID.
        /// @param identity The identity of the object.
        /// @param reply A proxy to a ResolvedWellKnownProxyReply object created by the caller. The server calls
        /// foundAdapterId or foundEndpoints on this object when it has an ice2 object adapter that hosts an object with
        /// the given identity.
        void resolveWellKnownProxy(string domainId, Ice::Identity identity, ResolveWellKnownProxyReply reply);
#endif
    }

    /// Handles the reply or replies to findAdapterById and findObjectById calls on {@see Lookup}.
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

#ifdef __SLICE2CS__
    /// Handles the reply or replies to resolveAdapterId calls on {@see Lookup}.
    interface ResolveAdapterIdReply
    {
        /// Provides the endpoints for an object adapter in response to a resolveAdapterId call on a Lookup object.
        /// @param endpoints A sequence of endpoints. Cannot be empty.
        /// @param isReplicaGroup True if the adapter ID provided to the resolveAdapterId call corresponds to a replica
        /// group ID and false otherwise.
        void foundAdapterId(Ice::EndpointDataSeq endpoints, bool isReplicaGroup);
    }

    /// Handles the reply or replies to resolveWellKnownProxy calls on {@see Lookup}.
    interface ResolveWellKnownProxyReply
    {
        /// Provides the adapter ID or replica group ID for an object adapter that hosts the desired well-known object,
        /// in response to a resolveWellKnownProxy call on a Lookup object.
        /// @param adapterId The adapter ID or replica group ID of the object adapter that hosts the object.
        void foundWellKnownProxy(string adapterId);
    }
#endif
}
