// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:header-ext:h"]]

[["js:module:@zeroc/ice"]]
[["python:pkgdir:IceDiscovery"]]

#include "Ice/Identity.ice"

["java:identifier:com.zeroc.IceDiscovery"]
module IceDiscovery
{
    /// Represents a callback object implemented by IceDiscovery clients. It allows IceDiscovery clients to receive
    /// replies to Lookup requests.
    interface LookupReply
    {
        /// Provides a reply to a {@link Lookup#findObjectById} request.
        /// @param id The identity of the object.
        /// @param prx The proxy of the object. This proxy is never null.
        void foundObjectById(Ice::Identity id, Object* prx);

        /// Provides a reply to a {@link Lookup#findAdapterById} request.
        /// @param id The adapter ID.
        /// @param prx The adapter proxy (a dummy proxy created by the adapter). The proxy provides the adapter
        /// endpoints. This proxy is never null.
        /// @param isReplicaGroup Indicates whether the adapter is a member of a replica group.
        void foundAdapterById(string id, Object* prx, bool isReplicaGroup);
    }

    /// Looks up objects and object adapters using UDP multicast.
    interface Lookup
    {
        /// Finds a well-known Ice object.
        /// @param domainId The IceDiscovery domain identifier. An IceDiscovery client only replies to requests with a
        /// matching domain identifier.
        /// @param id The well-known object identity.
        /// @param reply The proxy of the LookupReply interface that should be used to send the reply if a matching
        /// object is found. The reply proxy is never null.
        idempotent void findObjectById(string domainId, Ice::Identity id, LookupReply* reply);

        /// Finds an object adapter.
        /// @param domainId The IceDiscovery domain identifier. An IceDiscovery client only replies to requests with a
        /// matching domain identifier.
        /// @param id The adapter ID.
        /// @param reply The proxy of the LookupReply interface that should be used to send the reply if a matching
        /// adapter is found. The reply proxy is never null.
        idempotent void findAdapterById(string domainId, string id, LookupReply* reply);
    }
}
