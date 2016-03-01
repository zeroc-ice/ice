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

#include <Glacier2/Session.ice>
#include <IceGrid/Exception.ice>
#include <IceGrid/Descriptor.ice>
#include <IceGrid/Admin.ice>

["objc:prefix:ICEGRID"]
module IceGrid
{

/**
 *
 * Dynamic information about the state of a server.
 *
 **/
struct ServerDynamicInfo
{
    /**
     *
     * The id of the server.
     *
     **/
    string id;

    /**
     *
     * The state of the server.
     *
     **/
    ServerState state;

    /**
     *
     * The process id of the server.
     *
     **/
    int pid;

    /**
     *
     * Indicates whether the server is enabled.
     *
     **/
    bool enabled;
};

/**
 *
 * A sequence of server dynamic information structures.
 *
 **/
["java:type:java.util.LinkedList<ServerDynamicInfo>"] sequence<ServerDynamicInfo> ServerDynamicInfoSeq;

/**
 *
 * Dynamic information about the state of an adapter.
 *
 **/
struct AdapterDynamicInfo
{
    /**
     *
     * The id of the adapter.
     *
     **/
    string id;

    /**
     *
     * The direct proxy containing the adapter endpoints.
     *
     **/
    Object* proxy;
};

/**
 *
 * A sequence of adapter dynamic information structures.
 *
 **/
["java:type:java.util.LinkedList<AdapterDynamicInfo>"] sequence<AdapterDynamicInfo> AdapterDynamicInfoSeq;

/**
 *
 * Dynamic information about the state of a node.
 *
 **/
struct NodeDynamicInfo
{
    /**
     *
     * Some static information about the node.
     *
     **/
    NodeInfo info;

    /**
     *
     * The dynamic information of the servers deployed on this node.
     *
     **/
    ServerDynamicInfoSeq servers;

    /**
     *
     * The dynamic information of the adapters deployed on this node.
     *
     **/
    AdapterDynamicInfoSeq adapters;
};

/**
 *
 * A sequence of node dynamic information structures.
 *
 **/
sequence<NodeDynamicInfo> NodeDynamicInfoSeq;

/**
 *
 * The node observer interface. Observers should implement this
 * interface to receive information about the state of the IceGrid
 * nodes.
 *
 **/
interface NodeObserver
{
    /**
     *
     * The <tt>nodeInit</tt> operation indicates the current state
     * of nodes. It is called after the registration of an observer.
     *
     * @param nodes The current state of the nodes.
     *
     **/
    void nodeInit(NodeDynamicInfoSeq nodes);

    /**
     *
     * The <tt>nodeUp</tt> operation is called to notify an observer that a node
     * came up.
     *
     * @param node The node state.
     *
     **/
    void nodeUp(NodeDynamicInfo node);

    /**
     *
     * The <tt>nodeDown</tt> operation is called to notify an observer that a node
     * went down.
     *
     * @param name The node name.
     *
     **/
    void nodeDown(string name);

    /**
     *
     * The <tt>updateServer</tt> operation is called to notify an observer that
     * the state of a server changed.
     *
     * @param node The node hosting the server.
     *
     * @param updatedInfo The new server state.
     *
     **/
    void updateServer(string node, ServerDynamicInfo updatedInfo);

    /**
     *
     * The <tt>updateAdapter</tt> operation is called to notify an observer that
     * the state of an adapter changed.
     *
     * @param node The node hosting the adapter.
     *
     * @param updatedInfo The new adapter state.
     *
     **/
     void updateAdapter(string node, AdapterDynamicInfo updatedInfo);
};

/**
 *
 * The database observer interface. Observers should implement this
 * interface to receive information about the state of the IceGrid
 * registry database.
 *
 **/
interface ApplicationObserver
{
    /**
     *
     * <tt>applicationInit</tt> is called after the registration
     * of an observer to indicate the state of the registry.
     *
     * @param serial The current serial number of the registry
     * database. This serial number allows observers to make sure that
     * their internal state is synchronized with the registry.
     *
     * @param applications The applications currently registered with
     * the registry.
     *
     **/
    void applicationInit(int serial, ApplicationInfoSeq applications);

