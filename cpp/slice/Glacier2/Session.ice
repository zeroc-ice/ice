// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef GLACIER2_SESSION_ICE
#define GLACIER2_SESSION_ICE

#include <Ice/BuiltinSequences.ice>
#include <Ice/Identity.ice>
#include <Glacier2/SSLInfo.ice>

module Glacier2
{

/**
 *
 * This exception is raised if an attempt to create a new session
 * failed.
 *
 * @see Router::createSession
 * @see SessionManager::createSession
 *
 **/
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
 * a [Router].
 *
 * @see Router
 * @see SessionManager
 *
 **/
interface Session
{
    /**
     *
     * Destroy the session. This is called automatically when the
     * [Router] is destroyed.
     *
     **/
    void destroy();
};

/**
 *
 * TODO: comments!
 *
 **/

interface StringSet
{
    idempotent void add(Ice::StringSeq additions);
    idempotent void remove(Ice::StringSeq deletions);
    idempotent Ice::StringSeq get();
};

interface IdentitySet
{
    idempotent void add(Ice::IdentitySeq additions);
    idempotent void remove(Ice::IdentitySeq deletions);
    idempotent Ice::IdentitySeq get();
};

/**
 *
 * An administrative session control object, which is tied to the
 * lifecycle of a [Session].
 *
 * @see Session
 *
 **/
interface SessionControl
{
    /**
     *
     * TODO: comments! 
     *
     **/
    StringSet* categories();

    StringSet* adapterIds();

    IdentitySet* identities();
    
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
 * is responsible for managing [Session] objects. New session objects
 * are created by the [Router] object calling on an application-provided
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
    Session* create(string userId, SessionControl* control)
	throws CannotCreateSessionException;
};

/**
 *
 * The session manager for SSL authenticated users that is
 * responsible for managing [Session] objects. New session objects are
 * created by the [Router] object calling on an application-provided
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
    Session* create(SSLInfo info, SessionControl* control)
	throws CannotCreateSessionException;
};

};

#endif
