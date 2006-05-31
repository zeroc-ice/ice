// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_REGISTRY_ICE
#define ICE_GRID_REGISTRY_ICE

#include <IceGrid/Exception.ice>
#include <IceGrid/Session.ice>
#include <IceGrid/Admin.ice>

module IceGrid
{

interface Registry
{
    /**
     *
     * Create a client session.
     *
     * @see Session
     * @see Glacier2::PermissionsVerifier
     *
     * @return A proxy for the newly created session.
     *
     * @param userId The user id for which to check the password.
     *
     * @param password The password for the given user id.
     *
     * @throws PermissionDeniedException Raised if the password for
     * the given user id is not correct, or if the user is not allowed
     * access.
     *
     **/
    Session* createSession(string userId, string password)
	throws PermissionDeniedException;

    /**
     *
     * Create an administrative session.
     *
     * @see Session
     * @see Glacier2::PermissionsVerifier
     *
     * @return A proxy for the newly created session.
     *
     * @param userId The user id for which to check the password.
     *
     * @param password The password for the given user id.
     *
     * @throws PermissionDeniedException Raised if the password for
     * the given user id is not correct, or if the user is not allowed
     * access.
     *
     **/
    AdminSession* createAdminSession(string userId, string password)
	throws PermissionDeniedException;

    /**
     *
     * Create a client session.
     *
     * @see Session
     * @see Glacier2::PermissionsVerifier
     *
     * @return A proxy for the newly created session.
     *
     * @param userId The user id for which to check the password.
     *
     * @param password The password for the given user id.
     *
     * @throws PermissionDeniedException Raised if the password for
     * the given user id is not correct, or if the user is not allowed
     * access.
     *
     **/
    Session* createSessionFromSecureConnection()
	throws PermissionDeniedException;

    /**
     *
     * Create an administrative session.
     *
     * @see Session
     * @see Glacier2::PermissionsVerifier
     *
     * @return A proxy for the newly created session.
     *
     * @throws PermissionDeniedException Raised if the password for
     * the given user id is not correct, or if the user is not allowed
     * access.
     *
     **/
    AdminSession* createAdminSessionFromSecureConnection()
	throws PermissionDeniedException;
};

};

#endif
