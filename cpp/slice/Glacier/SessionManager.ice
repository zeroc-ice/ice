// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


#ifndef GLACIER_SESSION_MANAGER_ICE
#define GLACIER_SESSION_MANAGER_ICE

#include <Glacier/SessionF.ice>

module Glacier
{

/**
 *
 * The session manager, which is responsible for managing [Session]
 * objects. New session objects are created by the [Router] object.
 *
 * @see Session
 *
 **/
interface SessionManager
{
    /**
     *
     * Create a new session object.
     *
     * @param userId The user id for the session.
     *
     * @return A proxy to the newly created session.
     *
     **/
    Session* create(string userId);
};

};

#endif
