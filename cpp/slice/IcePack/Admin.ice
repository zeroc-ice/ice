
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

/**
 *
 * The &Ice; module for object location and activation.
 *
 **/
module IcePack
{

/**
 *
 * This exception is raised if an adapter already exists.
 *
 **/
exception AdapterExistsException
{
};

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
 * A sequence of adapter names.
 *
 **/
sequence<string> AdapterNames;

/**
 *
 * This exception is raised if a server already exists.
 *
 **/
exception ServerExistsException
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
 * This exception is raised if an operation on a server failed because
 * the operation requires the server to be inactive.
 *
 **/
exception ServerNotInactiveException
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

exception ParserDeploymentException extends DeploymentException
{
};

exception AdapterDeploymentException extends DeploymentException
{
    string adapter;
};

exception OfferDeploymentException extends DeploymentException
{
    string intf;
    Object* proxy;
};

exception ServerDeploymentException extends DeploymentException
{
    string server;
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
    Inactive,
    Activating,
    Active,
    Deactivating,
    Destroyed
};

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
     * The optional server adapter names. If a client makes a request
     * to locate an adapter from the server, the server will be
     * automatically started if it's not already active. If empty, no
     * automatic activation of the server will be performed.
     *
     **/
    AdapterNames adapters;

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
     * True if the server is an &IceBox;. &IceBox; servers are
     * shutdown using the service manager instead of relying on
     * signals.
     *
     **/
    bool isIceBox;
};

/**
 *
 * A sequence of server names.
 *
 **/
sequence<string> ServerNames;

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
     * Add a server to &IcePack;.
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
     * @throws ServerExistsException Raised if a server with the same
     * name already exists.
     *
     * @throws DeploymentException Raised if the deployment of the
     * server failed.
     *
     * @see removeServer
     *
     **/
    void addServer(string name, string path, string libraryPath, string descriptor, Targets tgts)
	throws DeploymentException;

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
     * Remove a server and the adapters implemented by that server
     * from &IcePack;. The server needs to be inactive for this
     * operation to succeed.
     *
     * @param name Must match the name of the
     * [ServerDescription::name].
     *
     * @throws ServerNotExistException Raised if the server is not
     * found.
     *
     * @throws SeverNotInactiveException Raised if the server is not
     * in the inactive state.
     *
     * @see addServer
     *
     **/
    void removeServer(string name)
	throws DeploymentException, ServerNotExistException, ServerNotInactiveException;

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
    nonmutating ServerNames getAllServerNames();

    /**
     *
     * Add an adapter with a list of endpoints to &IcePack;.
     *
     * @param name The adapter's name.
     *
     * @param endpoints The list of endpoints for the adapter.
     *
     * @throws AdapterExistsException Raised if an adapter with the
     * same name already exists.
     *
     * @see removeAdapter
     *
     **/
    void addAdapterWithEndpoints(string name, string endpoints)
	throws AdapterExistsException;

    /**
     *
     * Remove an adapter from &IcePack;.
     *
     * @param name The adapter name.
     *
     * @throws AdapterNotExistException Raised if the adapter is not
     * found.
     *
     **/
    void removeAdapter(string name)
	throws AdapterNotExistException;

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
    nonmutating AdapterNames getAllAdapterNames();

    /**
     *
     * Shut down &IcePack;.
     *
     **/
    void shutdown();
};

};

#endif
