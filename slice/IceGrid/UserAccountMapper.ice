// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h", "objc:header-dir:objc", "js:ice-build"]]
[["cpp:include:IceGrid/Config.h"]]

["objc:prefix:ICEGRID"]
module IceGrid
{

/**
 *
 * This exception is raised if a user account for a given session
 * identifier can't be found.
 *
 **/
exception UserAccountNotFoundException
{
};

/**
 *
 * A user account mapper object is used by IceGrid nodes to map
 * session identifiers to user accounts.
 *
 **/
interface UserAccountMapper
{
    /**
     *
     * Get the name of the user account for the given user. This is
     * used by IceGrid nodes to figure out the user account to use
     * to run servers.
     *
     * @param user The value of the server descriptor's <tt>user</tt>
     * attribute. If this attribute is not defined, and the server's
     * activation mode is <tt>session</tt>, the default value of
     * <tt>user</tt> is the session identifier.
     *
     * @return The user account name.
     *
     * @throws UserAccountNotFoundException Raised if no user account
     * is found for the given user.
     *
     **/
    string getUserAccount(string user)
        throws UserAccountNotFoundException;
};

};

