// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:GLACIER2_API"]]
[["cpp:doxygen:include:Glacier2/Glacier2.h"]]
[["cpp:header-ext:h"]]

[["cpp:include:Glacier2/Config.h"]]

[["js:module:@zeroc/ice"]]
[["python:pkgdir:Glacier2"]]

#include "Ice/Router.ice"
#include "PermissionsVerifier.ice"
#include "Session.ice"

/// Communicate through firewalls and across NATs.
["java:identifier:com.zeroc.Glacier2"]
module Glacier2
{
    /// The exception that is thrown when a client tries to destroy a session with a router, but no session exists for
    /// this client.
    /// @see Router#destroySession
    exception SessionNotExistException
    {
    }

    /// The Glacier2 specialization of the {@link Ice::Router} interface.
    interface Router extends Ice::Router
    {
        /// Gets a unique category that identifies the client (caller) in the router. This category must be used in the
        /// identities of all the client's callback objects.
        /// @return The category. It's an empty string when `Glacier2.Server.Endpoints` is not configured.
        ["cpp:const"]
        idempotent string getCategoryForClient();

        /// Creates a session for the client (caller) with the router. If a {@link SessionManager} is configured,
        /// a proxy to a {@link Session} object is returned to the client. Otherwise, null is returned and only an
        /// internal session (i.e., not visible to the client) is created.
        /// If a non-null session proxy is returned, it must be configured to route through the router that created it.
        /// This occurs automatically when the router is configured as the client's default router at the time the
        /// session proxy is created in the client application; otherwise, the client must configure the session proxy
        /// explicitly.
        /// @param userId The user ID.
        /// @param password The password.
        /// @return A proxy for the newly created session, or null if no {@link SessionManager} is configured.
        /// @throws PermissionDeniedException Thrown when an authentication or authorization failure occurs.
        /// @throws CannotCreateSessionException Thrown when the session cannot be created.
        /// @see Session
        /// @see SessionManager
        /// @see PermissionsVerifier
        ["amd"]
        Session* createSession(string userId, string password)
            throws PermissionDeniedException, CannotCreateSessionException;

        /// Creates a per-client session with the router. The user is authenticated through the SSL certificate(s)
        /// associated with the connection. If an {@link SSLSessionManager} is configured, a proxy to a {@link Session}
        /// object is returned to the client. Otherwise, null is returned and only an internal session (i.e., not
        /// visible to the client) is created.
        /// If a non-null session proxy is returned, it must be configured to route through the router that created it.
        /// This occurs automatically when the router is configured as the client's default router at the time the
        /// session proxy is created in the client application; otherwise, the client must configure the session proxy
        /// explicitly.
        /// @see Session
        /// @see SessionManager
        /// @see PermissionsVerifier
        /// @return A proxy for the newly created session, or null if no {@link SSLSessionManager} is configured.
        /// @throws PermissionDeniedException Thrown when an authentication or authorization failure occurs.
        /// @throws CannotCreateSessionException Thrown when the session cannot be created.
        ["amd"]
        Session* createSessionFromSecureConnection()
            throws PermissionDeniedException, CannotCreateSessionException;

        /// Keeps the session with this router alive.
        /// @throws SessionNotExistException Thrown when no session exists for the caller (client).
        ["deprecated:As of Ice 3.8, this operation does nothing."]
        void refreshSession()
            throws SessionNotExistException;

        /// Destroys the session of the caller with this router.
        /// @throws SessionNotExistException Thrown when no session exists for the caller (client).
        void destroySession()
            throws SessionNotExistException;

        /// Gets the idle timeout used by the server-side of the connection.
        /// @return The idle timeout (in seconds).
        ["cpp:const"]
        idempotent long getSessionTimeout();

        /// Gets the idle timeout used by the server-side of the connection.
        /// @return The idle timeout (in seconds).
        ["cpp:const"]
        idempotent int getACMTimeout();
    }
}
