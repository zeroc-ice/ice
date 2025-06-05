// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICEGRID_API"]]
[["cpp:doxygen:include:IceGrid/IceGrid.h"]]
[["cpp:header-ext:h"]]

[["cpp:include:IceGrid/Config.h"]]

[["js:module:@zeroc/ice"]]
[["python:pkgdir:IceGrid"]]

#include "Admin.ice"
#include "Exception.ice"
#include "Ice/Locator.ice"
#include "Session.ice"

/// Deploy and manage Ice servers.
["java:identifier:com.zeroc.IceGrid"]
module IceGrid
{
    /// Determines which load sampling interval to use.
    enum LoadSample
    {
        /// Sample every minute.
        ["swift:identifier:loadSample1"]
        LoadSample1,

        /// Sample every five minutes.
        ["swift:identifier:loadSample5"]
        LoadSample5,

        /// Sample every fifteen minutes.
        ["swift:identifier:loadSample15"]
        LoadSample15
    }

    /// Finds well-known Ice objects registered with the IceGrid registry.
    interface Query
    {
        /// Finds a well-known object by identity.
        /// @param id The identity.
        /// @return A proxy to the well-known object, or null if no such object is registered.
        ["cpp:const"]
        idempotent Object* findObjectById(Ice::Identity id);

        /// Finds a well-known object by type. If there are several objects registered for the given type, the object is
        /// randomly selected.
        /// @param type The object type.
        /// @return A proxy to a well-known object with the specified type, or null if no such object is registered.
        ["cpp:const"]
        idempotent Object* findObjectByType(string type);

        /// Finds a well-known object by type on the least-loaded node. If the registry does not know which node hosts
        /// the object (for example, because the object was registered with a direct proxy), the registry assumes the
        /// object is hosted on a node that has a load average of 1.0.
        /// @param type The object type.
        /// @param sample The sampling interval.
        /// @return A proxy to the well-known object, or null if no such object is registered.
        ["cpp:const"]
        idempotent Object* findObjectByTypeOnLeastLoadedNode(string type, LoadSample sample);

        /// Finds all the well-known objects with the given type.
        /// @param type The object type.
        /// @return A list of proxies to the well-known objects with the specified type. Can be empty.
        ["cpp:const"]
        idempotent Ice::ObjectProxySeq findAllObjectsByType(string type);

        /// Finds all the replicas of a well-known object.
        /// @param proxy A proxy that identifies the well-known object.
        /// @return A list of proxies  to the replicas of the well-known object specified by @p proxy. Can be empty.
        ["cpp:const"]
        idempotent Ice::ObjectProxySeq findAllReplicas(Object* proxy);
    }

    /// Represents the main entry point into the IceGrid registry service. It provides operations to create sessions
    /// with the registry.
    interface Registry
    {
        /// Creates a client session.
        /// @param userId The user ID.
        /// @param password The password for the given user.
        /// @return A proxy to the newly created session. This proxy is never null.
        /// @throws PermissionDeniedException Thrown when authentication or authorization fails.
        Session* createSession(string userId, string password)
            throws PermissionDeniedException;

        /// Creates an administrative session.
        /// @param userId The user ID.
        /// @param password The password for the given user.
        /// @return A proxy for the newly created session. This proxy is never null.
        /// @throws PermissionDeniedException Thrown when authentication or authorization fails.
        AdminSession* createAdminSession(string userId, string password)
            throws PermissionDeniedException;

        /// Creates a client session from a secure connection.
        /// @return A proxy for the newly created session. This proxy is never null.
        /// @throws PermissionDeniedException Thrown when authentication or authorization fails.
        Session* createSessionFromSecureConnection()
            throws PermissionDeniedException;

        /// Creates an administrative session from a secure connection.
        /// @return A proxy for the newly created session. This proxy is never null.
        /// @throws PermissionDeniedException Thrown when authentication or authorization fails.
        AdminSession* createAdminSessionFromSecureConnection()
            throws PermissionDeniedException;

        /// Gets the session timeout. An Ice 3.7 or earlier client can use this value to determine how often it needs to
        /// send heartbeats (using ACM) or call {@link Session#keepAlive} (resp. {@link AdminSession#keepAlive}) to keep
        /// a session alive in the IceGrid registry.
        /// @return The session timeout (in seconds).
        ["cpp:const"]
        ["deprecated"]
        idempotent int getSessionTimeout();
    }

    /// Provides access to the {@link Query} and {@link Registry} objects implemented by the IceGrid registry.
    interface Locator extends Ice::Locator
    {
        /// Gets a proxy to the registry object hosted by this IceGrid registry.
        /// @return A proxy to the registry object. This proxy is never null.
        ["cpp:const"]
        idempotent Registry* getLocalRegistry();

        /// Gets a proxy to the query object hosted by this IceGrid registry.
        /// @return A proxy to the query object. This proxy is never null.
        ["cpp:const"]
        idempotent Query* getLocalQuery();
    }
}
