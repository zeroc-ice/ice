// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
 * The &Ice; locator registry interface. This interface is used by
 * servers to register adapters with the locator.
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
     * Add the adapter and its endpoints to the locator registry.
     *
     * @param name The adapter name.
     *
     * @param proxy The adapter proxy (a dummy direct proxy created
     * with the adapter).
     *
     */
    idempotent void addAdapter(string name, Object* proxy);
};

};

#endif
