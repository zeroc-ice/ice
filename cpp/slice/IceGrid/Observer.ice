// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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
};

/**
 *
 * A sequence of server dynamic information structures.
 * 
 **/
sequence<ServerDynamicInfo> ServerDynamicInfoSeq;

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
     * The id of the server this adapter belongs to.
     *
     **/
    string serverId;

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
sequence<AdapterDynamicInfo> AdapterDynamicInfoSeq;

/**
 *
 * Dynamic information about the state of a node.
 *
 **/
struct NodeDynamicInfo
{
    string name;
    ServerDynamicInfoSeq servers;
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
     * The init method is called after the registration of the
     * observer to communicate the current state of the node to the
     * observer implementation.
     *
     * @param nodes The current state of the nodes.
     *
     **/
    ["ami"] void init(NodeDynamicInfoSeq nodes);

    /**
     *
     * The nodeUp method is called to notify the observer that a node
     * came up.
     * 
     * @param node The node state.
     *
     **/
    void nodeUp(NodeDynamicInfo node);

    /**
     *
     * The nodeDown method is called to notify the observer that a node
     * went down.
     * 
     * @param name The node name.
     *
     **/
    void nodeDown(string name);

    /**
     *
     * The updateServer method is called to notify the observer that
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
     * The updateAdapter method is called to notify the observer that
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
     * The init method is called after the registration of the
     * observer to communicate the current state of the registry to the
     * observer implementation.
     *
     * @param serial The current serial of the registry database. This
     * serial allows observers to make sure that their internal state
     * is synchronized with the registry.
     *
     * @param applications The applications currently registered with
     * the registry.
     *
     **/
    ["ami"] void init(int serial, ApplicationDescriptorSeq applications);

    /**
     * 
     * The applicationAdded method is called to notify the observer
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
     * The applicationRemoved method is called to notify the observer
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
     * The applicationUpdated method is called to notify the observer
     * that an application was updated.
     *
     * @param serial The new serial number of the registry database.
     *
     * @param desc The descriptor of the update.
     * 
     **/
    void applicationUpdated(int serial, ApplicationUpdateDescriptor desc);
};

interface Session extends Glacier2::Session
{
    /**
     *
     * Keep alive the session. Clients should call this method
     * regularily to ensure the server won't reap the session.
     *
     **/
    void keepAlive();

    /**
     *
     * Set the proxies of the observer objects that will receive
     * notifications from the servers when the state of the registry
     * or nodes changes.
     *
     * @param registryObs The registry observer.
     *
     * @param nodeObs The node observer.
     *
     **/
    void setObservers(RegistryObserver* registryObs, NodeObserver* nodeObs);

    /**
     *
     * Set the identities of the observer objects that will receive
     * notifications from the servers when the state of the registry
     * or nodes changes. This method should be used by clients which
     * are using a bi-directional connection to communicate with the
     * session.
     *
     * @param registryObs The registry observer identity.
     *
     * @param nodeObs The node observer identity.
     *
     **/
    void setObserversByIdentity(Ice::Identity registryObs, Ice::Identity nodeObs);

    /**
     *
     * Acquires to registry exclusive lock to start updating the
     * registry applications.
     *
     * @return The current serial.
     * 
     * @throws AccessDeniedException Raised if the exclusive lock can't be
     * acquired. This might be because it's already acquired by
     * another session.
     *
     **/
    int startUpdate()
	throws AccessDeniedException;
    
    /**
     *
     * Add an application. This method must be called to update the
     * registry applications using the lock mechanism.
     *
     * @throws AccessDeniedException Raised if the session doesn't hold the
     * exclusive lock.
     *
     **/
    void addApplication(ApplicationDescriptor application)
	throws AccessDeniedException, DeploymentException;

    /**
     *
     * Update an application. This method must be called to update the
     * registry applications using the lock mechanism.
     *
     * @throws AccessDeniedException Raised if the session doesn't hold the
     * exclusive lock.
     *
     **/
    void syncApplication(ApplicationDescriptor app)
	throws AccessDeniedException, DeploymentException, ApplicationNotExistException;

    /**
     *
     * Update an application. This method must be called to update the
     * registry applications using the lock mechanism.
     *
     * @throws AccessDeniedException Raised if the session doesn't hold the
     * exclusive lock.
     *
     **/
    void updateApplication(ApplicationUpdateDescriptor update)
	throws AccessDeniedException, DeploymentException, ApplicationNotExistException;

    /**
     *
     * Update an application. This method must be called to update the
     * registry applications using the lock mechanism.
     *
     * @throws AccessDeniedException Raised if the session doesn't hold the
     * exclusive lock.
     *
     **/
    void removeApplication(string name)
	throws AccessDeniedException, ApplicationNotExistException;

    /**
     *
     * Finish to update the registry and release the exclusive
     * lock.
     *
     * @throws AccessDeniedException Raised if the session doesn't hold the
     * exclusive lock.
     *
     **/
    void finishUpdate()
	throws AccessDeniedException;
};

interface SessionManager extends Glacier2::SessionManager
{
    /**
     *
     * Create a local session.
     *
     * @param userId An identifier to identify the session user.
     *
     * @return The proxy on the local session.
     *
     **/
    Session* createLocalSession(string userId);
};

};

#endif
