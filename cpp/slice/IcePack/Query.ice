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

#ifndef ICE_PACK_QUERY_ICE
#define ICE_PACK_QUERY_ICE

#include <Ice/Identity.ice>
#include <Ice/BuiltinSequences.ice>

#include <IcePack/Exception.ice>

module IcePack
{

/**
 *
 * The &IcePack; query interface. This interface is accessible to
 * &Ice; clients who wish to lookup objects.
 *
 **/
interface Query
{
    /**
     *
     * Find an object by identity.
     *
     * @param id The identity.
     *
     * @return The proxy.
     *
     * @throws ObjectNotExistException Raised if no objects can be
     * found.
     *
     **/
    nonmutating Object* findObjectById(Ice::Identity id)
	throws ObjectNotExistException;

    /**
     *
     * Find an object by type.
     *
     * @param type The object type.
     *
     * @return The proxy.
     *
     * @throws ObjectNotExistException Raised if no objects can be
     * found.
     *
     **/
    nonmutating Object* findObjectByType(string type)
	throws ObjectNotExistException;

    /**
     *
     * Find all the objects with the given type.
     *
     * @param type The object type.
     *
     * @return The proxies.
     *
     * @throws ObjectNotExistException Raised if no objects can be
     * found.
     *
     **/
    nonmutating Ice::ObjectProxySeq findAllObjectsWithType(string type)
	throws ObjectNotExistException;
};

};

#endif
