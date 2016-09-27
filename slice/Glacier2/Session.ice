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
[["cpp:include:Glacier2/Config.h"]]

#include <Ice/BuiltinSequences.ice>
#include <Ice/Identity.ice>
#include <Glacier2/SSLInfo.ice>

["objc:prefix:GLACIER2"]
module Glacier2
{

/**
 *
 * This exception is raised if an attempt to create a new session failed.
 *
 **/
["preserve-slice"]
exception CannotCreateSessionException
{
    /**
     *
     * The reason why session creation has failed.
     *
     **/
    string reason;
};

/**
 *
 * A client-visible session object, which is tied to the lifecycle of
 * a {@link Router}.
 *
 * @see Router
 * @see SessionManager
 *
 **/
interface Session
{
    /**
     *
     * Destroy the session. This is called automatically when the router is destroyed.
     *
     **/
    void destroy();
};

/**
 *
 * An object for managing the set of identity constraints for specific
 * parts of object identity on a
 * {@link Session}.
 *
 * @see Session
 * @see SessionControl
 *
 **/
interface StringSet
{
    /**
     *
     * Add a sequence of strings to this set of constraints. Order is
     * not preserved and duplicates are implicitly removed.
     *
     * @param additions The sequence of strings to be added.
     *
     **/
    idempotent void add(Ice::StringSeq additions);

    /**
     *
     * Remove a sequence of strings from this set of constraints. No
     * errors are returned if an entry is not found.
     *
     * @param deletions The sequence of strings to be removed.
     *
     **/
    idempotent void remove(Ice::StringSeq deletions);

    /**
     *
     * Returns a sequence of strings describing the constraints in this
     * set.
     *
     * @return The sequence of strings for this set.
     *
     **/
    idempotent Ice::StringSeq get();
};

/**
 *
 * An object for managing the set of object identity constraints on a
 * {@link Session}.
 *
 * @see Session
 * @see SessionControl
 *
 **/
interface IdentitySet
{
    /**
     *
     * Add a sequence of Ice identities to this set of constraints. Order is
     * not preserved and duplicates are implicitly removed.
     *
     * @param additions The sequence of Ice identities to be added.
     *
     **/
    idempotent void add(Ice::IdentitySeq additions);

    /**
     *
     * Remove a sequence of identities from this set of constraints. No
     * errors are returned if an entry is not found.
     *
     * @param deletions The sequence of Ice identities to be removed.
     *
     **/
    idempotent void remove(Ice::IdentitySeq deletions);

    /**
     *
     * Returns a sequence of identities describing the constraints in this
     * set.
     *
     * @return The sequence of Ice identities for this set.
     *
     **/
    idempotent Ice::IdentitySeq get();
};

/**
 *
 * An administrative session control object, which is tied to the
 * lifecycle of a {@link Session}.
 *
 * @see Session
 *
 **/
interface SessionControl
{
    /**
     *
     * Access the object that manages the allowable categories
     * for object identities for this session.
     *
     * @return A StringSet object.
     *
     **/
    StringSet* categories();

    /**
     *
     * Access the object that manages the allowable adapter identities
     * for objects for this session.
     *
     * @return A StringSet object.
     *
     **/
    StringSet* adapterIds();

    /**
     *
     * Access the object that manages the allowable object identities
     * for this session.
     *
     * @return An IdentitySet object.
     *
     **/
    IdentitySet* identities();

    /**
     *
     * Get the session timeout.
     *
     * @return The timeout.
     *
     **/
    idempotent int getSessionTimeout();

    /**
     *
     * Destroy the associated session.
     *
     **/
    void destroy();
};

/**
 *
 * The session manager for username/password authenticated users that
 * is responsible for managing {@link Session} objects. New session objects
 * are created by the {@link Router} object calling on an application-provided
 * session manager. If no session manager is provided by the application,
 * no client-visible sessions are passed to the client.
 *
 * @see Router
 * @see Session
 *
 **/
interface SessionManager
{
    /**
     *
     * Create a new session.
     *
     * @param userId The user id for the session.
     *
     * @param control A proxy to the session control object.
     *
     * @return A proxy to the newly created session.
     *
     * @throws CannotCreateSessionException Raised if the session
     * cannot be created.
     *
     **/
    ["format:sliced"]
    Session* create(string userId, SessionControl* control)
        throws CannotCreateSessionException;
};

/**
 *
 * The session manager for SSL authenticated users that is
 * responsible for managing {@link Session} objects. New session objects are
 * created by the {@link Router} object calling on an application-provided
 * session manager. If no session manager is provided by the
 * application, no client-visible sessions are passed to the client.
 *
 * @see Router
 * @see Session
 *
 **/
interface SSLSessionManager
{
    /**
     *
     * Create a new session.
     *
     * @param info The SSL info.
     *
     * @param control A proxy to the session control object.
     *
     * @return A proxy to the newly created session.
     *
     * @throws CannotCreateSessionException Raised if the session
     * cannot be created.
     *
     **/
    ["format:sliced"]
    Session* create(SSLInfo info, SessionControl* control)
        throws CannotCreateSessionException;
};

};

