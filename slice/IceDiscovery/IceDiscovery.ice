// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:doxygen:include:IceDiscovery/IceDiscovery.h"]]
[["cpp:header-ext:h"]]

[["js:module:@zeroc/ice"]]

[["python:pkgdir:IceDiscovery"]]

#include "Ice/Identity.ice"

[["java:package:com.zeroc"]]

module IceDiscovery
{
    /// The LookupReply interface is used by IceDiscovery clients to answer requests received on the Lookup interface.
    interface LookupReply
    {
        /// Reply to the {@link Lookup#findObjectById} request.
        /// @param id The identity of the object.
        /// @param prx The proxy of the object. This proxy is never null.
        void foundObjectById(Ice::Identity id, Object* prx);

        /// Reply to the {@link Lookup#findAdapterById} request.
        /// @param id The adapter ID.
        /// @param prx The adapter proxy (a dummy proxy created by the adapter). The proxy provides the adapter
        /// endpoints. This proxy is never null.
        /// @param isReplicaGroup Indicates whether the adapter is a member of a replica group.
        void foundAdapterById(string id, Object* prx, bool isReplicaGroup);
    }

    /// The Lookup interface is used by IceDiscovery clients to look for objects and adapters using UDP multicast.
    interface Lookup
    {
        /// Request to find an Ice object.
        /// @param domainId The IceDiscovery domain identifier. An IceDiscovery client only replies to requests with a
        /// matching domain identifier.
        /// @param id The object identity.
        /// @param reply The proxy of the LookupReply interface that should be used to send the reply if a matching
        /// object is found. The reply proxy is never null.
        idempotent void findObjectById(string domainId, Ice::Identity id, LookupReply* reply);

        /// Request to find an object adapter.
        /// @param domainId The IceDiscovery domain identifier. An IceDiscovery client only replies to requests with a
        /// matching domain identifier.
        /// @param id The adapter ID.
        /// @param reply The proxy of the LookupReply interface that should be used to send the reply if a matching
        /// adapter is found. The reply proxy is never null.
        idempotent void findAdapterById(string domainId, string id, LookupReply* reply);
    }
}
