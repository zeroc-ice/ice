// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PACK_ADMIN_ICE
#define ICE_PACK_ADMIN_ICE

#include <Ice/Identity.ice>
#include <Ice/BuiltinSequences.ice>
#include <Ice/SliceChecksumDict.ice>
#include <IceBox/IceBox.ice>
#include <IcePack/Exception.ice>

module IcePack
{

/**
 *
 * The server activation mode.
 *
 **/
enum ServerActivation
{
    /**
     *
     * The server is activated on demand if a client requests one of
     * the server's adapter endpoints and the server is not already
     * running.
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
 * An &Ice; object descriptor.
 * 
 **/
struct ObjectDescriptor
{
    /**
     *
     * The object proxy.
     *
     **/
    Object* proxy;

    /**
     *
     * The object type.
     *
     **/
    string type;
    
    /**
     *
     * The object adapter id.
     *
     **/
    string adapterId;
};

/**
 *
 * A sequence of object descriptors.
 *
 **/
sequence<ObjectDescriptor> ObjectDescriptorSeq;

/**
 *
 * An &Ice; object adapter descriptor.
 *
 **/
struct AdapterDescriptor
{
    /**
     *
     * The object adapter name.
     *
     **/
    string name;

    /**
     *
     * The object adapter id.
     *
     **/
    string id;

    /**
     *
     * The object adapter endpoints.
     *
     **/
    string endpoints;

    /**
     *
     * Flag to specify if the object adapter will register a process object.
     *
     **/
    bool registerProcess;

    /**
     *
     * The object descriptor associated to this object adapter descriptor.
     *
     **/
    ObjectDescriptorSeq objects;
};

/**
 *
 * A sequence of adapter descriptors.
 *
 **/
sequence<AdapterDescriptor> AdapterDescriptorSeq;

/**
 *
 * A configuration property descriptor.
 * 
 **/
struct PropertyDescriptor
{
    /**
     *
     * The name of the property.
     *
     **/
    string name;

    /**
     *
     * The value of the property.
     *
     **/
    string value;
};

/**
 *
 * A sequence of property descriptors.
 *
 **/
sequence<PropertyDescriptor> PropertyDescriptorSeq;

/**
 *
 * A &Freeze; database environment descriptor.
 *
 **/
struct DbEnvDescriptor
{
    /**
     *
     * The name of the database environment.
     *
     **/
    string name;

    /**
     *
     * The home of the database environment (i.e.: the directory where the database file
     * will be stored).
     *
     **/
    string dbHome;

    /**
     *
     * The configuration properties of the database environment.
     *
     **/
    PropertyDescriptorSeq properties;
};

/**
 *
 * A sequence of database environment descriptors.
 *
 **/
sequence<DbEnvDescriptor> DbEnvDescriptorSeq;

/**
 *
 * A component descriptor. A component is either an &Ice; server or
 * an &IceBox; service.
 *
 **/
class ComponentDescriptor
{
    /**
     *
     * The component nane.
     *
     **/
    string name;

    /**
     *
     * The component object adapters.
     *
     **/
    AdapterDescriptorSeq adapters;

    /**
     *
     * The component configuration properties.
     *
     **/
    PropertyDescriptorSeq properties;

    /**
     *
     * The component database environments.
     *
     **/
    DbEnvDescriptorSeq dbEnvs;

    /**
     *
     * Some comments on the component.
     *
     **/
    string comment;
};

/**
 *
 * An &Ice; server descriptor.
 *
 **/
class ServerDescriptor extends ComponentDescriptor
{
    /**
     *
     * The path of the server executable.
     *
     **/
    string exe;

    /**
     *
     * The path to the server working directory.
     *
     **/
    string pwd;

    /**
     *
     * The &IcePack node on which the server is deployed.
     *
     **/
    string node;
    
    /**
     *
     * The name of the application this server belongs to.
     *
     **/
    string application;

    /**
     *
     * The command line options to pass to the server executable.
     *
     **/
    Ice::StringSeq options;

    /**
     *
     * The server environment variables.
     *
     **/
    Ice::StringSeq envs;

    /**
     *
     * The server initial activation mode.
     *
     **/
    ServerActivation activation;
};

/**
 *
 * A sequence of server descriptors.
 *
 **/
sequence<ServerDescriptor> ServerDescriptorSeq;

/**
 *
 * A Java &Ice; server descriptor.
 *
 **/
class JavaServerDescriptor extends ServerDescriptor
{
    /**
     *
     * The name of the Java class containing the main function.
     *
     **/
    string className;

