// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
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
     * The path of the component that caused the deployment to
     * fail. The path is a dot-separated list of component names. It
     * always starts with the node name, followed by the server name,
     * and finally the service name.
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
 * This exception is raised if an error occurs while parsing the
 * XML descriptor of a component.
 *
 **/
exception ParserDeploymentException extends DeploymentException
{
};

/**
 *
 * This exception is raised if an error occurs during adapter
 * registration.
 *
 **/
exception AdapterDeploymentException extends DeploymentException
{
    /**
     *
     * The id of the adapter that could not be registered.
     *
     **/
    string id;
};

/**
 *
 * This exception is raised if an error occurs during object
 * registration.
 *
 **/
exception ObjectDeploymentException extends DeploymentException
{
    /**
     *
     * The object that could not be registered.
     *
     **/
    Object* proxy;
};

/**
 * 
 * This exception is raised if an error occurs while deploying a
 * server.
 *
 **/
exception ServerDeploymentException extends DeploymentException
{
    /**
     *
     * The name of the server that could not be deployed.
     *
     **/
    string server;
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