    /**
     *
     * The <tt>applicationAdded</tt> operation is called to notify an observer
     * that an application was added.
     *
     * @param serial The new serial number of the registry database.
     *
     * @param desc The descriptor of the new application.
     *
     **/
    void applicationAdded(int serial, ApplicationInfo desc);

    /**
     *
     * The <tt>applicationRemoved</tt> operation is called to notify an observer
     * that an application was removed.
     *
     * @param serial The new serial number of the registry database.
     *
     * @param name The name of the application that was removed.
     *
     **/
    void applicationRemoved(int serial, string name);

    /**
     *
     * The <tt>applicationUpdated</tt> operation is called to notify an observer
     * that an application was updated.
     *
     * @param serial The new serial number of the registry database.
     *
     * @param desc The descriptor of the update.
     *
     **/
    void applicationUpdated(int serial, ApplicationUpdateInfo desc);
};

/**
 *
 * This interface allows applications to monitor the state of object
 * adapters that are registered with IceGrid.
 *
 **/
interface AdapterObserver
{
    /**
     *
     * <tt>adapterInit</tt> is called after registration of
     * an observer to indicate the state of the registry.
     *
     * @param adpts The adapters that were dynamically registered
     * with the registry (not through the deployment mechanism).
     *
     **/
    void adapterInit(AdapterInfoSeq adpts);

    /**
     *
     * The <tt>adapterAdded</tt> operation is called to notify an observer when
     * a dynamically-registered adapter was added.
     *
     * @param info The details of the new adapter.
     **/
    void adapterAdded(AdapterInfo info);

    /**
     *
     * The adapterUpdated operation is called to notify an observer when
     * a dynamically-registered adapter was updated.
     *
     * @param info The details of the updated adapter.
     *
     **/
    void adapterUpdated(AdapterInfo info);

    /**
     *
     * The adapterRemoved operation is called to notify an observer when
     * a dynamically-registered adapter was removed.
     *
     * @param id The ID of the removed adapter.
     *
     **/
    void adapterRemoved(string id);
};

/**
 *
 * This interface allows applications to monitor IceGrid well-known objects.
 *
 **/
interface ObjectObserver
{
    /**
     *
     * <tt>objectInit</tt> is called after the registration of
     * an observer to indicate the state of the registry.
     *
     * @param objects The objects registered with the {@link Admin}
     * interface (not through the deployment mechanism).
     *
     **/
    void objectInit(ObjectInfoSeq objects);

    /**
     *
     * The <tt>objectAdded</tt> operation is called to notify an observer when an
     * object was added to the {@link Admin} interface.
     *
     * @param info The details of the added object.
     *
     **/
    void objectAdded(ObjectInfo info);

    /**
     *
     * <tt>objectUpdated</tt> is called to notify an observer when
     * an object registered with the {@link Admin} interface was updated.
     *
     * @param info The details of the updated object.
     *
     **/
    void objectUpdated(ObjectInfo info);

    /**
     *
     * <tt>objectRemoved</tt> is called to notify an observer when
     * an object registered with the {@link Admin} interface was removed.
     *
     * @param id The identity of the removed object.
     *
     **/
    void objectRemoved(Ice::Identity id);
};

/**
 *
 * This interface allows applications to monitor changes the state
 * of the registry.
 *
 **/
interface RegistryObserver
{
    /**
     *
     * The <tt>registryInit</tt> operation is called after registration of
     * an observer to indicate the state of the registries.
     *
     * @param registries The current state of the registries.
     *
     **/
    void registryInit(RegistryInfoSeq registries);

    /**
     *
     * The <tt>nodeUp</tt> operation is called to notify an observer that a node
     * came up.
     *
     * @param node The node state.
     *
     **/
    void registryUp(RegistryInfo node);

    /**
     *
     * The <tt>nodeDown</tt> operation is called to notify an observer that a node
     * went down.
     *
     * @param name The node name.
     *
     **/
    void registryDown(string name);
};

};

