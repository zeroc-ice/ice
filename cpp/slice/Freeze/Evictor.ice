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
     **/
    void initialize(Ice::ObjectAdapter adapter, string identity, Object servant);
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
     * @see DB::createEvictor
     *
     **/
    DB getDB();

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
     * @see getSize
     *
     **/
    void setSize(int sz) throws DBException;

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
     * @see destroyObject
     *
     **/
    void createObject(string identity, Object servant) throws DBException;

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
     * @see createObject
     *
     **/
    void destroyObject(string identity) throws DBException;

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
