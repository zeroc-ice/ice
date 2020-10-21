// Copyright (c) ZeroC, Inc. All rights reserved.

#pragma once

[[cpp:doxygen:include(IceDiscovery/IceDiscovery.h)]]
[[cpp:header-ext:h]]

[[suppress-warning(reserved-identifier)]]
[[js:module(ice)]]

[[python:pkgdir(IceDiscovery)]]

#include <Ice/Identity.ice>
#include <Ice/Protocol.ice>

/// The IceDiscovery plug-in implements the {@see Ice::Locator} interface to locate (or discover) objects and object
/// adapters using UDP multicast. It also implements the {@see Ice::LocatorDiscovery} interface to allow servers to
/// respond to such multicast discovery requests.
[cs:namespace(ZeroC)]
[java:package(com.zeroc)]
module IceDiscovery
{
    /// The IceDiscovery.Reply object adapter of a client application hosts a LookupReply object that processes
    /// replies to discovery requests.
    interface LookupReply
    {
        /// Provides the adapter ID or endpoints for an object in response to a findObjectById call on a Lookup object.
        /// @param id The identity of the object, as specified in the findObjectById call.
        /// @param proxy A dummy proxy that carries the adapter ID or endpoints for the well-known object.
        void foundObjectById(Ice::Identity id, Object proxy);

        /// Provides the endpoints for an object adapter in response to a findAdapterById} call on a Lookup object.
        /// @param id The adapter or replica group ID, as specified in the findAdapterById call.
        /// @param proxy A dummy proxy that carries the endpoints of the object adapter.
        /// @param isReplicaGroup True if `id` corresponds to a replica group ID and false otherwise.
        void foundAdapterById(string id, Object proxy, bool isReplicaGroup);
    }

    /// The IceDiscovery.Multicast object adapter of a server application hosts a Lookup object that receives discovery
    /// requests from clients.
    interface Lookup
    {
#ifdef __SLICE2CS__
        /// Finds an object adapter hosted by the target object's server.
        /// @param domainId The IceDiscovery domain ID. An IceDiscovery server only replies to requests that include a
        /// domain ID that matches the server's configured domain ID.
        /// @param id The adapter ID.
        /// @param reply A proxy to the caller's LookupReply object. The server calls foundAdapterById on this object
        /// when it hosts an object adapter that has the requested adapter ID (or replica group ID) and this object
        /// adapter uses the specified protocol.
        /// @param protocol The protocol of the object adapter (optional). Not set is equivalent to Protocol::Ice1.
        idempotent void findAdapterById(
            string domainId,
            string id,
            LookupReply reply,
            tag(1) Ice::Protocol? protocol);

        /// Finds an object hosted by the target object's server.
        /// @param domainId The IceDiscovery domain ID. An IceDiscovery server only replies to requests that include a
        /// domain ID that matches the server's configured domain ID.
        /// @param id The object identity.
        /// @param reply A proxy to the caller's LookupReply object. The server calls foundObjectById on this object
        /// when it hosts an object with the requested identity in an object adapter that uses the specified protocol.
        /// @param protocol The protocol of the object adapter hosting the desired object (optional). Not set is
        /// equivalent to Protocol::Ice1.
        idempotent void findObjectById(
            string domainId,
            Ice::Identity id,
            LookupReply reply,
            tag(1) Ice::Protocol? protocol);
#else
        idempotent void findAdapterById(string domainId, string id, LookupReply reply);
        idempotent void findObjectById(string domainId, Ice::Identity id, LookupReply reply);
#endif
    }
}
