// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:GLACIER2_API"]]
[["cpp:doxygen:include:Glacier2/Glacier2.h"]]
[["cpp:header-ext:h"]]
[["cpp:include:Glacier2/Config.h"]]

[["js:module:@zeroc/ice"]]

[["python:pkgdir:Glacier2"]]

#include "Ice/BuiltinSequences.ice"
#include "Ice/Identity.ice"
#include "SSLInfo.ice"

[["java:package:com.zeroc"]]

module Glacier2
{
    interface Router; // So that doc-comments can link to `Glacier2::Router`.

    /// This exception is raised if an attempt to create a new session failed.
    exception CannotCreateSessionException
    {
        /// The reason why session creation has failed.
        string reason;
    }

    /// A client-visible session object, which is tied to the lifecycle of a {@link Router}.
    /// @see Router
    /// @see SessionManager
    interface Session
    {
        /// Destroy the session. This is called automatically when the router is destroyed.
        void destroy();
    }

    /// An object for managing the set of identity constraints for specific parts of object identity on a
    /// {@link Session}.
    /// @see Session
    /// @see SessionControl
    interface StringSet
    {
        /// Add a sequence of strings to this set of constraints. Order is not preserved and duplicates are implicitly
        /// removed.
        /// @param additions The sequence of strings to be added.
        idempotent void add(Ice::StringSeq additions);

        /// Remove a sequence of strings from this set of constraints. No errors are returned if an entry is not found.
        /// @param deletions The sequence of strings to be removed.
        idempotent void remove(Ice::StringSeq deletions);

        /// Returns a sequence of strings describing the constraints in this set.
        /// @return The sequence of strings for this set.
        idempotent Ice::StringSeq get();
    }

    /// An object for managing the set of object identity constraints on a {@link Session}.
    /// @see Session
    /// @see SessionControl
    interface IdentitySet
    {
        /// Add a sequence of Ice identities to this set of constraints. Order is not preserved and duplicates are
        /// implicitly removed.
        /// @param additions The sequence of Ice identities to be added.
        idempotent void add(Ice::IdentitySeq additions);

        /// Remove a sequence of identities from this set of constraints. No errors are returned if an entry is not
        /// found.
        /// @param deletions The sequence of Ice identities to be removed.
        idempotent void remove(Ice::IdentitySeq deletions);

        /// Returns a sequence of identities describing the constraints in this set.
        /// @return The sequence of Ice identities for this set.
        idempotent Ice::IdentitySeq get();
    }

    /// An administrative session control object, which is tied to the lifecycle of a {@link Session}.
    /// @see Session
    interface SessionControl
    {
        /// Access the object that manages the allowable categories for object identities for this session.
        /// @return A StringSet object. The returned proxy is never null.
        StringSet* categories();

        /// Access the object that manages the allowable adapter identities for objects for this session.
        /// @return A StringSet object. The returned proxy is never null.
        StringSet* adapterIds();

        /// Access the object that manages the allowable object identities for this session.
        /// @return An IdentitySet object. The returned proxy is never null.
        IdentitySet* identities();

        /// Get the session timeout.
        /// @return The timeout.
        idempotent int getSessionTimeout();

        /// Destroy the associated session.
        void destroy();
    }

    /// The session manager for username/password authenticated users that is responsible for managing {@link Session}
    /// objects. New session objects are created by the {@link Router} object calling on an application-provided
    /// session manager. If no session manager is provided by the application, no client-visible sessions are passed to
    /// the client.
    /// @see Router
    /// @see Session
    interface SessionManager
    {
        /// Create a new session. The implementation must return a non-null proxy or raise
        /// {@link CannotCreateSessionException} if the session cannot be created.
        /// @param userId The user id for the session.
        /// @param control A proxy to the session control object. The control proxy is null if Glacier2.Server.Endpoints
        /// are not configured.
        /// @return A proxy to the newly created session.
        /// @throws CannotCreateSessionException Raised if the session cannot be created.
        Session* create(string userId, SessionControl* control)
            throws CannotCreateSessionException;
    }

    /// The session manager for SSL authenticated users that is responsible for managing {@link Session} objects. New
    /// session objects are created by the {@link Router} object calling on an application-provided session manager. If
    /// no session manager is provided by the application, no client-visible sessions are passed to the client.
    /// @see Router
    /// @see Session
    interface SSLSessionManager
    {
        /// Create a new session.
        /// @param info The SSL info.
        /// @param control A proxy to the session control object.
        /// @return A proxy to the newly created session.
        /// @throws CannotCreateSessionException Raised if the session cannot be created.
        Session* create(SSLInfo info, SessionControl* control)
            throws CannotCreateSessionException;
    }
}
