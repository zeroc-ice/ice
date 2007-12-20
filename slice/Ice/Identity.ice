// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_IDENTITY_ICE
#define ICE_IDENTITY_ICE

module Ice
{

/**
 *
 * The identity of an Ice object. An empty [name] denotes a null
 * object.
 *
 * @see ServantLocator
 * @see ObjectAdapter::addServantLocator
 *
 **/
struct Identity
{
    /**
     *
     * The name of the Ice object.
     *
     **/
    string name;

    /**
     *
     * The Ice object category.
     *
     **/
    string category;
};

/**
 *
 * A mapping between identities and Ice objects.
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
