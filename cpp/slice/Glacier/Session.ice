// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
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
 * A Session object which is tied to the lifecycle of an [Router].
 *
 * @see Router
 * @see SessionManager
 *
 **/
interface Session
{
    /**
     *
     * Stop the session. This is called automatically when the
     * [Router] terminates.
     *
     **/
    void stop();
};

};

#endif
