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
 * This exception is raised if a server with the same name already
 * exists.
 *
 **/
exception ApplicationExistsException
{
    string name;
};

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
 * This exception is raised if a server with the same name already
 * exists.
 *
 **/
exception ServerExistsException
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
    string id;
};

/**
 *
 * This exception is raised if an adapter does not exist.
 *
 **/
exception AdapterNotExistException
{
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
 * This exception is raised if an object does not exist.
 *
 **/
exception ObjectNotExistException
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
    string name;
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

};

#endif
