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

module Ice
{

interface LocatorRegistry;

/**
 *
 * The &Ice; locator interface. This interface is used by clients to
 * lookup adapters and by servers to get the locator registry.
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
     * Find an adapter by name and return the adapter proxy (a dummy
     * direct proxy created with the adapter).
     *
     * @param name The adapter name.
     *
     * @return The adapter proxy or null if the adapter is not found.
     *
     **/
    nonmutating Object* findAdapterByName(string name);

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
 * This exception is raised if the server tries to set endpoints for
 * an adapter which is not registered with the locator.
 *
 **/
exception AdapterNotRegistered
{
};

/**
 *
 * This exception is raised if the server tries to set endpoints for
 * an adapter which is already active.
 *
 **/
exception AdapterAlreadyActive
{
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
     * @param name The adapter name.
     *
     * @param proxy The adapter proxy (a dummy direct proxy created
     * with the adapter). The direct proxy contains the adapter
     * endpoints.
     *
     * @throws AdapterNotRegistered Raised if the locator only allows
     * registered adapters to set their active proxy and if the
     * adapter is not registered with the locator.
     *
     * @throws AdapterAlreadyActive Raised if an adapter with the same
     * name is already active.
     *
     */
    idempotent void setAdapterDirectProxy(string name, Object* proxy)
	throws AdapterNotRegistered, AdapterAlreadyActive;
};

};

#endif
