// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef GLACIER2_PERMISSIONS_VERIFIER_ICE
#define GLACIER2_PERMISSIONS_VERIFIER_ICE

#include <Glacier2/SSLInfo.ice>

module Glacier2
{

/**
 *
 * The Glacier2 permissions verifier. This is called through the
 * process of establishing a session.
 *
 * @see Router
 *
 **/
interface PermissionsVerifier
{
    /**
     *
     * Check whether a user has permission to access the router.
     *
     * @param userId The user id for which to check permission.
     *
     * @param password The user's password.
     *
     * @param reason The reason why access was denied.
     *
     * @return True if access is granted, or false otherwise.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent bool checkPermissions(string userId, string password, out string reason);
};

/**
 *
 * The SSL Glacier2 permissions verifier. This is called through the
 * process of establishing a session.
 *
 * @see Router
 *
 **/
interface SSLPermissionsVerifier
{
    /**
     *
     * Check whether a user has permission to access the router.
     *
     * @param info The SSL information.
     *
     * @param reason The reason why access was denied.
     *
     * @return True if access is granted, or false otherwise.
     *
     * @see SSLInfo
     *
     **/
    ["nonmutating", "cpp:const"] idempotent bool authorize(SSLInfo info, out string reason);
};

};

#endif
