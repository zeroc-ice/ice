// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:GLACIER2_API"]]
[["cpp:doxygen:include:Glacier2/Glacier2.h"]]
[["cpp:header-ext:h"]]

[["cpp:include:Glacier2/Config.h"]]

[["js:module:@zeroc/ice"]]

#include "Ice/BuiltinSequences.ice"
#include "Ice/Identity.ice"
#include "SSLInfo.ice"

["java:identifier:com.zeroc.Glacier2"]
module Glacier2
{
    interface Router; // So that doc-comments can link to `Glacier2::Router`.

    /// The exception that is thrown when an attempt to create a new session fails.
    exception CannotCreateSessionException
    {
        /// The reason why the session creation failed.
        string reason;
    }

    /// Represents a session between a client application and the Glacier2 router. With Glacier2, the lifetime of a
    /// session is tied to the Ice connection between the client and the router: the session is destroyed when the
    /// connection is closed.
    /// @see Router
    /// @see SessionManager
    interface Session
    {
        /// Destroys this session.
        void destroy();
    }

    /// Manages a set of constraints on a {@link Session}.
    /// @see SessionControl
    interface StringSet
    {
        /// Adds a sequence of strings to this set of constraints. Order is not preserved and duplicates are implicitly
        /// removed.
        /// @param additions The sequence of strings to add.
        idempotent void add(Ice::StringSeq additions);

        /// Removes a sequence of strings from this set of constraints. No errors are returned if an entry is not found.
        /// @param deletions The sequence of strings to remove.
        idempotent void remove(Ice::StringSeq deletions);

        /// Gets a sequence of strings describing the constraints in this set.
        /// @return The sequence of strings for this set.
        ["swift:identifier:`get`"]
        idempotent Ice::StringSeq get();
    }

    /// Manages a set of object identity constraints on a {@link Session}.
    /// @see SessionControl
    interface IdentitySet
    {
        /// Adds a sequence of Ice identities to this set of constraints. Order is not preserved and duplicates are
        /// implicitly removed.
        /// @param additions The sequence of Ice identities to add.
        idempotent void add(Ice::IdentitySeq additions);

        /// Removes a sequence of identities from this set of constraints. No errors are returned if an entry is not
        /// found.
        /// @param deletions The sequence of Ice identities to remove.
        idempotent void remove(Ice::IdentitySeq deletions);

        /// Gets a sequence of identities describing the constraints in this set.
        /// @return The sequence of Ice identities for this set.
        ["swift:identifier:`get`"]
        idempotent Ice::IdentitySeq get();
    }

    /// Represents a router-provided object that allows an application-provided session manager to configure the
    /// routing constraints for a session.
    /// @see SessionManager
    interface SessionControl
    {
        /// Gets a proxy to the object that manages the allowable categories for object identities for this session.
        /// @return A proxy to a {@link StringSet} object. This proxy is never null.
        StringSet* categories();

        /// Gets a proxy to the object that manages the allowable adapter identities for objects for this session.
        /// @return A proxy to a {@link StringSet} object. This proxy is never null.
        StringSet* adapterIds();

        /// Gets a proxy to the object that manages the allowable object identities for this session.
        /// @return A proxy to an {@link IdentitySet} object. This proxy is never null.
        IdentitySet* identities();

        /// Gets the session timeout.
        /// @return The timeout.
        idempotent int getSessionTimeout();

        /// Destroys the associated session.
        void destroy();
    }

    /// Represents an application-provided factory for session objects. You can configure a Glacier2 router with your
    /// own SessionManager implementation; this router will then return the sessions created by this session manager to
    /// its clients.
    interface SessionManager
    {
        /// Creates a new session object.
        /// @param userId The user ID for the session.
        /// @param control A proxy to the session control object. This proxy is null when `Glacier2.Server.Endpoints`
        /// is not configured.
        /// @return A proxy to the newly created session. This proxy is never null.
        /// @throws CannotCreateSessionException Thrown when the session cannot be created.
        Session* create(string userId, SessionControl* control)
            throws CannotCreateSessionException;
    }

    /// Represents an application-provided factory for session objects. You can configure a Glacier2 router with your
    /// own SSLSessionManager implementation; this router will then return the sessions created by this session manager
    /// to its clients.
    interface SSLSessionManager
    {
        /// Creates a new session object.
        /// @param info The SSL info.
        /// @param control A proxy to the session control object. This proxy is null when `Glacier2.Server.Endpoints`
        /// is not configured.
        /// @return A proxy to the newly created session. This proxy is never null.
        /// @throws CannotCreateSessionException Thrown when the session cannot be created.
        Session* create(SSLInfo info, SessionControl* control)
            throws CannotCreateSessionException;
    }
}
