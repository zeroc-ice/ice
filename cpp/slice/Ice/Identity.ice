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

#ifndef ICE_IDENTITY_ICE
#define ICE_IDENTITY_ICE

module Ice
{

/**
 *
 * The identity of an &Ice; object. An empty [name] denotes a null
 * object.
 *
 **/
struct Identity
{
    /**
     *
     * The name of the &Ice; object.
     *
     **/
    string name;

    /**
     *
     * The &Ice; object category.
     *
     * @see ServantLocator
     * @see ObjectAdapter::addServantLocator
     *
     **/
    string category;
};

/**
 *
 * A mapping between identities and &Ice; objects.
 *
 **/
local dictionary<Identity, Object> ObjectDict;

/**
 *
 * A sequence of identities.
 *
 **/
sequence<Identity> IdentitySeq;

};

#endif
