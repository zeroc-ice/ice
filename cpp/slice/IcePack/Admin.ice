// **********************************************************************
//
// Copyright (c) 2002
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

#ifndef ICE_PACK_ADMIN_ICE
#define ICE_PACK_ADMIN_ICE

#include <Ice/Identity.ice>
#include <Ice/BuiltinSequences.ice>
#include <IceBox/IceBox.ice>
#include <IcePack/Exception.ice>

module IcePack
{

/**
 *
 * A vector of strings representing command line arguments.
 *
 **/
sequence<string> ServerArgs;

/**
 *
 * A vector of strings representing deployment targets.
 *
 **/
sequence<string> ServerTargets;

/**
 *
 * An enumeration representing the state of the server.
 *
 **/
enum ServerState
{
    /**
     *
     * The server is not running.
     *
     **/
    Inactive,

    /**
     *
     * The server is being activated and will change to the Active
     * state if the server fork succeed or to the Inactive state if it
     * failed.
     *
     **/
    Activating,

    /**
     *
     * The server is running.
     *
     **/
    Active,
    
    /**
     *
     * The server is being deactivated.
     *
     **/
    Deactivating,

    /**
     *
     * The server is being destroyed.
     *
     **/
    Destroying,

    /**
     *
     * The server is destroyed.
     *
     **/
    Destroyed
};

/**
 *
 * The server activation mode.
 *
 **/
enum ServerActivation
{
    /**
     *
     * The server is activated on demand when a client requests one of
     * the adapter endpoints and if the server isn't already running.
     *
     **/
    OnDemand,

    /**
     *
     * The server is activated manually through the administrative
     * interface.
     *
     **/
    Manual
};

/**
 *
 * The server description.
 *
 **/
struct ServerDescription
{
    /**
     *
     * Server name.
     *
     **/
    string name;

    /**
     *
     * The name of the node where the server is deployed.
     *
     **/
    string node;

    /**
     *
     * The path of the deployment descriptor used to deploy the
     * server.
     *
     **/
    string descriptor;

    /**
     *
     * Targets used to deploy the server.
     *
     **/
    ServerTargets targets;

    /**
     *
     * The server path.
     *
     **/
    string path;

    /**
     *
     * The server working directory.
     *
     **/
    string pwd;

    /**
     *
     * The server arguments.
     *
     **/
    ServerArgs args;
    
    /**
     *
     * The &IceBox; service manager proxy if the server is an
     * &IceBox; or a null proxy otherwise.
     *
     **/
    IceBox::ServiceManager* serviceManager;
};

/**
 *
 * The &IcePack; administrative interface. <warning><para>Allowing
 * access to this interface is a security risk! Please see the &IcePack;
 * documentation for further information.</para></warning>
 *
 **/
interface Admin
{
    /**
     *
     * Add an application to &IcePack;. An application is a set of
     * servers.
     *
     * @param descriptor The application descriptor.
     * 
     * @param targets The optional targets to deploy. A target is a list
     * of components separated by dots and a target name. For example,
     * the "debug" target of the "service1" in the "server1" will be
     * deployed if the target "server1.service1.debug" is specified.
     *
     * @throws DeploymentException Raised if the deployment of the
     * application failed.
     *
     * @see removeApplication
     *
     **/
    void addApplication(string descriptor, ServerTargets targets)
	throws DeploymentException;    

    /**
     *
     * Remove an application from &IcePack;.
     *
     * @param descriptor The application descriptor.
     *
     * @see addApplication
     *
     **/
    void removeApplication(string descriptor)
	throws DeploymentException;

    /**
     *
     * Add a server to an &IcePack; node.
     *
     * @param node The name of the node where the server will be
     * deployed.
     *
     * @param name The server name.
     *
     * @param path The server path. For C++ servers, this is the path
     * of the executable. For C++ icebox, this is the path of the C++
     * icebox executable or if empty IcePack will rely on the path to
     * find it. For Java server or Java icebox, this is the path of
     * the java command or if empty IcePack will rely on the path to
     * find it.
     *
     * @param librarypath Specify the LD_LIBRARY_PATH value for C++
     * servers or the CLASSPATH value for Java servers.
     *
     * @param desciptor The server deployment descriptor.
     *
     * @param targets The optional targets to deploy. A target is a list
     * of components separated by dots and a target name. For example,
     * the "debug" target of the "service1" in the "server1" will be
     * deployed if the target "server1.service1.debug" is specified.
     *
     * @throws DeploymentException Raised if the deployment of the
     * server failed.
     *
     * @throws NodeUnreachableException Raised if the node couldn't be
     * reached.
     *
     * @see removeServer
     *
     **/
    void addServer(string node, string name, string path, string libraryPath, string descriptor, ServerTargets targets)
	throws DeploymentException, NodeUnreachableException;

    /**
     *
     * Remove a server from an &IcePack; node.
     *
     * @param name Must match the name of [ServerDescription::name].
     *
     * @throws DeploymentException Raised if the server deployer
     * failed to remove the server.
     *
     * @throws ServerNotExistException Raised if the server is not
     * found.
     *
     * @throws NodeUnreachableException Raised if the node couldn't be
     * reached.
     *
     * @see addServer
     *
     **/
    void removeServer(string name)
	throws DeploymentException, ServerNotExistException, NodeUnreachableException;

    /**
     *
     * Get a server description.
     *
     * @param name Must match the name of [ServerDescription::name].
     *
     * @return The server description.
     *
     * @throws ServerNotExistException Raised if the server is not
     * found.
     *
     * @throws NodeUnreachableException Raised if the node couldn't be
     * reached.
     *
     * @see getServerState
     * @see getServerPid
     * @see getAllServerNames
     *
     **/
    nonmutating ServerDescription getServerDescription(string name)
	throws ServerNotExistException, NodeUnreachableException;
    
