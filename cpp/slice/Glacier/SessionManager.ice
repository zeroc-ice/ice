// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
 * objects. New session objects are created from the [Router] object.
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
     * @see Properties
     *
     **/
    Session* create(string userId);
};

};

#endif
