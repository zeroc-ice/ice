// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef FREEZE_EVICTOR_ICE
#define FREEZE_EVICTOR_ICE

#include <Ice/ObjectAdapter.ice>

module Freeze
{

/**
 *
 * Servant Initializers for Evictors. Servant Initializers are
 * installed with Evictors and provide a callback to application code
 * for custom Servant initialization.
 *
 * @see Evictor
 * @see Evictor::installServantInitializer
 *
 **/
local interface ServantInitializer
{
    /**
     *
     * Called whenever the Evictor creates a new Servant. This
     * operation allows application code to perform custom Servant
     * initialization after the Servant has been created by the
     * Evictor and it has been loaded from persistent store.
     *
     * @param adapter The ObjectAdapter the Evictor is installed with.
     *
     * @param identity The identity of the Ice Object for which the
     * Servant was created.
     *
     * @param servant The Servant to set up.
     *
     **/
    void initialize(Ice::ObjectAdapter adapter, string identity, Object servant);
};

/**
 *
 * A semi-automatic Ice Object persistence manager, based on the
 * evictor pattern. The Evictor is an extended Servant Locator, with
 * an implementation in the Freeze module. Instances of this
 * implementation can be created with the operation
 * <literal>DB::createEvictor</literal>, and must be registered with
 * an Object Adapter like other Servant Locators.
 *
 * @see DB::createEvictor
 * @see Ice::ServantLocator
 *
 **/
local interface Evictor extends Ice::ServantLocator
{
    /**
     *
     * Set the Evictor Servant queue size. This is the number of
     * Servants the Evictor will hold in memory. Queue sizes smaller
     * than one are illegal and result in undefined behavior.
     *
     * @param sz The Evictor Servant queue size. If the Evictor
     * currently holds more Servants in its queue, Servants will be
     * evicted until the number of Servants match the new size.
     *
     * @see getSize
     *
     **/
    void setSize(int sz);

    /**
     *
     * Get the Evictor Servant queue size.
     *
     * @return The Evictor Servant queue size.
     *
     * @see setSize
     *
     **/
    int getSize();

    /**
     *
     * Create a new Ice Object for this Evictor. The state of the
     * initial Servant passed to this operation is put into this
     * Evictor's persistent store. Furthermore, the initial Servant is
     * added to the head of the Evictor queue, so that it will be
     * evicted last.
     *
     * @param identity The identity of the Ice Object to create.
     *
     * @param servant The initial Servant for the Ice Object to
     * create.
     *
     * @see destroyObject
     *
     **/
    void createObject(string identity, Object servant);

    /**
     *
     * Permanently destroy an Ice Object by removing it from this
     * Evictor's persistent store. Furthermore, if the Evictor
     * currently holds a Servant for the Ice Object, such Servant will
     * be removed.
     *
     * @param identity The identity of the Ice Object to destroy.
     *
     * @see createObject
     *
     **/
    void destroyObject(string identity);

    /**
     *
     * Install a Servant Initializer with this Evictor.
     *
     * @param initializer The Servant Initializer to install with this
     * Evictor. Subsequent calls overwrite any perviously set
     * value. Null can be used as argument to reset the Evictor's
     * Servant Initializer.
     *
     * @see ServantInitializer
     *
     **/
    void installServantInitializer(ServantInitializer initializer);
};

};

#endif
