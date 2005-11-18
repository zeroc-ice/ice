// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_QUERY_ICE
#define ICE_GRID_QUERY_ICE

#include <Ice/Identity.ice>
#include <Ice/BuiltinSequences.ice>

#include <IceGrid/Exception.ice>

/**
 *
 * &IceGrid; is a server activation and deployment tool. &IceGrid;,
 * simplifies the complex task of deploying applications in a
 * heterogenous computer network.
 *
 **/
module IceGrid
{

enum LoadSample
{
    LoadSample1,
    LoadSample5,
    LoadSample15
};

/**
 *
 * The &IceGrid; query interface. This interface is accessible to
 * &Ice; clients who wish to lookup objects.
 *
 **/
interface Query
{
    /**
     *
     * Find an object by identity.
     *
     * @param id The identity.
     *
     * @return The proxy or null if no such object has been found.
     *
     **/
    nonmutating Object* findObjectById(Ice::Identity id);

    /**
     *
     * Find an object by type. If there's several objects registered
     * for the given type, the object will be randomly selected.
     *
     * @param type The object type.
     *
     * @return The proxy or null if no such object has been found.
     *
     **/
    nonmutating Object* findObjectByType(string type);


    /**
     *
     * Find an object by type on the least loaded node. If the
     * registry can't figure out the node that hosts the object (e.g.,
     * if the object was registered with a direct proxy), the registry
     * assumes the object is hosted on a node that has a load average
     * of 1.0.
     *
     * @param type The object type.
     *
     * @return The proxy or null if no such object has been found.
     *
     **/
    nonmutating Object* findObjectByTypeOnLeastLoadedNode(string type, LoadSample sample);

    /**
     *
     * Find all the objects with the given type.
     *
     * @param type The object type.
     *
     * @return The proxies or an empty sequence if no such objects
     * have been found.
     *
     **/
    nonmutating Ice::ObjectProxySeq findAllObjectsByType(string type);
};

};

#endif
