// **********************************************************************
//
// Copyright (c) 2003
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
 * A servant initializer is installed in an Evictor and provides the
 * application with an opportunity to perform custom servant
 * initialization.
 *
 * @see Evictor
 *
 **/
local interface ServantInitializer
{
    /**
     *
     * Called whenever the Evictor creates a new servant. This
     * operation allows application code to perform custom servant
     * initialization after the servant has been created by the
     * Evictor and its persistent state has been restored.
     *
     * @param adapter The ObjectAdapter in which the Evictor is installed.
     *
     * @param identity The identity of the &Ice; object for which the
     * servant was created.
     *
     * @param servant The servant to initialize.
     *
     * @see Ice::Identity
     *
     **/
    void initialize(Ice::ObjectAdapter adapter, Ice::Identity identity, Object servant);
};

/**
 *
 * This exception is raised if there are no further elements in the iteration.
 *
 **/
local exception NoSuchElementException
{
};

/**
 *
 * An iterator for the identities managed by the Evictor.
 *
 * @see Evictor
 *
 **/
local interface EvictorIterator
{
    /**
     *
     * Determines if the iteration has more elements.
     *
     * @return true if the iterator has more elements, false
     * otherwise.
     *
     **/
    bool hasNext();

    /**
     *
     * Obtains the next identity in the iteration.
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

    /**
     *
     * Destroy the iterator. Once the iterator has been destroyed it
     * may no longer be accessed.
     *
     **/
    void destroy();
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
 * A semi-automatic &Ice; object persistence manager, based on the
 * evictor pattern. The Evictor is a ServantLocator implementation
 * that stores the persistent state of its objects in a database.
 * Any number of objects can be registered with an Evictor, but
 * only a configurable number of servants are active at a time. 
 * These active servants reside in a queue; the least recently
 * used servant in the queue is the first to be evicted when a new
 * servant is activated.
 * <para>
 * An Evictor is created using the operation [DB::createEvictor] and
 * must be registered with an object adapter just like any other
 * ServantLocator.</para>
 *
 * @see ServantInitializer
 *
 **/
local interface Evictor extends Ice::ServantLocator
{
    /**
     *
     * Get the database object that created this Evictor, in which
     * the Evictor stores the persistent state of its objects.
     *
     * @return The database used by this Evictor.
     *
     * @throws EvictorDeactivatedException Raised if the evictor has
     * been deactivated.
     *
     * @see DB::createEvictor
     *
     **/
    DB getDB() throws EvictorDeactivatedException;

    /**
     *
     * Set the size of the Evictor's servant queue. This is the maximum
     * number of servants the Evictor keeps active. Requests to set the
     * queue size to a value smaller than zero are ignored.
     *
     * @param sz The size of the servant queue. If the Evictor
     * currently holds more than [sz] servants in its queue, it evicts
     * enough servants to match the new size. Note that this operation
     * can block if the new queue size is smaller than the current number
     * of servants that are servicing requests. In this case, the operation
     * waits until a sufficient number of servants complete their requests.
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
     * Get the size of the Evictor's servant queue.
     *
     * @return The size of the servant queue.
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
     * Create a new &Ice; object for this Evictor. The state of the
     * servant passed to this operation is saved in the Evictor's
     * persistent store.
     *
     * @param identity The identity of the &Ice; object to create.
     *
     * @param servant The servant for the &Ice; object.
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
     * Permanently destroy an &Ice; object by removing it from the
     * Evictor's persistent store. Furthermore, if a servant is
     * currently active for the &Ice; object, it will be removed.
     *
     * @param identity The identity of the &Ice; object to destroy.
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
     * Install a servant initializer for this Evictor.
     *
     * @param initializer The servant initializer to install.
     * Subsequent calls overwrite any previously set value. A null value
     * removes an existing servant initializer.
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
     * Get an iterator for the identities managed by the Evictor.
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
     * Returns true if the given identity is managed by the Evictor.
     *
     * @return true if the identity is managed by the Evictor,
     * false otherwise.
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
