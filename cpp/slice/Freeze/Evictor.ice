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

#include <Ice/ObjectAdapterF.ice>
#include <Ice/ServantLocator.ice>
#include <Freeze/DBException.ice>
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
     * @see Ice::Identity
     *
     **/
    void initialize(Ice::ObjectAdapter adapter, Ice::Identity identity, Object servant);
};

/**
 *
 * This exception is raised if there are no further elements in the
 * iteration.
 *
 **/
local exception NoSuchElementException
{
};

/**
 *
 * An iterator for Identities managed by the Evictor.
 *
 * @see Evictor
 *
 **/
local interface EvictorIterator
{
    /**
     *
     * Returns true if the iteration has more elements.
     *
     * @return true if the iterator has more elements, false
     * otherwise.
     *
     **/
    bool hasNext();

    /**
     *
     * Returns the next identity in the iteration.
     *
     * @returns The next identity in the iteration.
     *
     * @throws NoSuchElementException Raised if there is no further
     * elements in the iteration.
     *
     * @throws DBException Raised if any other database failure
     * occurred.
     *
     **/
    Ice::Identity next() throws DBException;
};

/**
 *
 * This exception is raised if the evictor has been deactivated.
 *
 **/
local exception EvictorDeactivatedException
{
};

/**
 *
 * A semi-automatic Ice Object persistence manager, based on the
 * evictor pattern. The Evictor is an extended Servant Locator, with
 * an implementation in the Freeze module. Instances of this
 * implementation can be created with the operation
 * [DB::createEvictor], and must be registered with an Object Adapter
 * like other Servant Locators.
 *
 * @see ServantInitializer
 *
 **/
local interface Evictor extends Ice::ServantLocator
{
    /**
     *
     * Get the Servant database that is being used by this Evictor. This is
     * the database from which this Evictor was created.
     *
     * @return The database used by this Evictor.
     *
     * @throws EvictorDeactivatedException Raised if a the evictor has
     * been deactivated.
     *
     * @see DB::createEvictor
     *
     **/
    DB getDB() throws EvictorDeactivatedException;

    /**
     *
     * Set the Evictor's Servant queue size. This is the maximum
     * number of idle Servants the Evictor will hold in
     * memory. Requests to set the queue size to a value smaller than
     * zero are ignored.
     *
     * @param sz The Evictor's Servant queue size. If the Evictor
     * currently holds more Servants in its queue, Servants will be
     * evicted until the number of Servants matches the new size. Note
     * that this operation can block if the new queue size is smaller
     * than the current number of active Servants. "Active Servant"
     * means a Servant for which an operation invocations is in
     * progress. In such case, this operation will wait until a
     * sufficient number of currently active Servants become
     * non-active.
     *
     * @throws DBException Raised if a database failure occurred.
     *
     * @throws EvictorDeactivatedException Raised if a the evictor has
     * been deactivated.
     *
     * @see getSize
     *
     **/
    void setSize(int sz) throws DBException, EvictorDeactivatedException;

    /**
     *
     * Get the Evictor's Servant queue size.
     *
     * @return The Evictor's Servant queue size.
     *
     * @throws EvictorDeactivatedException Raised if a the evictor has
     * been deactivated.
     *
     * @see setSize
     *
     **/
    int getSize() throws EvictorDeactivatedException;

    /**
     *
     * Create a new Ice Object for this Evictor. The state of the
     * initial Servant passed to this operation is put into the
     * Evictor's persistent store.
     *
     * @param identity The identity of the Ice Object to create.
     *
     * @param servant The initial Servant for the Ice Object to
     * create.
     *
     * @throws DBException Raised if a database failure occurred.
     *
     * @throws EvictorDeactivatedException Raised if a the evictor has
     * been deactivated.
     *
     * @see Ice::Identity
     * @see destroyObject
     *
     **/
    void createObject(Ice::Identity identity, Object servant) throws DBException, EvictorDeactivatedException;

    /**
     *
     * Permanently destroy an Ice Object by removing it from this
     * Evictor's persistent store. Furthermore, if the Evictor
     * currently holds a Servant for the Ice Object, such Servant will
     * be removed.
     *
     * @param identity The identity of the Ice Object to destroy.
     *
     * @throws DBException Raised if a database failure occurred.
     *
     * @throws EvictorDeactivatedException Raised if a the evictor has
     * been deactivated.
     *
     * @see Ice::Identity
     * @see createObject
     *
     **/
    void destroyObject(Ice::Identity identity) throws DBException, EvictorDeactivatedException;

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
     * @throws EvictorDeactivatedException Raised if a the evictor has
     * been deactivated.
     *
     **/
    void installServantInitializer(ServantInitializer initializer) throws EvictorDeactivatedException;

    /**
     *
     * Get an iterator over the Identities stored by the Evictor.
     *
     * @return A new iterator.
     *
     * @throws EvictorDeactivatedException Raised if a the evictor has
     * been deactivated.
     *
     **/
    EvictorIterator getIterator() throws EvictorDeactivatedException;

    /**
     *
     * Returns true if the given identity is in the Evictors
     * persistent store.
     *
     * @return true if the identity is in the Evictors persistent
     * store, false otherwise.
     *
     * @throws DBException Raised if a database failure occurred.
     *
     * @throws EvictorDeactivatedException Raised if a the evictor has
     * been deactivated.
     *
     **/
    bool hasObject(Ice::Identity ident) throws EvictorDeactivatedException, DBException;
};

};

#endif
