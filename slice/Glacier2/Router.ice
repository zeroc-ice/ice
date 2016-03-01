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

#include <Ice/Router.ice>
#include <Glacier2/Session.ice>
#include <Glacier2/PermissionsVerifier.ice>

/**
 *
 * Glacier2 is a firewall solution for Ice. Glacier2 authenticates
 * and filters client requests and allows callbacks to the client in a
 * secure fashion. In combination with IceSSL, Glacier2 provides a
 * security solution that is both non-intrusive and easy to configure.
 *
 **/
["objc:prefix:GLACIER2"]
module Glacier2
{

/**
 *
 * This exception is raised if a client tries to destroy a session
 * with a router, but no session exists for the client.
 *
 * @see Router#destroySession
 *
 **/
exception SessionNotExistException
{
};

/**
 *
 * The Glacier2 specialization of the {@link Ice.Router}
 * interface.
 *
 **/
interface Router extends Ice::Router
{
    /**
     *
     * This category must be used in the identities of all of the client's
     * callback objects. This is necessary in order for the router to
     * forward callback requests to the intended client. If the Glacier2
     * server endpoints are not set, the returned category is an empty
     * string.
     *
     * @return The category.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent string getCategoryForClient();

    /**
     *
     * Create a per-client session with the router. If a
     * {@link SessionManager} has been installed, a proxy to a {@link Session}
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
     * {@link SessionManager} has been installed.
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
    ["amd", "format:sliced"] Session* createSession(string userId, string password)
        throws PermissionDeniedException, CannotCreateSessionException;

    /**
     *
     * Create a per-client session with the router. The user is
     * authenticated through the SSL certificates that have been
     * associated with the connection. If a {@link SessionManager} has been
     * installed, a proxy to a {@link Session} object is returned to the
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
     * {@link SessionManager} has been installed.
     *
     * @throws PermissionDeniedException Raised if the user cannot be
     * authenticated or if the user is not allowed access.
     *
     * @throws CannotCreateSessionException Raised if the session
     * cannot be created.
     *
     **/
    ["amd", "format:sliced"] Session* createSessionFromSecureConnection()
        throws PermissionDeniedException, CannotCreateSessionException;

    /**
     *
     * Keep the calling client's session with this router alive.
     *
     * @throws SessionNotExistException Raised if no session exists
     * for the calling client.
     *
     **/
    ["amd"] void refreshSession()
        throws SessionNotExistException;

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

    /**
     *
     * Get the value of the ACM timeout. Clients supporting connection
     * heartbeats can enable them instead of explicitly sending keep
     * alives requests.
     *
     * NOTE: This method is only available since Ice 3.6.
     *
     * @return The timeout (in seconds).
     *
     **/
    ["nonmutating", "cpp:const"] idempotent int getACMTimeout();
};

};

