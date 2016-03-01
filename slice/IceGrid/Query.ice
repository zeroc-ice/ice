// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h", "objc:header-dir:objc", "js:ice-build"]]
[["cpp:include:IceGrid/Config.h"]]

#include <Ice/Identity.ice>
#include <Ice/BuiltinSequences.ice>

#include <IceGrid/Exception.ice>

/**
 *
 * IceGrid is a server activation and deployment tool. IceGrid,
 * simplifies the complex task of deploying applications in a
 * heterogeneous computer network.
 *
 **/
["objc:prefix:ICEGRID"]
module IceGrid
{

/**
 *
 * Determines which load sampling interval to use.
 *
 **/
enum LoadSample
{
    /**
     * Sample every minute.
     **/
    LoadSample1,

    /**
     * Sample every five minutes.
     **/
    LoadSample5,

    /**
     * Sample every fifteen minutes.
     **/
    LoadSample15
};

/**
 *
 * The IceGrid query interface. This interface is accessible to
 * Ice clients who wish to look up well-known objects.
 *
 **/
interface Query
{
    /**
     *
     * Find a well-known object by identity.
     *
     * @param id The identity.
     *
     * @return The proxy or null if no such object has been found.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent Object* findObjectById(Ice::Identity id);

    /**
     *
     * Find a well-known object by type. If there are several objects
     * registered for the given type, the object is randomly
     * selected.
     *
     * @param type The object type.
     *
     * @return The proxy or null, if no such object has been found.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent Object* findObjectByType(string type);

    /**
     *
     * Find a well-known object by type on the least-loaded node. If
     * the registry does not know which node hosts the object
     * (for example, because the object was registered with a direct proxy), the
     * registry assumes the object is hosted on a node that has a load
     * average of 1.0.
     *
     * @param type The object type.
     *
     * @param sample The sampling interval.
     *
     * @return The proxy or null, if no such object has been found.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent Object* findObjectByTypeOnLeastLoadedNode(string type, LoadSample sample);

    /**
     *
     * Find all the well-known objects with the given type.
     *
     * @param type The object type.
     *
     * @return The proxies or an empty sequence, if no such objects
     * have been found.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent Ice::ObjectProxySeq findAllObjectsByType(string type);

    /**
     *
     * Find all the object replicas associated with the given
     * proxy. If the given proxy is not an indirect proxy from a
     * replica group, an empty sequence is returned.
     *
     * @param proxy The object proxy.
     *
     * @return The proxies of each object replica or an empty sequence,
     * if the given proxy is not from a replica group.
     *
     **/
    ["cpp:const"] idempotent Ice::ObjectProxySeq findAllReplicas(Object* proxy);
};

};

