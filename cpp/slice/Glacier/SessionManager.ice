// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef GLACIER_SESSION_MANAGER_ICE
#define GLACIER_SESSION_MANAGER_ICE

#include <Glacier/SessionF.ice>

module Glacier
{

/**
 *
 * The Session Manager, which is responsible for managing [Session]
 * objects. New session objects are created by the [Router] object.
 *
 * @see Session
 *
 **/
interface SessionManager
{
    /**
     *
     * Create a new Session object.
     *
     * @param userId The user id for the session.
     *
     * @return A proxy to the newly created Session.
     *
     **/
    Session* create(string userId);
};

};

#endif
