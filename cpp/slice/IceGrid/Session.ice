// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_SESSION_ICE
#define ICE_GRID_SESSION_ICE

#include <Ice/Locator.ice>
#include <Glacier2/Session.ice>
#include <IceGrid/Observer.ice>

module IceGrid
{

interface Query;
interface Admin;

exception AllocationException
{
    string reason;
};

exception AllocationTimeoutException extends AllocationException
{
};

exception NotAllocatableException extends AllocationException
{
};

interface Session extends Glacier2::Session
{
    /**
     *
     * Keep the session alive. Clients should call this method
     * regularly to prevent the server from reaping the session.
     *
     **/
    void keepAlive();

    /**
     *
     * Get the session timeout configured for the node.
     *
     **/
    nonmutating int getTimeout();

    /**
     *
     * Get the query interface.
     *
     **/
    nonmutating Query* getQuery();

    /**
     *
     * Get the session locator.
     *
     **/
    nonmutating Ice::Locator* getLocator();

    /**
     *
     * Allocate an object. Depending on the allocation timeout, this
     * method might hang until the object is available or until the
     * timeout is reached.
     *
     **/
    ["ami", "amd"] void allocateObject(Object* prx)
	throws ObjectNotRegisteredException, AllocationException;
    
    /**
     *
     * Release an object.
     *
     **/
    void releaseObject(Object* prx)
	throws ObjectNotRegisteredException, AllocationException;
    
    /**
     *
     * Set the allocation timeout. If no objects are available for an
     * allocation request, the request will hang for the duration of 
     * this timeout.
     *
     **/
    void setAllocationTimeout(int timeout);
};

interface AdminSession extends Session
{
    /**
     *
     * Get the admin interface.
     *
     **/
    nonmutating Admin* getAdmin();

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
     * are using a bidirectional connection to communicate with the
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
     * Acquires an exclusive lock to start updating the registry applications.
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
     * Finish updating the registry and release the exclusive lock.
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
     * @param userId Identifies the session user.
     *
     * @return The proxy of the local session.
     *
     **/
    Session* createLocalSession(string userId);
};

};

#endif
