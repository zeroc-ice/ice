// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef GLACIER2_ROUTER_ICE
#define GLACIER2_ROUTER_ICE

#include <Ice/Router.ice>
#include <Glacier2/SessionF.ice>

/**
 *
 * &Glacier2; is a firewall solution for Ice. &Glacier2; authenticates
 * and filters client requests and allows callbacks to the client in a
 * secure fashion. In combination with &IceSSL;, &Glacier2; provides a
 * security solution that is both non-intrusive and easy to configure.
 *
 **/
module Glacier2
{

/**
 *
 * This exception is raised if a client is denied to create a session
 * with the router.
 *
 * @see Router::createSession
 *
 **/
exception PermissionDeniedException
{
    /**
     *
     * Details as to why session creation has been denied.
     *
     **/
    string reason;
};

/**
 *
 * This exception is raised if a client attempts to create a new
 * session, even though there is already an existing session for this
 * client.
 *
 * @see Router::createSession
 *
 **/
exception SessionExistsException
{
    /**
     *
     * The session that exists already for the client, or null if no
     * [SessionManager] is installed, and therefore the session is
     * Glacier2 internal only.
     *
     **/
    Session* existingSession;
};

/**
 *
 * This exception is raised if a client tries to destroy a session
 * with a router, but not session for the client exists.
 *
 * @see Router::destroySession
 *
 **/
exception SessionNotExistException
{
};

/**
 *
 * The &Glacier2; specialization of the standard &Ice; router
 * interface.
 *
 **/
interface Router extends Ice::Router
{
    /**
     *
     * Create a per-client session with the router. If a
     * [SessionManager] has been installed, a proxy to a [Session]
     * object is returned to the client. Otherwise, null is returned
     * and only an internal session (i.e., not visible to the client)
     * is created.
     *
     * @see Session
     * @see SessionManager
     * @see PermssionVerifier
     *
     * @return A proxy for the newly created session, or null if no
     * [SessionManager] has been installed.
     *
     * @param userId The user id for which to check the password.
     *
     * @param password The password for the given user id.
     *
     * @throws PermissionDeniedException Raised if the password for
     * the given user id is not correct, or if the user isn't allowed
     * access.
     *
     * @throws SessionExistsException Raised if there is already a
     * session for the calling client.
     *
     **/
    Session* createSession(string userId, string password)
	throws PermissionDeniedException, SessionExistsException;

    /**
     *
     * Destroy the calling client's session with this router.
     *
     * @throws SessionNotExistException Raised if no session exists
     * for the calling client.
     *
     **/
    void destroySession()
	throws SessionNotExistException;
};

};

#endif