    /**
     *
     * The command line options to pass to the JVM.
     *
     **/ 
    Ice::StringSeq jvmOptions;
};

/**
 *
 * An &IceBox; service descriptor.
 *
 **/
class ServiceDescriptor extends ComponentDescriptor
{
    /**
     *
     * The entry point of the &IceBox; service.
     * 
     **/
    string entry;
};

/**
 *
 * A sequence of service descriptors.
 *
 **/
sequence<ServiceDescriptor> ServiceDescriptorSeq;

/**
 *
 * A C++ &IceBox; server descriptor.
 *
 **/
class CppIceBoxDescriptor extends ServerDescriptor
{
    /**
     *
     * The &IceBox; C++ services.
     * 
     **/
    ServiceDescriptorSeq services;

    /**
     *
     * The endpoints of the &IceBox; service manager interface.
     *
     **/
    string endpoints;
};

/**
 *
 * A Java &IceBox; server descriptor.
 *
 **/
class JavaIceBoxDescriptor extends JavaServerDescriptor
{
    /**
     *
     * The &IceBox; Java services.
     * 
     **/
    ServiceDescriptorSeq services;

    /**
     *
     * The endpoints of the &IceBox; service manager interface.
     *
     **/
    string endpoints;
};

/**
 *
 * An application descriptor.
 *
 **/
class ApplicationDescriptor
{
    /**
     *
     * The application name.
     *
     **/
    string name;
    
    /**
     *
     * The application servers.
     *
     **/
    ServerDescriptorSeq servers;
    
    /**
     *
     * Some comments on the application.
     *
     **/ 
    string comment;
};

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
     * The server is being activated and will change to the active
     * state if the server fork succeeded or to the Inactive state if
     * it failed.
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
 * The &IcePack; administrative interface. <warning><para>Allowing
 * access to this interface is a security risk! Please see the
 * &IcePack; documentation for further information.</para></warning>
 *
 **/
interface Admin
{
    /**
     *
     * Add an application to &IcePack;. An application is a set of servers.
     *
     * @param descriptor The application descriptor.
     *
     * @throws DeploymentException Raised if application deployment failed.
     *
     * @see removeApplication
     *
     **/
    void addApplication(ApplicationDescriptor descriptor)
	throws DeploymentException;    

    /**
     *
     * Update an application. An application is a set of servers.
     *
     * @param descriptor The application descriptor.
     *
     * @throws DeploymentException Raised if application deployment failed.
     *
     * @see removeApplication
     *
     **/
    void updateApplication(ApplicationDescriptor descriptor)
	throws DeploymentException;    

    /**
     *
     * Get all the &IcePack; applications currently registered.
     *
     * @return The application names.
     *
     **/
    nonmutating Ice::StringSeq getAllApplicationNames();

    /**
     *
     * Remove an application from &IcePack;.
     *
     * @param name The application name.
     *
     * @see addApplication
     *
     **/
    void removeApplication(string name)
	throws ApplicationNotExistException;

    /**
     *
     * Get an application descriptor.
     *
     * @param name The application name.
     *
     * @throws ApplicationNotExistException Raised if the application doesn't exist.
     *
     * @returns The application descriptor.
     *
     **/
    nonmutating ApplicationDescriptor getApplicationDescriptor(string name)
	throws ApplicationNotExistException;

    /**
     *
     * Add a server to an &IcePack; node.
     *
     * @param descriptor The server deployment descriptor.
     *
     * @throws DeploymentException Raised if server deployment failed.
     *
     * @see removeServer
     * @see updateServer
     *
     **/
    void addServer(ServerDescriptor server)
	throws DeploymentException;

    /**
     *
     * Update a server.
     *
     * @param descriptor The server deployment descriptor.
     *
     * @throws DeploymentException Raised if server deployment failed.
     *
     * @throws ServerNotExistException Raised if the server doesn't exist.
     *
     * @see addServer
     * @see removeServer
     *
     **/
    void updateServer(ServerDescriptor server)
	throws ServerNotExistException, DeploymentException;

    /**
     *
     * Remove a server from an &IcePack; node.
     *
     * @param name The server name.
     *
     * @throws ServerNotExistException Raised if the server doesn't exist.
     *
     * @see addServer
     * @see updateServer
     *
     **/
    void removeServer(string name)
	throws ServerNotExistException, DeploymentException;

    /**
     *
     * Get a server descriptor.
     *
     * @param name The server name.
     *
     * @throws ServerNotExistException Raised if the server doesn't exist.
     *
     * @returns The server descriptor.
     *
     **/
    nonmutating ServerDescriptor getServerDescriptor(string name)
	throws ServerNotExistException;

    /**
     *
     * Get a server's state.
     *
     * @param name The name of the server.
     *
     * @return The server state.
     * 
     * @throws ServerNotExistException Raised if the server is not
     * found.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     * @see getServerPid
     * @see getAllServerNames
     *
     **/
    nonmutating ServerState getServerState(string name)
	throws ServerNotExistException, NodeUnreachableException;
    
