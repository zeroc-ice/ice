//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[cpp:dll-export(GLACIER2_API)]]
[[cpp:doxygen:include(Glacier2/Glacier2.h)]]
[[cpp:header-ext(h)]]
[[cpp:include(Glacier2/Config.h)]]

[[js:module(ice)]]

[[python:pkgdir(Glacier2)]]

#include <Glacier2/SSLInfo.ice>

[[java:package(com.zeroc)]]
[cs:namespace(ZeroC)]
module Glacier2
{
    /// This exception is raised if a client is denied the ability to create
    /// a session with the router.
    [preserve-slice]
    exception PermissionDeniedException
    {
        /// The reason why permission was denied.
        string reason;
    }

    /// The Glacier2 permissions verifier. This is called through the
    /// process of establishing a session.
    ///
    /// @see Router
    interface PermissionsVerifier
    {
        /// Check whether a user has permission to access the router.
        ///
        /// @param userId The user id for which to check permission.
        ///
        /// @param password The user's password.
        ///
        /// @param reason The reason why access was denied.
        ///
        /// @return True if access is granted, or false otherwise.
        ///
        /// @throws PermissionDeniedException Raised if the user access is
        /// denied. This can be raised in place of returning false with a
        /// reason set in the reason out parameter.
        [nonmutating] [cpp:const] [format(sliced)]
        idempotent bool checkPermissions(string userId, string password, out string reason)
            throws PermissionDeniedException;
    }

    /// The SSL Glacier2 permissions verifier. This is called through the
    /// process of establishing a session.
    ///
    /// @see Router
    interface SSLPermissionsVerifier
    {
        /// Check whether a user has permission to access the router.
        ///
        /// @param info The SSL information.
        ///
        /// @param reason The reason why access was denied.
        ///
        /// @return True if access is granted, or false otherwise.
        ///
        /// @throws PermissionDeniedException Raised if the user access is
        /// denied. This can be raised in place of returning false with a
        /// reason set in the reason out parameter.
        ///
        /// @see SSLInfo
        [nonmutating] [cpp:const] [format(sliced)]
        idempotent bool authorize(SSLInfo info, out string reason)
            throws PermissionDeniedException;
    }
}
