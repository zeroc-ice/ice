// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef GLACIER_SESSION_ICE
#define GLACIER_SESSION_ICE

module Glacier
{

/**
 *
 * A session object, which is tied to the lifecycle of an [Router].
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

};

#endif