    /**
     *
     * Get a server's system process id. The process id is operating
     * system dependent.
     *
     * @param name The name of the server.
     *
     * @return The server process id.
     * 
     * @throws ServerNotExistException Raised if the server is not
     * found.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     * @see getServerState
     * @see getAllServerNames
     *
     **/
    nonmutating int getServerPid(string name)
	throws ServerNotExistException, NodeUnreachableException;

    /**
     *
     * Get the server's activation mode.
     *
     * @param name The name of the server.
     *
     * @return The server activation mode.
     * 
     * @throws ServerNotExistException Raised if the server is not
     * found.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     * @see getServerState
     * @see getAllServerNames
     *
     **/
    nonmutating ServerActivation getServerActivation(string name)
	throws ServerNotExistException, NodeUnreachableException;

    /**
     *
     * Set the server's activation mode.
     *
     * @param name The name of the server.
     *
     * @return The server activation mode.
     * 
     * @throws ServerNotExistException Raised if the server is not
     * found.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
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
     * @param name The name of the server.
     *
     * @return True if the server was successfully started, false
     * otherwise.
     *
     * @throws ServerNotExistException Raised if the server is not
     * found.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     **/
    bool startServer(string name)
	throws ServerNotExistException, NodeUnreachableException;

    /**
     *
     * Stop a server.
     *
     * @param name The name of the server.
     *
     * @throws ServerNotExistException Raised if the server is not
     * found.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     **/
    void stopServer(string name)
	throws ServerNotExistException, NodeUnreachableException;


    /**
     *
     * Send signal to a server.
     *
     * @param name The name of the server.
     *
     * @param signal The signal, for example SIGTERM or 15.
     *
     * @throws ServerNotExistException Raised if the server is not
     * found.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     * @throws BadSignalException Raised if the signal is not recognized 
     * by the target server.
     *
     **/
    void sendSignal(string name, string signal)
	throws ServerNotExistException, NodeUnreachableException, BadSignalException;

    /**
     *
     * Write message on server stdout or stderr
     *
     * @param name The name of the server.
     *
     * @param message The message.
     *
     * @param fd 1 for stdout, 2 for stderr.
     *
     * @throws ServerNotExistException Raised if the server is not
     * found.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     **/
    void writeMessage(string name, string message, int fd)
	throws ServerNotExistException, NodeUnreachableException;

    /**
     *
     * Get all the server names registered with &IcePack;.
     *
     * @return The server names.
     *
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
     * Get all the adapter ids registered with &IcePack;.
     *
     * @return The adapter ids.
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
     * @throws ObjectExistsException Raised if the object is already
     * registered.
     *
     **/
    void addObject(Object* obj)
	throws ObjectExistsException, DeploymentException;

    /**
     *
     * Add an object to the object registry and explicitly specify
     * its type.
     *
     * @param obj The object to be added to the registry.
     *
     * @param type The object type.
     *
     * @throws ObjectExistsException Raised if the object is already
     * registered.
     *
     **/
    void addObjectWithType(Object* obj, string type)
	throws ObjectExistsException;

    /**
     *
     * Remove an object from the object registry.
     *
     * @param id The identity of the object to be removed from the
     * registry.
     *
     * @throws ObjectNotExistException Raised if the object cannot be
     * found.
     *
     **/
    void removeObject(Ice::Identity id) 
	throws ObjectNotExistException;

    /**
     *
     * Get the object descriptor if the object with the given
     * identity.
     *
     * @param id The identity of the object.
     *
     * @return The object descriptor.
     *
     * @throws ObjectNotExistExcpetion Raised if the object cannot be
     * found.
     *
     **/
    nonmutating ObjectDescriptor getObjectDescriptor(Ice::Identity id)
	throws ObjectNotExistException;

    /**
     *
     * List all the objects registered with the object registry and
     * which have a stringified identity matching the given expression.
     *
     * @param expr The expression to use to match against the
     * stringified identity of the object.
     *
     * @return All the object descriptors with a stringified identity
     * matching the given expression.
     *
     **/
    nonmutating ObjectDescriptorSeq getAllObjectDescriptors(string patt);
    
    /**
     *
     * Ping an &IcePack; node to see if it is active.
     *
     * @param name The node name.
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
     * @param name The node name.
     *
     **/
    idempotent void shutdownNode(string name)
	throws NodeNotExistException, NodeUnreachableException;

    /**
     *
     * Remove the given node and associated servers from the &IcePack; registry.
     *
     * @param name The node name.
     *
     **/
    idempotent void removeNode(string name)
	throws NodeNotExistException;

    /**
     *
     * Get the hostname of this node.
     *
     * @param name The node name.
     *
     **/
    nonmutating string getNodeHostname(string name)
	throws NodeNotExistException, NodeUnreachableException;

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

    /**
     *
     * Returns the checksums for the IcePack Slice definitions.
     *
     * @return A dictionary mapping Slice type ids to their checksums.
     *
     **/
    nonmutating Ice::SliceChecksumDict getSliceChecksums();
};

};

#endif
