// Copyright (c) ZeroC, Inc. All rights reserved.

#pragma once

#ifdef __SLICE2CS__

[[cpp:doxygen:include(Ice/Discovery.h)]]
[[cpp:header-ext(h)]]

[[suppress-warning(reserved-identifier)]]
[[js:module(ice)]]

[[python:pkgdir(IceDiscovery)]]

#include <Ice/Endpoint.ice>
#include <Ice/Identity.ice>

/// Ice Discovery is a built-in {@see Ice::Locator} implementation that locates (or discovers) objects and object
/// adapters using UDP multicast. You enable this built-in locator implementation by setting the configuration property
/// Ice.Default.Locator to Discovery.
/// Ice Discovery also implements the {@see Ice::LocatorDiscovery} interface to allow servers to respond to such
/// multicast discovery requests.
[cs:namespace(ZeroC)]
[java:package(com.zeroc)]
module Ice::Discovery
{
    interface FindAdapterByIdReply;
    interface FindObjectByIdReply;
    interface ResolveAdapterIdReply;
    interface ResolveWellKnownProxyReply;

    /// The Ice.Discovery.Multicast object adapter of a server application hosts a Lookup object that receives discovery
    /// requests from Discovery clients.
    interface Lookup
    {
        /// Finds an ice1 object adapter hosted by the target object's server.
        /// @param domainId The Discovery domain ID. An Discovery server only replies to requests that include a domain
        /// ID that matches the server's configured domain ID.
        /// @param id The adapter ID.
        /// @param reply A proxy to a FindAdapterByIdReply object created by the caller. The server calls
        /// foundAdapterById on this object when it hosts an ice1 object adapter that has the requested adapter ID (or
        /// replica group ID).
        [oneway] idempotent void findAdapterById(string domainId, string id, FindAdapterByIdReply reply);

        /// Finds an object hosted by an ice1 object adapter of the target object's server
        /// @param domainId The Discovery domain ID. An Discovery server only replies to requests that include a domain
        /// ID that matches the server's configured domain ID.
        /// @param id The object identity.
        /// @param facet The facet of the object. Null is equivalent to the empty string.
        /// @param reply A proxy to a FindObjectByIdReply object created by the caller. The server calls foundObjectById
        /// on this object when it hosts an object with the requested identity and facet in an ice1 object adapter.
        [oneway] idempotent void findObjectById(
            string domainId,
            Ice::Identity id,
            tag(1) string? facet,
            FindObjectByIdReply reply);

        /// Finds an ice2 object adapter hosted by the target object's server.
        /// @param domainId The Discovery domain ID. An Discovery server only replies to requests that include a domain
        /// ID that matches the server's configured domain ID.
        /// @param adapterId The adapter ID.
        /// @param reply A proxy to a ResolveAdapterIdReply object created by the caller. The server calls
        /// foundAdapterId on this object when it hosts an ice2 object adapter that has the requested adapter ID (or
        /// replica group ID).
        [oneway] void resolveAdapterId(string domainId, string adapterId, ResolveAdapterIdReply reply);

        /// Finds an object hosted by an ice2 object adapter of the target object's server.
        /// @param domainId The Discovery domain ID. An Discovery server only replies to requests that include a domain
        /// ID that matches the server's configured domain ID.
        /// @param identity The identity of the object.
        /// @param facet The facet of the object.
        /// @param reply A proxy to a ResolvedWellKnownProxyReply object created by the caller. The server calls
        /// foundWellKnownProxy on this object when it has an ice2 object adapter that hosts an object with the given
        /// identity and facet.
        [oneway] void resolveWellKnownProxy(
            string domainId,
            Ice::Identity identity,
            string facet,
            ResolveWellKnownProxyReply reply);
    }

    /// Handles the reply or replies to findAdapterById calls on {@see Lookup}.
    // Note: for compatibility with Ice 3.7, the operation name (and its parameters) must remain unchanged.
    interface FindAdapterByIdReply
    {
        /// Provides the endpoints for an object adapter in response to a findAdapterById call on a Lookup object.
        /// @param id The adapter or replica group ID, as specified in the findAdapterById call.
        /// @param proxy A dummy proxy that carries the endpoints of the object adapter.
        /// @param isReplicaGroup True if `id` corresponds to a replica group ID and false otherwise.
        [oneway] void foundAdapterById(string id, Object proxy, bool isReplicaGroup);
    }

    /// Handles the reply or replies to findObjectById calls on {@see Lookup}.
    // Note: for compatibility with Ice 3.7, the operation name (and its parameters) must remain unchanged.
    interface FindObjectByIdReply
    {
        /// Provides the adapter ID or endpoints for an object in response to a findObjectById call on a Lookup object.
        /// @param id The identity of the object, as specified in the findObjectById call.
        /// @param proxy A dummy proxy that carries the adapter ID or endpoints for the well-known object.
        [oneway] void foundObjectById(Ice::Identity id, Object proxy);
    }

    /// Handles the reply or replies to resolveAdapterId calls on {@see Lookup}.
    interface ResolveAdapterIdReply
    {
        /// Provides the endpoints for an object adapter in response to a resolveAdapterId call on a Lookup object.
        /// @param endpoints A sequence of endpoints. Cannot be empty.
        /// @param isReplicaGroup True if the adapter ID provided to the resolveAdapterId call corresponds to a replica
        /// group ID and false otherwise.
        [oneway] void foundAdapterId(Ice::EndpointDataSeq endpoints, bool isReplicaGroup);
    }

    /// Handles the reply or replies to resolveWellKnownProxy calls on {@see Lookup}.
    interface ResolveWellKnownProxyReply
    {
        /// Provides the adapter ID or replica group ID for an object adapter that hosts the desired well-known object,
        /// in response to a resolveWellKnownProxy call on a Lookup object.
        /// @param adapterId The adapter ID or replica group ID of the object adapter that hosts the object.
        [oneway] void foundWellKnownProxy(string adapterId);
    }
}

#endif
