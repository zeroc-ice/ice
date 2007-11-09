// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef GLACIER2_ROUTER_ICE
#define GLACIER2_ROUTER_ICE

#include <Ice/Router.ice>
#include <Glacier2/Session.ice>

/**
 *
 * Glacier2 is a firewall solution for Ice. Glacier2 authenticates
 * and filters client requests and allows callbacks to the client in a
 * secure fashion. In combination with IceSSL, Glacier2 provides a
 * security solution that is both non-intrusive and easy to configure.
 *
 **/
module Glacier2
{

/**
 *
 * This exception is raised if a client is denied the ability to create
 * a session with the router.
 *
 * @see Router::createSession
 * @see Router::createSessionFromSecureConnection
 *
 **/
exception PermissionDeniedException
{
    /**
     *
     * The reason why permission was denied.
     *
     **/
    string reason;
};

/**
 *
 * This exception is raised if a client tries to destroy a session
 * with a router, but no session exists for the client.
 *
 * @see Router::destroySession
 *
 **/
exception SessionNotExistException
{
};

/**
 *
 * The Glacier2 specialization of the [Ice::Router]
 * interface.
 *
 **/
interface Router extends Ice::Router
{
    /**
     *
     * This category must be used in the identities of all of the client's
     * callback objects. This is necessary in order for the router to
     * forward callback requests to the intended client.
     *
     * @return The category.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent string getCategoryForClient();

    /**
     *
     * Create a per-client session with the router. If a
     * [SessionManager] has been installed, a proxy to a [Session]
     * object is returned to the client. Otherwise, null is returned
     * and only an internal session (i.e., not visible to the client)
     * is created.
     *
     * If a session proxy is returned, it must be configured to route
     * through the router that created it. This will happen automatically
     * if the router is configured as the client's default router at the
     * time the session proxy is created in the client process, otherwise
     * the client must configure the session proxy explicitly.
     *
     * @see Session
     * @see SessionManager
     * @see PermissionsVerifier
     *
     * @return A proxy for the newly created session, or null if no
     * [SessionManager] has been installed.
     *
     * @param userId The user id for which to check the password.
     *
     * @param password The password for the given user id.
     *
     * @throws PermissionDeniedException Raised if the password for
     * the given user id is not correct, or if the user is not allowed
     * access.
     *
     * @throws CannotCreateSessionException Raised if the session
     * cannot be created.
     *
     **/
    Session* createSession(string userId, string password)
        throws PermissionDeniedException, CannotCreateSessionException;

    /**
     *
     * Create a per-client session with the router. The user is
     * authenticated through the SSL certificates that have been
     * associated with the connection. If a [SessionManager] has been
     * installed, a proxy to a [Session] object is returned to the
     * client. Otherwise, null is returned and only an internal
     * session (i.e., not visible to the client) is created.
     *
     * If a session proxy is returned, it must be configured to route
     * through the router that created it. This will happen automatically
     * if the router is configured as the client's default router at the
     * time the session proxy is created in the client process, otherwise
     * the client must configure the session proxy explicitly.
     *
     * @see Session
     * @see SessionManager
     * @see PermissionsVerifier
     *
     * @return A proxy for the newly created session, or null if no
     * [SessionManager] has been installed.
     *
     * @throws PermissionDeniedException Raised if the user cannot be
     * authenticated or if the user is not allowed access.
     *
     * @throws CannotCreateSessionException Raised if the session
     * cannot be created.
     *
     **/
    Session* createSessionFromSecureConnection()
        throws PermissionDeniedException, CannotCreateSessionException;

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

    /**
     *
     * Get the value of the session timeout. Sessions are destroyed
     * if they see no activity for this period of time.
     *
     * @return The timeout (in seconds).
     *
     **/
    ["nonmutating", "cpp:const"] idempotent long getSessionTimeout();
};

/**
 *
 * The Glacier2 administrative interface. This must only be
 * accessible from inside the firewall.
 *
 **/
interface Admin
{
    /**
     *
     * Shut down the Glacier2 router.
     *
     **/
    void shutdown();
};

};

#endif
