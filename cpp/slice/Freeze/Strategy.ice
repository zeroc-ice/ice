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

#ifndef FREEZE_STRATEGY_ICE
#define FREEZE_STRATEGY_ICE

#include <Ice/Identity.ice>
#include <Freeze/DBException.ice>

module Freeze
{

/**
 *
 * Provides a means for persistence strategy implementations to
 * save an object's persistent state in the evictor's database.
 *
 * @see PersistenceStrategy
 *
 **/
local interface ObjectStore
{
    /**
     *
     * Saves an object's persistent state. It is not safe to invoke
     * this operation for a servant with pending mutating operations.
     *
     * @param ident The identity of the &Ice; object.
     *
     * @param servant The servant incarnating the &Ice; object.
     *
     * @throws DBException Raised if a database failure occurred.
     *
     **/
    void save(Ice::Identity ident, Object servant) throws DBException;
};

/**
 *
 * A persistence strategy determines when the evictor saves the
 * persistent state of an object.
 *
 * @see Evictor
 *
 **/
local interface PersistenceStrategy
{
    /**
     *
     * Called after the evictor activates a servant for an &Ice; object.
     * This occurs when the object is first registered with the evictor,
     * as well as when the evictor restores the object from persistent
     * storage. If the evictor has a servant initializer, the initializer
     * is invoked before [activatedObject].
     *
     * @param ident The identity of the &Ice; object.
     *
     * @param servant The servant incarnating the &Ice; object.
     *
     * @return A local object representing private state that the
     * strategy implementation associates with the &Ice; object. This
     * "cookie" is supplied to the strategy in all subsequent
     * invocations.
     *
     * @see Evictor::createObject
     *
     **/
    LocalObject activatedObject(Ice::Identity ident, Object servant);

    /**
     *
     * Called after the evictor destroys an object.
     *
     * @param ident The identity of the &Ice; object.
     *
     * @param cookie The strategy's private state associated with the &Ice; object.
     *
     * @see Evictor::destroyObject
     *
     **/
    void destroyedObject(Ice::Identity ident, LocalObject cookie);

    /**
     *
     * Called after the evictor has evicted a servant. The
     * persistence strategy typically saves the object at this
     * point, or may only save the object if it has been
     * modified.
     *
     * @param store The object store. This value may be retained
     * by the strategy if necessary.
     *
     * @param ident The identity of the &Ice; object.
     *
     * @param servant The servant incarnating the &Ice; object.
     *
     * @param cookie The strategy's private state associated with the &Ice; object.
     *
     * @see Evictor::destroyObject
     *
     **/
    void evictedObject(ObjectStore store, Ice::Identity ident, Object servant, LocalObject cookie) throws DBException;

    /**
     *
     * Called before an operation is invoked on an object.
     *
     * @param store The object store. This value may be retained
     * by the strategy if necessary.
     *
     * @param identity The identity of the &Ice; object.
     *
     * @param servant The servant incarnating the &Ice; object.
     *
     * @param mutating True if the operation was not declared as nonmutating, otherwise false.
     *
     * @param cookie The strategy's private state associated with the &Ice; object.
     *
     **/
    void preOperation(ObjectStore store, Ice::Identity ident, Object servant, bool mutating, LocalObject cookie)
        throws DBException;

    /**
     *
     * Called after an operation has been invoked on an object.
     *
     * @param store The object store. This value may be retained
     * by the strategy if necessary.
     *
     * @param identity The identity of the &Ice; object.
     *
     * @param servant The servant incarnating the &Ice; object.
     *
     * @param mutating True if the operation was not declared as nonmutating, otherwise false.
     *
     * @param cookie The strategy's private state associated with the &Ice; object.
     *
     **/
    void postOperation(ObjectStore store, Ice::Identity ident, Object servant, bool mutating, LocalObject cookie)
        throws DBException;

    /**
     *
     * Called when the evictor is deactivated and there are no
     * outstanding requests on the evictor's servants.
     *
     **/
    void destroy();
};

/**
 *
 * Saves the persistent state of an object after eviction of the servant
 * or deactivation of the evictor, but only if the object has been modified
 * by a mutating operation since the last time it was saved. A mutating
 * operation is one that is not defined as nonmutating.
 *
 **/
local interface EvictionStrategy extends PersistenceStrategy
{
};

/**
 *
 * Saves the persistent state of an object after the invocation of a
 * mutating operation, but only when the object becomes idle (i.e., has
 * no pending mutating operations). A mutating operation is one that is
 * not defined as nonmutating.
 *
 **/
local interface IdleStrategy extends PersistenceStrategy
{
};

};

#endif
