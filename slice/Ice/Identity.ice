//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["ice-prefix"]]

[["js:module:ice"]]

[["objc:dll-export:ICE_API"]]
[["objc:header-dir:objc"]]

[["python:pkgdir:Ice"]]

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
