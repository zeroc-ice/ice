// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_LOCATOR_ICE
#define ICE_LOCATOR_ICE

#include <Ice/Identity.ice>

module Ice
{

/**
 *
 * This exception is raised if an adapter can't be found.
 *
 **/
exception AdapterNotFoundException
{
};

/**
 *
 * This exception is raised if a server tries to set endpoints for
 * an adapter which is already active.
 *
 **/
exception AdapterAlreadyActiveException
{
};

/**
 *
 * This exception is raised if an object can't be found.
 *
 **/
exception ObjectNotFoundException
{
};

interface LocatorRegistry;

/**
 *
 * The &Ice; locator interface. This interface is used by clients to
 * lookup adapters and objects. It's also used by servers to get the
 * locator registry proxy.
 *
 * <note><para> The [Locator] interface is intended to be used by
 * &Ice; internals and by locator implementations. Regular user code
 * should not attempt to use any functionality of this interface
 * directly.</para></note>
 *
 **/
interface Locator
{
    /**
     *
     * Find an object by identity and return its proxy.
     *
     * @param id The identity.
     *
     * @return The proxy or null if the object is not active.
     *
     * @throws ObjectNotFoundException Raised if the object can't
     * be found.
     *
     **/
    nonmutating Object* findObjectById(Ice::Identity id)
	throws ObjectNotFoundException;

    /**
     *
     * Find an adapter by id and return its proxy (a dummy direct
     * proxy created with the adapter).
     *
     * @param id The adapter id.
     *
     * @return The adapter proxy or null if the adapter is not active.
     * 
     * @throws AdapterNotFoundException Raised if the adapter can't be
     * found.
     *
     **/
    nonmutating Object* findAdapterById(string id)
	throws AdapterNotFoundException;

    /**
     *
     * Get the locator registry.
     *
     * @return The locator registry.
     *
     **/
    nonmutating LocatorRegistry* getRegistry();
};

/**
 *
 * The &Ice; locator registry interface. This interface is used by
 * servers to register adapter endpoints with the locator.
 *
 * <note><para> The [LocatorRegistry] interface is intended to be used
 * by &Ice; internals and by locator implementations. Regular user
 * code should not attempt to use any functionality of this interface
 * directly.</para></note>
 *
 **/
interface LocatorRegistry
{
    /**
     *
     * Set the adapter endpoints with the locator registry.
     *
     * @param id The adapter id.
     *
     * @param proxy The adapter proxy (a dummy direct proxy created
     * with the adapter). The direct proxy contains the adapter
     * endpoints.
     *
     * @throws AdapterNotFound Raised if the locator only allows
     * registered adapters to set their active proxy and if the
     * adapter is not registered with the locator.
     *
     * @throws AdapterAlreadyActive Raised if an adapter with the same
     * id is already active.
     *
     * @throws AdapterNotFoundException Raised if the adapter can't be
     * found.
     *
     */
    idempotent void setAdapterDirectProxy(string id, Object* proxy)
	throws AdapterNotFoundException, AdapterAlreadyActiveException;
};

};

#endif
