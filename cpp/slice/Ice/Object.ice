// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

//
// This file is not used in the Ice core. It is only for documentation
// purposes, and for tools such as <literal>slice2pythonext</literal>.
//

#ifndef ICE_OBJECT_ICE
#define ICE_OBJECT_ICE

module Ice
{

/**
 *
 * The base interface for all objects in Ice.
 *
 * @see Ice::LocalObject
 *
 **/
interface \Object
{
    /**
     *
     * Check whether the object is of a certain type.
     *
     * @param id The type id, which is the same as the fully qualified
     * Slice type name.
     *
     * @return True, if the object is of the specified type, or false
     * otherwise.
     *
     **/
    nonmutating bool ice_isA(string id);

    /**
     *
     * Check if an object is alive.
     *
     **/
    nonmutating void ice_ping();
};

};

#endif
