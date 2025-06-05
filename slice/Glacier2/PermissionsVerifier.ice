// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:GLACIER2_API"]]
[["cpp:doxygen:include:Glacier2/Glacier2.h"]]
[["cpp:header-ext:h"]]

[["cpp:include:Glacier2/Config.h"]]

[["js:module:@zeroc/ice"]]
[["python:pkgdir:Glacier2"]]

#include "SSLInfo.ice"

["java:identifier:com.zeroc.Glacier2"]
module Glacier2
{
    /// The exception that is thrown when a client is not allowed to create a session.
    exception PermissionDeniedException
    {
        /// The reason why permission was denied.
        string reason;
    }

    /// Represents an object that checks user permissions. The Glacier2 router and other services use a
    /// {@link PermissionsVerifier} proxy when the user is authenticated using a user ID and password.
    interface PermissionsVerifier
    {
        /// Checks if a user is authorized to establish a session.
        /// @param userId The user ID.
        /// @param password The user's password.
        /// @param reason The reason why access was denied.
        /// @return `true` if access is granted, `false` otherwise.
        /// @throws PermissionDeniedException Thrown when the user access is denied. This exception can be thrown
        /// instead of returning `false` with a reason set in the reason out parameter.
        ["cpp:const"]
        idempotent bool checkPermissions(string userId, string password, out string reason)
            throws PermissionDeniedException;
    }

    /// Represents an object that checks user permissions. The Glacier2 router and other services use an
    /// {@link SSLPermissionsVerifier} proxy when the user is authenticated through an SSL certificate.
    interface SSLPermissionsVerifier
    {
        /// Checks if a user is authorized to establish a session.
        /// @param info The SSL information.
        /// @param reason The reason why access was denied.
        /// @return `true` if access is granted, `false` otherwise.
        /// @throws PermissionDeniedException Thrown when the user access is denied. This exception can be thrown
        /// instead of returning `false` with a reason set in the reason out parameter.
        ["cpp:const"]
        idempotent bool authorize(SSLInfo info, out string reason)
            throws PermissionDeniedException;
    }
}
