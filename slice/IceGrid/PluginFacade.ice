// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h", "objc:header-dir:objc"]]
[["cpp:include:IceGrid/Config.h"]]

#include <Ice/BuiltinSequences.ice>
#include <Ice/Current.ice>

#include <IceGrid/Admin.ice>

["objc:prefix:ICEGRID"]
module IceGrid
{

/**
 * 
 * The ReplicaGroupFilter is used by IceGrid to filter adapters
 * returned to the client when it resolves a filtered replica group.
 *
 * IceGrid provides the list of available adapters. The implementation
 * of this method can use the provided context and connection to
 * filter and return the filtered set of adapters.
 *
 **/
local interface ReplicaGroupFilter
{
    /**
     *
     * Filter the the given set of adapters.
     *
     * @param replicaGroupId The replica group ID.
     *
     * @param adapterIds The adpater IDs to filter.
     *
     * @param con The connection from the Ice client which is
     * resolving the replica group endpoints.
     *
     * @param ctx The context from the ICe client which is resolving
     * the replica group endpoints.
     * 
     * @return The filtered adapter IDs.
     * 
     **/
    Ice::StringSeq filter(string replicaGroupId, Ice::StringSeq adapterIds, Ice::Connection con, Ice::Context ctx);
};

/**
 * 
 * The TypeFilter is used by IceGrid to filter well-known proxies
 * returned to the client when it searches a well-known object by
 * type.
 *
 * IceGrid provides the list of available proxies. The implementation
 * of this method can use the provided context and connection to
 * filter and return the filtered set of proxies.
 *
 **/
local interface TypeFilter
{
    /**
     *
     * Filter the the given set of proxies.
     *
     * @param type The type.
     *
     * @param proxies The proxies to filter.
     *
     * @param con The connection from the Ice client which is
     * looking up well-known objects by type.
     *
     * @param ctx The context from the ICe client which is looking up
     * well-known objects by type.
     * 
     * @return The filtered proxies.
     * 
     **/
    Ice::ObjectProxySeq filter(string type, Ice::ObjectProxySeq proxies, Ice::Connection con, Ice::Context ctx);
};


/**
 *
 * The RegistryPluginFacade is implemented by IceGrid and can be used
 * by plugins and filter implementations to retrieve information from
 * IceGrid about the well-known objects or adapters. It's also used to
 * register/unregister replica group and type filters.
 *
 **/
local interface RegistryPluginFacade
{
    /**
     *
     * Get an application descriptor.
     *
     * @param name The application name.
     *
     * @return The application descriptor.
     *
     * @throws ApplicationNotExistException Raised if the application
     * doesn't exist.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent ApplicationInfo getApplicationInfo(string name)
        throws ApplicationNotExistException;

    /**
     *
     * Get the server information for the server with the given id.
     *
     * @param id The server id.
     *
     * @throws ServerNotExistException Raised if the server doesn't exist.
     *
     * @return The server information.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent ServerInfo getServerInfo(string id)
        throws ServerNotExistException;

    /**
     *
     * Get the ID of the server to which the given adapter belongs.
     *
     * @param adapterId The adapter ID.
     *
     * @return The server ID or the empty string if the given
     * identifier is not associated to an object adapter defined with
     * an application descriptor.
     *
     * @throws AdapterNotExistException Raised if the adapter doesn't
     * exist.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent string getAdapterServer(string adapterId)
        throws AdapterNotExistException;

    /**
     *
     * Get the name of the application to which the given adapter belongs.
     *
     * @param adapterId The adapter ID.
     *
     * @return The application name or the empty string if the given
     * identifier is not associated to a replica group or object
     * adapter defined with an application descriptor.
     *
     * @throws AdapterNotExistException Raised if the adapter doesn't
     * exist.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent string getAdapterApplication(string adapterId)
        throws AdapterNotExistException;

    /**
     *
     * Get the name of the node to which the given adapter belongs.
     *
     * @param adapterId The adapter ID.
     *
     * @return The node name or the empty string if the given
     * identifier is not associated to an object adapter defined with
     * an application descriptor.
     *
     * @throws AdapterNotExistException Raised if the adapter doesn't
     * exist.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent string getAdapterNode(string adapterId)
        throws AdapterNotExistException;

    /**
     *
     * Get the adapter information for the replica group or adapter
     * with the given id.
     *
     * @param id The adapter id.
     *
     * @return A sequence of adapter information structures. If the
     * given id refers to an adapter, this sequence will contain only
     * one element. If the given id refers to a replica group, the
     * sequence will contain the adapter information of each member of
     * the replica group.
     *
     * @throws AdapterNotExistException Raised if the adapter or
     * replica group doesn't exist.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent AdapterInfoSeq getAdapterInfo(string id)
        throws AdapterNotExistException;

    /**
     *
     * Get the object info for the object with the given identity.
     *
     * @param id The identity of the object.
     *
     * @return The object info.
     *
     * @throws ObjectNotRegisteredException Raised if the object isn't
     * registered with the registry.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent ObjectInfo getObjectInfo(Ice::Identity id)
        throws ObjectNotRegisteredException;

    /**
     *
     * Get the node information for the node with the given name.
     *
     * @param name The node name.
     *
     * @return The node information.
     * 
     * @throws NodeNotExistException Raised if the node doesn't exist.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent NodeInfo getNodeInfo(string name)
        throws NodeNotExistException, NodeUnreachableException;

    /**
     *
     * Get the load averages of the node.
     *
     * @param name The node name.
     *
     * @return The node load information.
     * 
     * @throws NodeNotExistException Raised if the node doesn't exist.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent LoadInfo getNodeLoad(string name)
        throws NodeNotExistException, NodeUnreachableException;

    /**
     *
     * Get the property value for the given property and adapter. The
     * property is looked up in the server or service descriptor where
     * the adapter is defined.
     *
     * @param adapterId The adapter ID
     *
     * @param name The name of the property.
     *
     * @return The property value.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent string getPropertyForAdapter(string adapterId, string name)
        throws AdapterNotExistException;

    /**
     *
     * Add a replica group filter.
     *
     * @param id The identifier of the filter. This identifier must
     * match the value of the "filter" attribute specified in the
     * replica group descriptor. To filter dynamically registered
     * replica groups, you should use the empty filter id.
     *
     * @param filter The filter implementation.
     *
     **/
    void addReplicaGroupFilter(string id, ReplicaGroupFilter filter);

    /**
     *
     * Remove a replica group filter.
     *
     * @param id The identifier of the filter.
     *
     * @param filter The filter implementation.
     *
     * @return True of the filter was removed, false otherwise.
     *
     **/
    bool removeReplicaGroupFilter(string id, ReplicaGroupFilter filter);

    /**
     *
     * Add a type filter.
     *
     * @param type The type to register this filter with.
     *
     * @param filter The filter implementation.
     *
     **/
    void addTypeFilter(string type, TypeFilter filter);

    /**
     *
     * Remove a type filter.
     *
     * @param type The type to register this filter with.
     *
     * @param filter The filter implementation.
     *
     * @return True of the filter was removed, false otherwise.
     *
     **/
    bool removeTypeFilter(string type, TypeFilter filter);
};

};
