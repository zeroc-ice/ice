// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:GLACIER2_API"]]
[["cpp:doxygen:include:Glacier2/Glacier2.h"]]
[["cpp:header-ext:h"]]

[["cpp:include:Glacier2/Config.h"]]

[["java:package:com.zeroc"]]
[["js:module:@zeroc/ice"]]
[["python:pkgdir:Glacier2"]]

#include "Ice/Router.ice"
#include "PermissionsVerifier.ice"
#include "Session.ice"

/// Communicate through firewalls and across NATs.
module Glacier2
{
    /// This exception is raised if a client tries to destroy a session with a router, but no session exists for the
    /// client.
    /// @see Router#destroySession
    exception SessionNotExistException
    {
    }

    /// The Glacier2 specialization of the <code>Ice::Router</code> interface.
    interface Router extends Ice::Router
    {
        /// This category must be used in the identities of all of the client's callback objects. This is necessary in
        /// order for the router to forward callback requests to the intended client. If the Glacier2 server endpoints
        /// are not set, the returned category is an empty string.
        /// @return The category.
        ["cpp:const"] idempotent string getCategoryForClient();

        /// Create a per-client session with the router. If a {@link SessionManager} has been installed, a proxy to a
        /// {@link Session} object is returned to the client. Otherwise, null is returned and only an internal session
        /// (i.e., not visible to the client) is created.
        /// If a session proxy is returned, it must be configured to route through the router that created it. This will
        /// happen automatically if the router is configured as the client's default router at the time the session
        /// proxy is created in the client process, otherwise the client must configure the session proxy explicitly.
        /// @param userId The user id for which to check the password.
        /// @param password The password for the given user id.
        /// @return A proxy for the newly created session, or null if no {@link SessionManager} has been installed.
        /// @throws PermissionDeniedException Raised if the password for the given user id is not correct, or if the
        /// user is not allowed access.
        /// @throws CannotCreateSessionException Raised if the session cannot be created.
        /// @see Session
        /// @see SessionManager
        /// @see PermissionsVerifier
        ["amd"]
        Session* createSession(string userId, string password)
            throws PermissionDeniedException, CannotCreateSessionException;

        /// Create a per-client session with the router. The user is authenticated through the SSL certificates that
        /// have been associated with the connection. If a {@link SessionManager} has been installed, a proxy to a
        /// {@link Session} object is returned to the client. Otherwise, null is returned and only an internal session
        /// (i.e., not visible to the client) is created.
        /// If a session proxy is returned, it must be configured to route through the router that created it. This will
        /// happen automatically if the router is configured as the client's default router at the time the session
        /// proxy is created in the client process, otherwise the client must configure the session proxy explicitly.
        /// @see Session
        /// @see SessionManager
        /// @see PermissionsVerifier
        /// @return A proxy for the newly created session, or null if no {@link SessionManager} has been installed.
        /// @throws PermissionDeniedException Raised if the user cannot be authenticated or if the user is not allowed
        /// access.
        /// @throws CannotCreateSessionException Raised if the session cannot be created.
        ["amd"]
        Session* createSessionFromSecureConnection()
            throws PermissionDeniedException, CannotCreateSessionException;

        /// Keep the session with this router alive. This operation is provided for backward compatibility with Ice 3.7
        /// and earlier and does nothing in newer versions of Glacier2.
        /// @throws SessionNotExistException Raised if no session exists for the caller (client).
        void refreshSession()
            throws SessionNotExistException;

        /// Destroy the calling client's session with this router.
        /// @throws SessionNotExistException Raised if no session exists for the calling client.
        void destroySession()
            throws SessionNotExistException;

        /// Get the idle timeout used by the server-side of the connection.
        /// @return The idle timeout (in seconds).
        ["cpp:const"] idempotent long getSessionTimeout();

        /// Get the idle timeout used by the server-side of the connection.
        /// @return The idle timeout (in seconds).
        ["cpp:const"] idempotent int getACMTimeout();
    }
}
