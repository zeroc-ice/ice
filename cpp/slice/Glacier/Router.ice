// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
 * The &Glacier; router interface.
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
     * Create a new session. The session is automatically shutdown
     * when the Router terminates.
     *
     * @return A proxy to the new session.
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
