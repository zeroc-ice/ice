//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["cpp:doxygen:include:IceDiscovery/IceDiscovery.h"]]
[["cpp:header-ext:h"]]

[["ice-prefix"]]

[["js:module:ice"]]
[["js:cjs-module"]]

[["objc:header-dir:objc"]]

[["python:pkgdir:IceDiscovery"]]

#include <Ice/Identity.ice>

#ifndef __SLICE2JAVA_COMPAT__
[["java:package:com.zeroc"]]
#endif

module IceDiscovery
{

/**
 *
 * The LookupReply interface is used by IceDiscovery clients to answer requests
 * received on the Lookup interface.
 *
 **/
interface LookupReply
{
    /**
     * Reply to the findObjectById request.
     *
     * @param id The identity of the object.
     *
     * @param prx The proxy of the object.
     *
     **/
    void foundObjectById(Ice::Identity id, Object* prx);

    /**
     * Reply to the findAdpaterById request.
     *
     * @param id The adapter ID.
     *
     * @param prx The adapter proxy (a dummy proxy created by the adapter).
     * The proxy provides the adapter endpoints.
     *
     * @param isReplicaGroup True if the adapter is also a member of a
     * replica group.
     *
     **/
    void foundAdapterById(string id, Object* prx, bool isReplicaGroup);
}

/**
 *
 * The Lookup interface is used by IceDiscovery clients to look for objects
 * and adapters using UDP multicast.
 *
 **/
interface Lookup
{
    /**
     * Request to find an Ice object
     *
     * @param domainId The IceDiscovery domain identifier. An IceDiscovery client only
     * replies to requests with a matching domain identifier.
     *
     * @param id The object identity.
     *
     * @param reply The proxy of the LookupReply interface that should be used to send
     * the reply if a matching object is found.
     *
     **/
    idempotent void findObjectById(string domainId, Ice::Identity id, LookupReply* reply);

    /**
     * Request to find an object adapter
     *
     * @param domainId The IceDiscovery domain identifier. An IceDiscovery client only
     * replies to requests with a matching domain identifier.
     *
     * @param id The adapter ID.
     *
     * @param reply The proxy of the LookupReply interface that should be used to send
     * the reply if a matching adapter is found.
     *
     **/
    idempotent void findAdapterById(string domainId, string id, LookupReply* reply);
}

}
