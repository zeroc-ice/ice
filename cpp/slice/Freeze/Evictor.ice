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
#include <Freeze/DBF.ice>

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
 * The Evictor persistence mode.
 *
 * @see Evictor
 * @see Evictor::setPersistenceMode
 * @see Evictor::getPersistenceMode
 *
 **/
enum EvictorPersistenceMode
{
    /**
     *
     * This mode instructs the Evictor to save a Servant to persistent
     * store when it is evicted, or when the Evictor is deactivated.
     *
     * @see Ice::ServantLocator::deactivate
     *
     **/
    SaveUponEviction,

    /**
     *
     * This mode instructs the Evictor to save a Servant after each
     * mutating operation call. A mutating operation call is a call to
     * any operation that has not been declared as
     * <literal>nonmutating</literal>.
     *
     **/
    SaveAfterMutatingOperation
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
     * Get the database that is being used by this Evictor. This is
     * the database from which this Evictor was created.
     *
     * @return The database used by this Evictor.
     *
     * @see DB::createEvictor
     *
     **/
    DB getDB();

    /**
     *
     * Set the Evictor's Servant queue size. This is the maximum
     * number of Servants the Evictor will hold in memory. Requests to
     * set the queue size to a value smaller or equal to zero are
     * ignored.
     *
     * @param sz The Evictor's Servant queue size. If the Evictor
     * currently holds more Servants in its queue, Servants will be
     * evicted until the number of Servants match the new size.
     *
     * @see getSize
     *
     **/
    void setSize(int sz);

    /**
     *
     * Get the Evictor's Servant queue size.
     *
     * @return The Evictor's Servant queue size.
     *
     * @see setSize
     *
     **/
    int getSize();

    /**
     *
     * Set the Evictor's persistence mode. If the mode is change to
     * <literal>SaveAfterMutatingOperation</literal>, all Servants
     * that are currently in the Evictor's Servant queue are saved to
     * persistent store, so that it can safely be assumed that no data
     * can get lost after the mode has been changed.
     *
     * @param mode The Evictor's persistence mode.
     *
     * @see EvictorPersistenceMode
     * @see getPersistenceMode
     *
     **/
    void setPersistenceMode(EvictorPersistenceMode mode);

    /**
     *
     * Get the Evictor's perstence mode.
     *
     * @return The Evictor's persistence mode.
     *
     * @see EvictorPersistenceMode
     * @see setPersistenceMode
     *
     **/
    EvictorPersistenceMode getPersistenceMode();

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
