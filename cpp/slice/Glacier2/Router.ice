// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef GLACIER2_ROUTER_ICE
#define GLACIER2_ROUTER_ICE

#include <Ice/Router.ice>
#include <Glacier2/SessionF.ice>

/**
 *
 * &Glacier2; is a firewall solution for Ice. &Glacier2; authenticates
 * and filters client requests and allows callbacks to the client in a
 * secure fashion. In combination with &IceSSL;, &Glacier2; provides a
 * security solution that is both non-intrusive and easy to configure.
 *
 **/
module Glacier2
{

/**
 *
 * This exception is raised if router access is denied.
 *
 **/
exception PermissionDeniedException
{
    /**
     *
     * Details as to why access was denied.
     *
     **/
    string reason;
};

/**
 *
 * The &Glacier2; specialization of the standard &Ice; router
 * interface.
 *
 **/
interface Router extends Ice::Router
{
    /**
     *
     * Create a session with the router.
     *
     * @return A proxy for the newly created session.
     *
     * @param userId The user id for which to check the password.
     *
     * @param password The password for the given user id.
     *
     * @throws PermissionDeniedException Raised if the password for
     * the given user id is not correct, or if the user isn't allowed
     * access.
     *
     **/
    Session* createSession(string userId, string password)
	throws PermissionDeniedException;
};

};

#endif
