// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef GLACIER2_SESSION_ICE
#define GLACIER2_SESSION_ICE

module Glacier2
{

/**
 *
 * A session object, which is tied to the lifecycle of a [Router].
 *
 * @see Router
 * @see SessionManager
 *
 **/
interface Session
{
    /**
     *
     * Destroy the session. This is called automatically when the
     * [Router] is destroyed.
     *
     **/
    void destroy();
};

/**
 *
 * The session manager, which is responsible for managing [Session]
 * objects. New session objects are created by the [Router] object.
 *
 * @see Router
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
