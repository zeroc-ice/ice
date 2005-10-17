// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_EXCEPTION_ICE
#define ICE_GRID_EXCEPTION_ICE

#include <Ice/Identity.ice>

module IceGrid
{

/**
 *
 * This exception is raised if an application does not exist.
 *
 **/
exception ApplicationNotExistException
{
    string name;
};

/**
 *
 * This exception is raised if a server does not exist.
 *
 **/
exception ServerNotExistException
{
    /** The identifier of the server. */
    string id;
};

/**
 *
 * This exception is raised if a server failed to start.
 *
 **/
exception ServerStartException
{
    /** The identifier of the server. */
    string id;
    
    /** The reason of the failure. */
    string reason;
};

/**
 *
 * This exception is raised if an adapter does not exist.
 *
 **/
exception AdapterNotExistException
{
    /** The id of the object adapter. */
    string id;
};

/**
 *
 * This exception is raised if an object already exists.
 *
 **/
exception ObjectExistsException
{
    Ice::Identity id;
};

/**
 *
 * This exception is raised if an object is not registered.
 *
 **/
exception ObjectNotRegisteredException
{
    Ice::Identity id;
};

/**
 *
 * This exception is raised if a node does not exist.
 *
 **/
exception NodeNotExistException
{
    string name;
};

/**
 *
 * An exception for deployment failure errors.
 *
 **/
exception DeploymentException
{
    /**
     *
     * The reason for the failure.
     *
     **/
    string reason;
};

/**
 *
 * This exception is raised if a node could not be reached.
 *
 **/
exception NodeUnreachableException
{
    /** The name of the node which is not reachable. */
    string name;

    /** The reason why the node couldn't be reached. */
    string reason;
};

/**
 *
 * This exception is raised if an unknown signal was sent to
 * to a server.
 *
 **/
exception BadSignalException
{
};

/**
 *
 * This exception is raised if the patching failed.
 *
 **/
exception PatchException
{
    string reason;
};

/**
 *
 * This exception is raised if an operation can't be performed because
 * the regitry lock wasn't acquired or is already acquired by a session.
 *
 **/
exception AccessDeniedException
{
    string lockUserId;
};

};

#endif
