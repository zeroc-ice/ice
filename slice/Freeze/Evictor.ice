// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h", "objc:header-dir:objc"]]

#include <Ice/ObjectAdapterF.ice>
#include <Ice/ServantLocator.ice>
#include <Ice/Identity.ice>
#include <Freeze/Exception.ice>

module Freeze
{

/**
 *
 * A servant initializer provides the application with an
 * opportunity to perform custom servant initialization.
 *
 * @see Evictor
 *
 **/
local interface ServantInitializer
{
    /**
     *
     * Called whenever the evictor creates a new servant. This
     * operation allows application code to perform custom servant
     * initialization after the servant has been created by the
     * evictor and its persistent state has been restored.
     *
     * @param adapter The object adapter in which the evictor is
     * installed.
     *
     * @param identity The identity of the Ice object for which the
     * servant was created.
     *
     * @param facet The facet. An empty facet means the default
     * facet.
     *
     * @param servant The servant to initialize.
     *
     * @see Ice.Identity
     *
     **/
    void initialize(Ice::ObjectAdapter adapter, Ice::Identity identity, string facet, Object servant);
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
 * An iterator for the objects managed by the evictor.
 * Note that an EvictorIterator is not thread-safe: the application needs to
 * serialize access to a given EvictorIterator, for example by using it
 * in just one thread.
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
     * @return True if the iterator has more elements, false
     * otherwise.
     *
     * @throws DatabaseException Raised if a database failure
     * occurs while retrieving a batch of objects.
     *
     **/
    bool hasNext();

    /**
     *
     * Obtains the next identity in the iteration.
     *
     * @return The next identity in the iteration.
     *
     * @throws NoSuchElementException Raised if there is no further
     * elements in the iteration.
     *
     * @throws DatabaseException Raised if a database failure
     * occurs while retrieving a batch of objects.
     **/
    Ice::Identity next();
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
 * An automatic Ice object persistence manager, based on the
 * evictor pattern. The evictor is a servant locator implementation
 * that stores the persistent state of its objects in a database. Any
 * number of objects can be registered with an evictor, but only a
 * configurable number of servants are active at a time.  These active
 * servants reside in a queue; the least recently used servant in the
 * queue is the first to be evicted when a new servant is activated.
 *
 *
 * @see ServantInitializer
 *
 **/
local interface Evictor extends Ice::ServantLocator
{
  
    /**
     *
     * Set the size of the evictor's servant queue. This is the
     * maximum number of servants the evictor keeps active. Requests
     * to set the queue size to a value smaller than zero are ignored.
     *
     * @param sz The size of the servant queue. If the evictor
     * currently holds more than <tt>sz</tt> servants in its queue, it evicts
     * enough servants to match the new size. Note that this operation
     * can block if the new queue size is smaller than the current
     * number of servants that are servicing requests. In this case,
     * the operation waits until a sufficient number of servants
     * complete their requests.
     *
     *
     * @throws EvictorDeactivatedException Raised if a the evictor has
     * been deactivated.
     *
     * @see #getSize
     *
     **/
    void setSize(int sz);

    /**
     *
     * Get the size of the evictor's servant queue.
     *
     * @return The size of the servant queue.
     *
     * @throws EvictorDeactivatedException Raised if a the evictor has
     * been deactivated.
     *
     * @see #setSize
     *
     **/
    int getSize();


    /**
     *
     * Add a servant to this evictor. The state of the servant passed to 
     * this operation will be saved in the evictor's persistent store.
     *
     * @param servant The servant to add.
     *
     * @param id The identity of the Ice object that is implemented by 
     * the servant.
     *
     * @return A proxy that matches the given identity and this evictor's
     * object adapter.
     *
     * @throws Ice.AlreadyRegisteredException Raised if the evictor already has
     * an object with this identity.
     *
     * @throws DatabaseException Raised if a database failure occurred.
     *
     * @throws EvictorDeactivatedException Raised if the evictor has
     * been deactivated.
     *
     * @see #addFacet
     * @see #remove
     * @see #removeFacet
     *
     **/
    Object* add(Object servant, Ice::Identity id);

    /**
     *
     * Like {@link #add}, but with a facet. Calling <tt>add(servant, id)</tt>
     * is equivalent to calling {@link #addFacet} with an empty
     * facet.
     *
     * @param servant The servant to add.
     *
     * @param id The identity of the Ice object that is implemented by 
     * the servant.
     *
     * @param facet The facet. An empty facet means the default
     * facet.
     *
     * @return A proxy that matches the given identity and this evictor's
     * object adapter.
     *
     * @throws Ice.AlreadyRegisteredException Raised if the evictor already has
     * an object with this identity.
     *
     * @throws DatabaseException Raised if a database failure occurred.
     *
     * @throws EvictorDeactivatedException Raised if the evictor has
     * been deactivated.
     *
     * @see #add
     * @see #remove
     * @see #removeFacet
     *
     **/
    Object* addFacet(Object servant, Ice::Identity id, string facet);

    //
    // Note: no UUID operation as we don't know the category or 
    // categories this evictor was registered with.
    //

    /**
     *
     * Permanently destroy an Ice object.
     *
     * @param id The identity of the Ice object.
     *
     * @return The removed servant.
     *
     * @throws Ice.NotRegisteredException Raised if this identity was not 
     * registered with the evictor.
     *
     * @throws DatabaseException Raised if a database failure occurred.
     *
     * @throws EvictorDeactivatedException Raised if the evictor has
     * been deactivated.
     *
     * @see #add
     * @see #removeFacet
     *
     **/
    Object remove(Ice::Identity id);

    /**
     *
     * Like {@link #remove}, but with a facet. Calling <tt>remove(id)</tt> 
     * is equivalent to calling {@link #removeFacet} with an empty facet.
     *
     * @param id The identity of the Ice object.
     *
     * @param facet The facet. An empty facet means the default
     * facet.
     *
     * @return The removed servant.
     *
     * @throws Ice.NotRegisteredException Raised if this identity was not 
     * registered with the evictor.
     *
     * @throws DatabaseException Raised if a database failure occurred.
     *
     * @throws EvictorDeactivatedException Raised if the evictor has
     * been deactivated.
     *
     *
     * @see #remove
     * @see #addFacet
     *
     **/
    Object removeFacet(Ice::Identity id, string facet);

    /**
     *
     * Returns true if the given identity is managed by the evictor
     * with the default facet.
     *
     * @return true if the identity is managed by the evictor, false
     * otherwise.
     *
     * @throws DatabaseException Raised if a database failure occurred.
     *
     * @throws EvictorDeactivatedException Raised if a the evictor has
     * been deactivated.
     *
     **/
    bool hasObject(Ice::Identity id);

    /**
     * 
     * Like {@link #hasObject}, but with a facet. Calling <tt>hasObject(id)</tt>
     * is equivalent to calling {@link #hasFacet} with an empty
     * facet.
     *
     * @return true if the identity is managed by the evictor for the
     * given facet, false otherwise.
     *
     * @throws DatabaseException Raised if a database failure occurred.
     *
     * @throws EvictorDeactivatedException Raised if a the evictor has
     * been deactivated.
     *
     **/
    bool hasFacet(Ice::Identity id, string facet);  

    /**
     *
     * Get an iterator for the identities managed by the evictor.
     *
     * @param facet The facet. An empty facet means the default
     * facet.
     *
     * @param batchSize Internally, the Iterator retrieves the
     * identities in batches of size batchSize. Selecting a small batchSize
     * can have an adverse effect on performance.
     *
     * @return A new iterator.
     *
     * @throws EvictorDeactivatedException Raised if a the evictor has
     * been deactivated.
     *
     **/
    EvictorIterator getIterator(string facet, int batchSize);
};

};