    /**
     *
     * Get a server state.
     *
     * @param name Must match the name of [ServerDescription::name].
     *
     * @return The server state.
     * 
     * @throws ServerNotExistException Raised if the server is not
     * found.
     *
     * @throws NodeUnreachableException Raised if the node couldn't be
     * reached.
     *
     * @see getServerDescription
     * @see getServerPid
     * @see getAllServerNames
     *
     **/
    nonmutating ServerState getServerState(string name)
	throws ServerNotExistException, NodeUnreachableException;
    
    /**
     *
     * Get a server system process id. The process id is system
     * dependent and might mean different things depending on the
     * operating system the server is running.
     *
     * @param name Must match the name of [ServerDescription::name].
     *
     * @return The server process id.
     * 
     * @throws ServerNotExistException Raised if the server is not
     * found.
     *
     * @throws NodeUnreachableException Raised if the node couldn't be
     * reached.
     *
     * @see getServerDescription
     * @see getServerState
     * @see getAllServerNames
     *
     **/
    nonmutating int getServerPid(string name)
	throws ServerNotExistException, NodeUnreachableException;

    /**
     *
     * Get the server activation mode.
     *
     * @param name Must match the name of [ServerDescription::name].
     *
     * @return The server activation mode.
     * 
     * @throws ServerNotExistException Raised if the server is not
     * found.
     *
     * @throws NodeUnreachableException Raised if the node couldn't be
     * reached.
     *
     * @see getServerDescription
     * @see getServerState
     * @see getAllServerNames
     *
     **/
    nonmutating ServerActivation getServerActivation(string name)
	throws ServerNotExistException, NodeUnreachableException;

    /**
     *
     * Set the server activation mode.
     *
     * @param name Must match the name of [ServerDescription::name].
     *
     * @return The server activation mode.
     * 
     * @throws ServerNotExistException Raised if the server is not
     * found.
     *
     * @throws NodeUnreachableException Raised if the node couldn't be
     * reached.
     *
     * @see getServerDescription
     * @see getServerState
     * @see getAllServerNames
     *
     **/
    void setServerActivation(string name, ServerActivation mode)
	throws ServerNotExistException, NodeUnreachableException;

    /**
     *
     * Start a server.
     *
     * @param name Must match the name of [ServerDescription::name].
     *
     * @return True if the server was successfully started, false
     * otherwise.
     *
     * @throws ServerNotExistException Raised if the server is not
     * found.
     *
     * @throws NodeUnreachableException Raised if the node couldn't be
     * reached.
     *
     **/
    bool startServer(string name)
	throws ServerNotExistException, NodeUnreachableException;

    /**
     *
     * Stop a server.
     *
     * @param name Must match the name of [ServerDescription::name].
     *
     * @throws ServerNotExistException Raised if the server is not
     * found.
     *
     * @throws NodeUnreachableException Raised if the node couldn't be
     * reached.
     *
     **/
    void stopServer(string name)
	throws ServerNotExistException, NodeUnreachableException;

    /**
     *
     * Get all the server names registered with &IcePack;.
     *
     * @return The server names.
     *
     * @see getServerDescription
     * @see getServerState
     *
     **/
    nonmutating Ice::StringSeq getAllServerNames();

    /**
     *
     * Get the list of endpoints for an adapter.
     *
     * @param id The adapter id.
     *
     * @return The stringified adapter endpoints.
     *
     * @throws AdapterNotExistException Raised if the adapter is not
     * found.
     *
     **/
    nonmutating string getAdapterEndpoints(string id)
	throws AdapterNotExistException, NodeUnreachableException;

    /**
     *
     * Get all the adapter identities registered with &IcePack;.
     *
     * @return The adapter idendities.
     *
     **/
    nonmutating Ice::StringSeq getAllAdapterIds();

    /**
     *
     * Add an object to the object registry. &IcePack; will get the
     * object type by calling [ice_id] on the given proxy. The object
     * must be reachable.
     *
     * @param obj The object to be added to the registry.
     *
     * @throws ObjectDeploymentException Raised if an error occured
     * while trying to register this object. This can occur if the
     * type of the object can't be determine because the object is not
     * reachable.
     *
     * @throws ObjectExistsException Raised if the object is already
     * registered.
     *
     **/
    nonmutating void addObject(Object* obj)
	throws ObjectExistsException, ObjectDeploymentException;

    /**
     *
     * Add an object to the object registry and explicitly specifies
     * its type.
     *
     * @param obj The object to be added to the registry.
     *
     * @param type The obect type.
     *
     * @throws ObjectExistsException Raised if the object is already
     * registered.
     *
     **/
    nonmutating void addObjectWithType(Object* obj, string type)
	throws ObjectExistsException;

    /**
     *
     * Remove an object from the object registry.
     *
     * @param obj The object to be removed from the registry.
     *
     * @throws ObjectNotExistException Raised if the object can't be
     * found.
     *
     **/
    nonmutating void removeObject(Object* obj) 
	throws ObjectNotExistException;

    /**
     *
     * Ping an &IcePack; node to see if it's active.
     *
     * @return true if the node ping succeeded, false otherwise.
     * 
     **/
    nonmutating bool pingNode(string name)
	throws NodeNotExistException;

    /**
     *
     * Shutdown an &IcePack; node.
     * 
     **/
    idempotent void shutdownNode(string name)
	throws NodeNotExistException;

    /**
     *
     * Get all the &IcePack; nodes currently registered.
     *
     * @return The node names.
     *
     **/
    nonmutating Ice::StringSeq getAllNodeNames();

    /**
     *
     * Shut down the &IcePack; registry.
     *
     **/
    idempotent void shutdown();
};

};

#endif
