// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_USERACCOUNTMAPPER_ICE
#define ICE_GRID_USERACCOUNTMAPPER_ICE

module IceGrid
{

exception UserAccountNotFoundException
{
};

interface UserAccountMapper
{
    /**
     *
     * Get the name of the user account for the given user. This is
     * used by &IceGrid; nodes to figure out the user account to use
     * to run servers.
     *
     * @param user The value of the server descriptor user attribute
     * of the session identity if the user attribute is not set and
     * the server activation mode is "session".
     *
     * @return The user account name.
     *
     * @throws UserAccountNotFoundException Raised if no user account
     * is found for the given user.
     * 
     **/
    string getUserAccount(string user)
	throws UserAccountNotFoundException;
};

};

#endif
