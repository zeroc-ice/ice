// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef GLACIER2_PERMISSIONS_VERIFIER_ICE
#define GLACIER2_PERMISSIONS_VERIFIER_ICE

module Glacier2
{

/**
 *
 * The &Glacier2; permissions verifier.
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
    nonmutating bool checkPermissions(string userId, string password, out string reason);
};

};

#endif
