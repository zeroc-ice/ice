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

#ifndef GLACIER_ROUTER_ICE
#define GLACIER_ROUTER_ICE

#include <Ice/Router.ice>
#include <Glacier/SessionF.ice>

module Glacier
{

/**
 *
 * This exception is raised if no [SessionManager] object has been
 * configured.
 *
 **/
exception NoSessionManagerException
{
};

/**
 *
 * The Glacier router interface.
 *
 **/
interface Router extends Ice::Router
{
    /**
     *
     * Shutdown the router.
     *
     **/
    void shutdown();

    /**
     *
     * Create a new session. The Session is automatically shutdown
     * when the Router terminates.
     *
     * @return A proxy to the new Session.
     *
     * @throws NoSessionManagerException if there is no
     * configured [SessionManager].
     *
     **/
    Session* createSession()
	throws NoSessionManagerException;
};

};

#endif
