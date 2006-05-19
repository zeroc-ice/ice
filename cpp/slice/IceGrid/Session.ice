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

#include <Glacier2/Session.ice>
#include <IceGrid/Exception.ice>
#include <IceGrid/Observer.ice>

module IceGrid
{

interface Query;
interface Admin;

/**
 *
 * This exception is raised if a client is denied the ability to create
 * a session with IceGrid.
 *
 * @see SessionFactory::createSession
 *
 **/
exception PermissionDeniedException
{
    /**
     *
     * The reason why permission was denied.
     *
     **/
    string reason;
};

interface BaseSession extends Glacier2::Session
{
    /**
     *
     * Keep the session alive. Clients should call this method
     * regularly to prevent the server from reaping the session.
     *
     * @see getTimeout
     *
     **/
    void keepAlive();

    /**
     *
     * Get the session timeout. If a client doesn't call keepAlive in
     * the time interval defined by this timeout, IceGrid might reap
     * the session.
     *
     * @see keepAlive
     *
     * @return The timeout in milliseconds.
     *
     **/
    nonmutating int getTimeout();
};

interface Session extends BaseSession
{
    /**
     *
     * Allocate an object. Depending on the allocation timeout, this
     * method might hang until the object is available or until the
     * timeout is reached.
     * 
     * @param id The identity of the object to allocate.
     *
     * @return The proxy of the allocated object.
     *
     * @throws ObjectNotRegisteredException Raised if the object with
     * the given identity is not registered with the registry.
     *
     * @throws AllocationException Raised if the object can't be
     * allocated.
     *
     **/
    ["ami", "amd"] Object* allocateObjectById(Ice::Identity id)
	throws ObjectNotRegisteredException, AllocationException;
    
    /**
     *
     * Allocate an object with the given type. Depending on the
     * allocation timeout, this method might hang until an object
     * becomes available or until the timeout is reached.
     *
     * @param type The type of the object.
     *
     * @return The proxy of the allocated object.
     *
     * @throws Raised if no objects with the given type can be
     * allocated.
     *
     **/
    ["ami", "amd"] Object* allocateObjectByType(string type)
	throws AllocationException;
    
    /**
     *
     * Release an object.
     *
     * @param id The identity of the object to release.
     *
     * @throws ObjectNotRegisteredException Raised if the object with
     * the given identity is not registered with the regitry.
     *
     * @throws AllocationException Raised if the given object can't be
     * released. This might happen if the object isn't allocatable or
     * allocated by the session.
     *
     **/
    void releaseObject(Ice::Identity id)
	throws ObjectNotRegisteredException, AllocationException;
    
    /**
     *
     * Set the allocation timeout. If no objects are available for an
     * allocation request, the request will hang for the duration of 
     * this timeout.
     *
     * @param timeout The timeout in milliseconds.
     *
     **/
    void setAllocationTimeout(int timeout);
};

interface AdminSession extends BaseSession
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

};

#endif
