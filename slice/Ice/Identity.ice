// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["ice-prefix", "cpp:header-ext:h", "cpp:dll-export:ICE_API", "cpp:doxygen:include:Ice/Ice.h", "objc:header-dir:objc", "objc:dll-export:ICE_API", "js:ice-build", "python:pkgdir:Ice"]]

#ifndef __SLICE2JAVA_COMPAT__
[["java:package:com.zeroc"]]
#endif

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
}

/**
 *
 * A mapping between identities and Ice objects.
 *
 **/
#ifndef __SLICE2PHP__
//
// dictionary key type not supported in PHP
//
local dictionary<Identity, Object> ObjectDict;
#endif

/**
 *
 * A sequence of identities.
 *
 **/
sequence<Identity> IdentitySeq;

}
