// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h"]]

#include <IceGrid/Exception.ice>
#include <IceGrid/Session.ice>
#include <IceGrid/Admin.ice>

module IceGrid
{

/**
 *
 * The IceGrid registry allows clients create sessions
 * directly with the registry.
 *
 * @see Session
 * @see AdminSession
 * 
 **/
interface Registry
{
    /**
     *
     * Create a client session.
     *
     * @see Session
     * @see Glacier2.PermissionsVerifier
     *
     * @return A proxy for the newly created session.
     *
     * @param userId The user id.
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
     * @see Glacier2.PermissionsVerifier
     *
     * @return A proxy for the newly created session.
     *
     * @param userId The user id.
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
     * Create a client session from a secure connection.
     *
     * @see Session
     * @see Glacier2.SSLPermissionsVerifier
     *
     * @return A proxy for the newly created session.
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
     * Create an administrative session from a secure connection.
     *
     * @see Session
     * @see Glacier2.SSLPermissionsVerifier
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

    /**
     *
     * Get the session timeout. If a client or administrative client
     * doesn't call the session keepAlive method in the time interval
     * defined by this timeout, IceGrid might reap the session.
     *
     * @see Session#keepAlive
     * @see AdminSession#keepAlive
     *
     * @return The timeout (in seconds).
     *
     **/
    ["nonmutating", "cpp:const"] idempotent int getSessionTimeout();
};

};

