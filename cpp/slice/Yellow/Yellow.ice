// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef YELLOW_ICE
#define YELLOW_ICE

#include <Ice/BuiltinSequences.ice>

module Yellow
{

/**
 *
 * This exception is raised in the case that an offer cannot be found
 * for a given interface.

 * @see Query
 *
 **/
exception NoSuchOfferException
{
    /** The interface to which there is no corresponding offer. */
    string intf;
};

/**
 *
 * The query interface for the yellow pages service.
 *
 * @see Admin
 *
 **/
interface Query
{
    /**
     *
     * Lookup an offer for the given interface.
     *
     * @param if The interface name to lookup.
     *
     * @return A proxy to the offer.
     *
     * @throws NoSuchOfferException Raised if no offer can be found
     * for the given interface type.
     *
     **/
    Object* lookup(string if)
	throws NoSuchOfferException;

    /**
     *
     * Lookup all offer for the given interface.
     *
     * @param if The interface name to lookup.
     *
     * @return A sequence of proxies to the offer.
     *
     * @throws NoSuchOfferException Raised if no offer can be found
     * for the given interface type.
     *
     **/
    Ice::ObjectProxySeq lookupAll(string if)
	throws NoSuchOfferException;
};

interface Admin
{
    /**
     *
     * Add an offer for the given interface type.
     *
     * @param if The interface name.
     *
     * @param offer The offer to add.
     *
     **/
    void add(string if, Object* offer);

    /**
     *
     * Remove an offer for the given interface type.
     *
     * @param if The interface name.
     *
     * @param offer The offer to remove.
     *
     **/
    void remove(string if, Object* offer)
	throws NoSuchOfferException;
};

};

#endif
