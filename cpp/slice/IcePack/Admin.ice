// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PACK_ADMIN_ICE
#define ICE_PACK_ADMIN_ICE

#include <Ice/BuiltinSequences.ice>
#include <IceBox/IceBox.ice>

/**
 *
 * The &Ice; module for object location and activation.
 *
 **/
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
 * This exception is raised if a node doesn't exist.
 *
 **/
exception NodeNotExistException
{
};

/**
 *
 * These exceptions are raised when the deployment of the server failed.
 *
 **/
exception DeploymentException
{
    string component;
    string reason;
};

// TODO: ML: Documentation.
exception ParserDeploymentException extends DeploymentException
{
};

// TODO: ML: Documentation.
exception AdapterDeploymentException extends DeploymentException
{
// TODO: ML: Documentation.
    string adapter;
};

// TODO: ML: Documentation.
exception OfferDeploymentException extends DeploymentException
{
// TODO: ML: Documentation.
    string intf;
// TODO: ML: Documentation.
    Object* proxy;
};

// TODO: ML: Documentation.
exception ServerDeploymentException extends DeploymentException
{
// TODO: ML: Documentation.
    string server;
};

// TODO: ML: Documentation.
exception NodeUnreachableException
{
};

/**
 *
 * A vector of strings representing command line arguments.
 *
 **/
sequence<string> Args;

/**
 *
 * A vector of strings representing targets.
 *
 **/
sequence<string> Targets;

/**
 *
 * An enumeration representing the state of the server.
 *
 **/
enum ServerState
{
    // TODO: ML: Documentation for each individual element.
    Inactive,
    Activating,
    Active,
    Deactivating,
    Destroying,
    Destroyed
};

// TODO: ML: Documentation.
enum ServerActivation
{
    // TODO: ML: Documentation for each individual element.
    OnDemand,
    Manual
};

// TODO: ML: Documentation.
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
     * Deployment descriptor.
     *
     **/
    string descriptor;

    /**
     *
     * Targets used to deploy the server.
     *
     **/
    Targets targets;

    /**
     *
     * The server activation mode.
     *
     **/
    ServerActivation activation;

    /**
     *
     * The optional server path. If none is given, no automatic
     * activation will be performed. This path can be an absolute or
     * relative path (be aware of security risks if you use a relative
     * path).
     *
     * @see args
     * @see pwd
     *
     **/
    string path;

    /**
     *
     * The optional server working directory path. The current or
     * working directory of the server will be changed to this path
     * when the server is started.
     *
     * @see path
     * @see args
     * 
     **/
    string pwd;

    /**
     *
     * The optional server arguments.
     *
     * @see path
     * @see pwd
     *
     **/
    Args args;
    
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
class Admin
{
    /**
     *
     * Add an application to &IcePack;. An application is a set of
     * servers.
     *
     * @param descriptor The application descriptor.
     * 
     * @param tgts The optional targets to deploy. A target is a list
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
    void addApplication(string descriptor, Targets tgts)
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
     * @param tgts The optional targets to deploy. A target is a list
     * of components separated by dots and a target name. For example,
     * the "debug" target of the "service1" in the "server1" will be
     * deployed if the target "server1.service1.debug" is specified.
     *
     * @throws DeploymentException Raised if the deployment of the
     * server failed.
     *
     * @see removeServer
     *
     **/
    void addServer(string node, string name, string path, string libraryPath, string descriptor, Targets tgts)
	throws DeploymentException;

    /**
     *
     * Remove a server from an &IcePack; node.
     *
     * @param name Must match the name of the
     * [ServerDescription::name].
     *
     * @throws DeploymentException Raised if the server deployer
     * failed to remove the server.
     *
     * @throws ServerNotExistException Raised if the server is not
     * found.
     *
     * @see addServer
     *
     **/
    void removeServer(string name)
	throws DeploymentException, ServerNotExistException;

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
     * @see getServerState
     * @see getServerPid
     * @see getAllServerNames
     *
     **/
    nonmutating ServerDescription getServerDescription(string name)
	throws ServerNotExistException;
    
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
     * @see getServerDescription
     * @see getServerPid
     * @see getAllServerNames
     *
     **/
    nonmutating ServerState getServerState(string name)
	throws ServerNotExistException;
    
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
     * @see getServerDescription
     * @see getServerState
     * @see getAllServerNames
     *
     **/
    nonmutating int getServerPid(string name)
	throws ServerNotExistException;
    
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
     **/
    bool startServer(string name)
	throws ServerNotExistException;

    /**
     *
     * Stop a server.
     *
     * @param name Must match the name of [ServerDescription::name].
     *
     * @throws ServerNotExistException Raised if the server is not
     * found.
     *
     **/
    void stopServer(string name)
	throws ServerNotExistException;

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
     * @param name The adapter name.
     *
     * @return The stringified adapter endpoints.
     *
     * @throws AdapterNotExistException Raised if the adapter is not
     * found.
     *
     **/
    nonmutating string getAdapterEndpoints(string name)
	throws AdapterNotExistException;

    /**
     *
     * Get all the adapter names registered with &IcePack;.
     *
     * @return The adapter names.
     *
     **/
    nonmutating Ice::StringSeq getAllAdapterNames();

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
