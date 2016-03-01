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

#include <Freeze/Evictor.ice>

module Freeze
{

/**
 * 
 * A background-save evictor is an evictor that saves updates 
 * asynchronously in a background thread.
 *
 **/

local interface BackgroundSaveEvictor extends Evictor
{
    /**
     *
     * Lock this object in the evictor cache. This lock can be released
     * by {@link #release} or {@link #remove}. {@link #release} releases only one lock, while
     * {@link #remove} releases all the locks.
     *
     * @param id The identity of the Ice object.
     *
     * @throws Ice.NotRegisteredException Raised if this identity was not 
     * registered with the evictor.
     *
     * @throws DatabaseException Raised if a database failure occurred.
     *
     * @see #keepFacet
     * @see #release
     * @see #remove
     *
     **/
     void keep(Ice::Identity id);

    /**
     *
     * Like {@link #keep}, but with a facet. Calling <tt>keep(id)</tt> 
     * is equivalent to calling {@link #keepFacet} with an empty facet.
     *
     * @param id The identity of the Ice object.
     *
     * @param facet The facet. An empty facet means the default
     * facet.
     *
     * @throws Ice.NotRegisteredException Raised if this identity was not 
     * registered with the evictor.
     *
     * @throws DatabaseException Raised if a database failure occurred.
     *
     * @see #keep
     * @see #releaseFacet
     * @see #removeFacet
     *
     **/
    void keepFacet(Ice::Identity id, string facet);


    /**
     *
     * Release a lock acquired by {@link #keep}. Once all the locks on an
     * object have been released, the object is again subject to the 
     * normal eviction strategy.
     *
     * @param id The identity of the Ice object.
     *
     * @throws Ice.NotRegisteredException Raised if this object was not
     * locked with {@link #keep} or {@link #keepFacet}.
     *
     * @see #keepFacet
     * @see #release
     *
     **/
    void release(Ice::Identity id);

    /**
     *
     * Like {@link #release}, but with a facet. Calling <tt>release(id)</tt> 
     * is equivalent to calling {@link #releaseFacet} with an empty facet.
     *
     * @param id The identity of the Ice object.
     *
     * @param facet The facet. An empty facet means the default
     * facet.
     *
     * @throws Ice.NotRegisteredException Raised if this object was not
     * locked with {@link #keep} or {@link #keepFacet}.
     *
     * @see #keep
     * @see #releaseFacet
     *
     **/
    void releaseFacet(Ice::Identity id, string facet);
};

};



