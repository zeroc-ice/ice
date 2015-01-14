// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <Glacier2/Router.ice>

module Demo
{

/**
 *
 * The Demo router interface. This router is used to route requests directly
 * to an Ice server.
 *
 **/
interface Router extends Ice::Router
{
    /**
     *
     * Create a new session. The returned category must be used in the
     * identities of all of the client's callback objects. This is necessary
     * in order for the router to forward callback requests to the intended
     * client.
     *
     * @return A category.
     *
     */
    string createSession();

    /**
     *
     * Create a new glacier2 session.
     *
     * @param router A proxy to the Glacier2 router.
     *
     * @param username The user id for which to check the password.
     *
     * @param password The password for the given user id.
     *
     * @param category The returned category that must be used in the
     * identities of all of the client's callback objects.
     *
     * @param sessionTimeout The session timeout.
     *
     * @param sess The A proxy for the newly created session, or null
     * if no [Glacier2::SessionManager] was defined.
     *
     * @throws Glacier2::PermissionDeniedException Raised if the password for
     * the given user id is not correct, or if the user is not allowed
     * access.
     *
     * @throws Glacier2::CannotCreateSessionException Raised if the session
     * cannot be created.
     *
     */
    
    void createGlacier2Session(Ice::Router* router, string userId, string password,
                               out string category, out int sessionTimeout,
                               out Glacier2::Session* sess)
        throws Glacier2::PermissionDeniedException, Glacier2::CannotCreateSessionException;
};

};
