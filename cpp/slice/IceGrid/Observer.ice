// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_OBSERVER_ICE
#define ICE_GRID_OBSERVER_ICE

#include <Glacier2/Session.ice>
#include <IceGrid/Exception.ice>
#include <IceGrid/Descriptor.ice>
#include <IceGrid/Admin.ice>

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
["java:type:{java.util.LinkedList}"] sequence<ServerDynamicInfo> ServerDynamicInfoSeq;

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
["java:type:{java.util.LinkedList}"] sequence<AdapterDynamicInfo> AdapterDynamicInfoSeq;

/**
 *
 * Dynamic information about the state of a node.
 *
 **/
struct NodeDynamicInfo
{
    /**
     *
     * The name of the node.
     *
     **/
    string name;

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
     * The init operation is called after the registration of the
     * observer to communicate the current state of the node to the
     * observer implementation.
     *
     * @param nodes The current state of the nodes.
     *
     **/
    ["ami"] void init(NodeDynamicInfoSeq nodes);

    /**
     *
     * The nodeUp operation is called to notify the observer that a node
     * came up.
     * 
     * @param node The node state.
     *
     **/
    void nodeUp(NodeDynamicInfo node);

    /**
     *
     * The nodeDown operation is called to notify the observer that a node
     * went down.
     * 
     * @param name The node name.
     *
     **/
    void nodeDown(string name);

    /**
     *
     * The updateServer operation is called to notify the observer that
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
     * The updateAdapter operation is called to notify the observer that
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
 * The registry observer interface. Observers should implement this
 * interface to receive information about the state of the IceGrid
 * registry.
 * 
 **/
interface RegistryObserver
{
    /**
     *
     * The init operation is called after the registration of the
     * observer to communicate the current state of the registry to the
     * observer implementation.
     *
     * @param serial The current serial number of the registry database. This
     * serial number allows observers to make sure that their internal state
     * is synchronized with the registry.
     *
     * @param applications The applications currently registered with
     * the registry.
     *
     * @param adapters The adapters that were dynamically registered
     * with the registry (not through the deployment mechanism).
     *
     * @param objects The objects registered with the [Admin]
     * interface (not through the deployment mechanism).
     *
     **/
    ["ami"] void init(int serial, ApplicationDescriptorSeq applications, AdapterInfoSeq adpts, ObjectInfoSeq objects);

    /**
     * 
     * The applicationAdded operation is called to notify the observer
     * that an application was added.
     *
     * @param serial The new serial number of the registry database.
     *
     * @param desc The descriptor of the new application.
     * 
     **/
    void applicationAdded(int serial, ApplicationDescriptor desc);

    /**
     *
     * The applicationRemoved operation is called to notify the observer
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
     * The applicationUpdated operation is called to notify the observer
     * that an application was updated.
     *
     * @param serial The new serial number of the registry database.
     *
     * @param desc The descriptor of the update.
     * 
     **/
    void applicationUpdated(int serial, ApplicationUpdateDescriptor desc);

    /**
     *
     * The adapterAdded operation is called to notify the observer when
     * a dynamically-registered adapter was added.
     *
     **/
    void adapterAdded(int serial, AdapterInfo info);

    /**
     *
     * The adapterUpdated operation is called to notify the observer when
     * a dynamically-registered adapter was updated.
     *
     **/
    void adapterUpdated(int serial, AdapterInfo info);

    /**
     *
     * The adapterRemoved operation is called to notify the observer when
     * a dynamically-registered adapter was removed.
     *
     **/
    void adapterRemoved(int serial, string id);

    /**
     *
     * The objectAdded operation is called to notify the observer when an
     * object was added through the [Admin] interface.
     *
     **/
    void objectAdded(int serial, ObjectInfo info);

    /**
     *
     * The objectUpdated operation is called to notify the observer when
     * an object registered through the [Admin] interface was updated.
     *
     **/
    void objectUpdated(int serial, ObjectInfo info);

    /**
     *
     * The objectRemoved operation is called to notify the observer when
     * an object registered through the [Admin] interface was removed.
     *
     **/
    void objectRemoved(int serial, Ice::Identity id);
};

};

#endif
