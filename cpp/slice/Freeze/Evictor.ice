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
};

};

#endif
