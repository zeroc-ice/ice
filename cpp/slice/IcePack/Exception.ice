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

#ifndef ICE_PACK_EXCEPTION_ICE
#define ICE_PACK_EXCEPTION_ICE

module IcePack
{

/**
 *
 * This exception is raised if an adapter doesn't exist.
 *
 **/
exception AdapterNotExistException
{
};

/**
 *
 * This exception is raised if a server doesn't exist.
 *
 **/
exception ServerNotExistException
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
 * This exception is raised if an object doesn't exist.
 *
 **/
exception ObjectNotExistException
{
};

/**
 *
 * This exception is raised if a node doesn't exist.
 *
 **/
exception NodeNotExistException
{
};

/**
 *
 * A generic exception base for all kind of deployment error
 * exception.
 *
 **/
exception DeploymentException
{
    /**
     *
     * The path of the component which cause the deployment to
     * fail. The path is a dot separated list of component names. It
     * always starts with the node name is followed by the server name
     * and eventually the service name.
     *
     **/
    string component;
    
    /**
     *
     * The reason for the failure.
     *
     **/
    string reason;
};

/**
 *
 * This exception is raised when an error occured while parsing the
 * XML descriptor of a component.
 *
 **/
exception ParserDeploymentException extends DeploymentException
{
};

/**
 *
 * This exception is raised when an error occured during the adapter
 * registration.
 *
 **/
exception AdapterDeploymentException extends DeploymentException
{
    /**
     *
     * The id of the adapter which couldn't be registered.
     *
     **/
    string id;
};

/**
 *
 * This exception is raised when an error occured during the object
 * registration.
 *
 **/
exception ObjectDeploymentException extends DeploymentException
{
    /**
     *
     * The object which couldn't be registered with the registry.
     *
     **/
    Object* proxy;
};

/**
 * 
 * This exception is raised if an error occured when deploying a
 * server.
 *
 **/
exception ServerDeploymentException extends DeploymentException
{
    /**
     *
     * The name of the server which couldn't be deployed.
     *
     **/
    string server;
};

/**
 *
 * This exception is raised if a node couldn't be reach.
 *
 **/
exception NodeUnreachableException
{
};

};

#endif
