// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PACK_EXCEPTION_ICE
#define ICE_PACK_EXCEPTION_ICE

module IcePack
{

/**
 *
 * This exception is raised if an adapter does not exist.
 *
 **/
exception AdapterNotExistException
{
};

/**
 *
 * This exception is raised if a server does not exist.
 *
 **/
exception ServerNotExistException
{
};

/**
 *
 * This exception is raised if an application does not exist.
 *
 **/
exception ApplicationNotExistException
{
};

/**
 *
 * This exception is raised if an object already exists.
 *
 **/
exception ObjectExistsException
{
};

/**
 *
 * This exception is raised if an object does not exist.
 *
 **/
exception ObjectNotExistException
{
};

/**
 *
 * This exception is raised if a node does not exist.
 *
 **/
exception NodeNotExistException
{
};

/**
 *
 * A generic exception base for all kinds of deployment error
 * exception.
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
