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

#ifndef ICE_EVICTOR_ICE
#define ICE_EVICTOR_ICE

#include <Ice/ServantLocator.ice>

module Ice
{

/**
 *
 * A servant locator that maintains a cache of servants in
 * least-recently-used order. Note that [Evictor] derives from
 * [ServantLocator].
 * The Ice run time provides an implementation of [Evictor] in
 * the class [EvictorBase]. [EvictorBase] implements the
 * [locate], [finished], and [deactivate] operations of [ServantLocator],
 * as well as the [setSize] and [getSize] operations of [Evictor].
 * The [instantiate] and [evict] operations are abstract operations that
 * must be implemented by the application developer.
 *
 * @see ServantLocator
 * @see ObjectAdapter
 * @see Identity
 *
 **/
local interface Evictor extends ServantLocator
{
    /**
     *
     * Change the size of the evictor cache. (Sizes less than 1
     * are ignored.) Reducing the size of the cache by [n] slots
     * causes eviction of the least-recently-used [n] servants.
     * (The actual eviction may be delayed because servants are evicted
     * only once they are idle.) Eviction is triggered by [locate] and [finished], that is,
     * the evictor attempts to evict servants whenever an operation invocation
     * starts or completes.) Once the evictor is deactivated, all servants in the cache are evicted.
     *
     * @param size The number of servants to hold in the cache.
     *
     **/
    idempotent void setSize(int size);

    /**
     *
     * Return the current size of the evictor cache.
     *
     * If no size has been set, the default size is determined
     * by the following properties:
     * <orderedlist>
     *
     * <listitem><para>[<replaceable>name</replaceable>.<replaceable>category</replaceable>.Evictor.Size]
     * determines the evictor size for the object adapter with <replaceable>name</replaceable> and the object
     * identity [category] <replaceable>category</replaceable>.</para></listitem>
     *
     * <listitem><para>[<replaceable>name</replaceable>.Evictor.Size] determines the evictor size for evictors
     * that are registered as a default servant locator and for evictors for a [category] for which no specific
     * property is set.</para></listitem>
     *
     * <listitem><para>[Ice.Evictor.Size] determines the evictor size for evictors to which neither of the preceding
     * two properties apply.</para></listitem>
     *
     * <listitem><para>If none of the preceding properties are set, the default evictor size is 1000.</para></listitem>
     *
     * </orderedlist>
     *
     **/
    nonmutating int getSize();

    /**
     *
     * [instantiate] is an abstract operation and is called by the [Evictor] when it
     * does not find an instantiated servant for an incoming request. [instantiate]
     * should instantiate and return a servant for the request. If no servant can
     * be instantiated, the operation can return null, which is propagated to the
     * client as an [ObjectNotExistException]. [instantiate] also can throw an exception.
     * The thrown exception is propagated to the client if it is [ObjectNotExistException],
     * [OperationNotExistException], or [FacetNotExistException]; all other exceptions
     * are returned to the client as [UnknownLocalException].
     *
     * @param curr The [Current] object for the incoming request.
     *
     * @param cookie This parameter allows [instantiate] to return a smart pointer
     * to an arbitrary object. The &Ice; run time does not examine the contents of
     * of the cookie, but passes the returned value to [evict] when it evicts the
     * servant again. This permits an arbitrary amount of state to be passed from
     * [instantiate] to [evict].
     *
     **/
    Object instantiate(Current curr, out LocalObject cookie);

    /**
     *
     * [evict] is an abstract operation and is called by the [Evictor] when it
     * evicts a servant from the cache.
     *
     * @param id The object identity of the evicted servant.
     *
     * @param servant The servant to be evicted.
     *
     * @param cookie The cookie that was returned from the corresponding call
     * to [instantiate].
     *
     * <note><para>[evict] must not throw any exceptions.</para></note>
     **/
    void evict(Identity id, Object servant, LocalObject cookie);
};

};

#endif
