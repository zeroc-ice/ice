// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef GLACIER_ICE
#define GLACIER_ICE

#include <Ice/Router.ice>

/**
 *
 * &Glacier; is the Ice firewall and router.
 *
 **/
module Glacier
{

/**
 *
 * This exception is raised if an incorrect password was given.
 *
 **/
exception InvalidPasswordException
{
};

/**
 *
 * This exception is raised if the router cannot be started.
 *
 **/
exception CannotStartRouterException
{
    string reason;
};

/**
 *
 * The &Glacier; router starter.
 *
 **/
interface Starter
{
    /**
     *
     * Start a new &Glacier; router. If the password for the given
     * user id is incorrect, an [InvalidPasswordException] is
     * raised. Otherwise a new router is started, and a proxy to that
     * router is returned to the caller.
     *
     * @param userId The user id for which to check the password.
     * @param password The password for the given user id.
     *
     * @return A proxy to the router that has been started.
     *
     * @throws InvalidPasswordException Raised if the password for the
     * given user id is not correct.
     *
     **/
    Ice::Router* startRouter(string userId, string password)
	throws InvalidPasswordException, CannotStartRouterException;
};

};

#endif
