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
 * regsitration.
 *
 **/
exception AdapterDeploymentException extends DeploymentException
{
    /**
     *
     * The name of the adapter which couldn't be registered.
     *
     **/
    string adapter;
};

/**
 * 
 * This exception is raised when an error occured during the
 * registration of a Yellow offer.
 *
 **/
exception OfferDeploymentException extends DeploymentException
{
    /**
     *
     * The Yellow interface which couldn't be registered with the
     * Yellow service.
     *
     **/
    string intf;
    
    /**
     *
     * The proxy which couldn't be registered with the Yellow service.
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

/**
 *
 * A vector of strings representing command line arguments.
 *
 **/
sequence<string> Args;

/**
 *
 * A vector of strings representing deployment targets.
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
    Targets theTargets;

    /**
     *
     * The server path.
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
    Args theArgs;
    
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
     * Get the server activation mode.
     *
     * @param name Must match the name of [ServerDescription::name].
     *
     * @return The server activation mode.
     * 
     * @throws ServerNotExistException Raised if the server is not
     * found.
     *
     * @see getServerDescription
     * @see getServerState
     * @see getAllServerNames
     *
     **/
    nonmutating ServerActivation getServerActivation(string name)
	throws ServerNotExistException;

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
     * @see getServerDescription
     * @see getServerState
     * @see getAllServerNames
     *
     **/
    void setServerActivation(string name, ServerActivation mode)
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
