// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PACK_QUERY_ICE
#define ICE_PACK_QUERY_ICE

#include <Ice/Identity.ice>
#include <Ice/BuiltinSequences.ice>

#include <IcePack/Exception.ice>

/**
 *
 * &IcePack; is a server activation and deployment tool. &IcePack;,
 * simplifies the complex task of deploying applications in a
 * heterogenous computer network.
 *
 **/
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
