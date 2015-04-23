// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h", "objc:header-dir:objc", "js:ice-build"]]

["objc:prefix:ICE"]
module Ice
{

/**
 *
 * The identity of an Ice object. In a proxy, an empty {@link Identity#name} denotes a nil
 * proxy. An identity with an empty {@link Identity#name} and a non-empty {@link Identity#category}
 * is illegal. You cannot add a servant with an empty name to the Active Servant Map.
 *
 * @see ServantLocator
 * @see ObjectAdapter#addServantLocator
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

