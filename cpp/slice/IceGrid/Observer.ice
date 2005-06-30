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
#include <IceGrid/Descriptor.ice>
#include <IceGrid/Exception.ice>

module IceGrid
{

struct ServerDynamicInfo
{
    string name;
    ServerState state;
    int pid;
};
sequence<ServerDynamicInfo> ServerDynamicInfoSeq;

struct AdapterDynamicInfo
{
    string id;
    Object* proxy;
};
sequence<AdapterDynamicInfo> AdapterDynamicInfoSeq;

struct NodeDynamicInfo
{
    string name;
    ServerDynamicInfoSeq servers;
    AdapterDynamicInfoSeq adapters;
};
sequence<NodeDynamicInfo> NodeDynamicInfoSeq;

interface NodeObserver
{
    ["ami"] void init(NodeDynamicInfoSeq nodes);

    void nodeUp(NodeDynamicInfo node);

    void nodeDown(string name);

    void updateServer(string node, ServerDynamicInfo updatedInfo);

    void updateAdapter(string node, AdapterDynamicInfo updatedInfo);
};

interface RegistryObserver
{
    ["ami"] void init(int serial, ApplicationDescriptorSeq applications);

    void applicationAdded(int serial, ApplicationDescriptor desc);
    void applicationRemoved(int serial, string name);
    void applicationSynced(int serial, ApplicationDescriptor desc);
    void applicationUpdated(int serial, ApplicationUpdateDescriptor desc);
};

/**
 *
 * This exception is raised if the cache is out of date.
 *
 **/
exception CacheOutOfDate
{
};

/**
 *
 * This exception is raised if an operation can't be performed because
 * the regitry lock wasn't acquired or is already acquired by a session.
 *
 **/
exception AccessDenied
{
    string lockUserId;
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
     * are using a bi-directional connection to communicator with the
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
     * @param cacheSerialSession The client cache serial number.
     * 
     * @throws CacheOutOfDate Raised if the cache serial number
     * provided by the client is inferior to the current registry
     * serial number. The client should refresh its cache and try
     * again.
     *
     * @throws AccessDenied Raised if the exclusive lock can't be
     * acquired. This might be because it's already acquired by
     * another session.
     *
     **/
    void startUpdate(int cacheSerialSession)
	throws CacheOutOfDate, AccessDenied;
    
    /**
     *
     * Add an application. This method must be called to update the
     * registry applications using the lock mechanism.
     *
     * @throws AccessDenied Raised if the session doesn't hold the
     * exclusive lock.
     *
     **/
    void addApplication(ApplicationDescriptor application)
	throws AccessDenied, DeploymentException, ApplicationExistsException;

    /**
     *
     * Update an application. This method must be called to update the
     * registry applications using the lock mechanism.
     *
     * @throws AccessDenied Raised if the session doesn't hold the
     * exclusive lock.
     *
     **/
    void syncApplication(ApplicationDescriptor app)
	throws AccessDenied, DeploymentException, ApplicationNotExistException;

    /**
     *
     * Update an application. This method must be called to update the
     * registry applications using the lock mechanism.
     *
     * @throws AccessDenied Raised if the session doesn't hold the
     * exclusive lock.
     *
     **/
    void updateApplication(ApplicationUpdateDescriptor update)
	throws AccessDenied, DeploymentException, ApplicationNotExistException;

    /**
     *
     * Update an application. This method must be called to update the
     * registry applications using the lock mechanism.
     *
     * @throws AccessDenied Raised if the session doesn't hold the
     * exclusive lock.
     *
     **/
    void removeApplication(string name)
	throws AccessDenied, ApplicationNotExistException;

    /**
     *
     * Finish to update the registry and release the exclusive
     * lock.
     *
     * @throws AccessDenied Raised if the session doesn't hold the
     * exclusive lock.
     *
     **/
    void finishUpdate()
	throws AccessDenied;
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
