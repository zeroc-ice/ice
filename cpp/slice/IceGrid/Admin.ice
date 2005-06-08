// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_ADMIN_ICE
#define ICE_GRID_ADMIN_ICE

#include <Ice/Identity.ice>
#include <Ice/BuiltinSequences.ice>
#include <Ice/SliceChecksumDict.ice>
#include <IceGrid/Exception.ice>
#include <IceGrid/Observer.ice>

module IceGrid
{

dictionary<string, string> StringStringDict;

/**
 *
 * Property descriptor.
 * 
 **/
struct PropertyDescriptor
{
    string name;
    string value;
};
sequence<PropertyDescriptor> PropertyDescriptorSeq;

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
     * The variables defined in the component.
     *
     **/
    StringStringDict variables;

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

struct InstanceDescriptor
{
    /**
     *
     * The template used by this instance.
     * 
     **/
    string template;

    /**
     *
     * The template parameter values.
     *
     **/
    StringStringDict parameterValues;

    /**
     *
     * The instantiated component descriptor (NOTE: this is provided
     * as a convenience. This descriptor can also easily be computed
     * from the template and the instance variables.)
     *
     **/
    ComponentDescriptor descriptor;
    
    /**
     *
     * The targets used to deploy this instance.
     *
     **/
    Ice::StringSeq targets;
};
sequence<InstanceDescriptor> InstanceDescriptorSeq;

struct TemplateDescriptor
{
    /**
     *
     * The template.
     *
     **/
    ComponentDescriptor descriptor;

    /**
     *
     * The name of the parameters required to instantiate the template.
     *
     **/
    Ice::StringSeq parameters;
};
dictionary<string, TemplateDescriptor> TemplateDescriptorDict;

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
     * The &IceGrid node on which the server is deployed.
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
     * The name of the interpreter or empty if the executable isn't
     * interpreted.
     *
     **/
    string interpreter;

    /**
     *
     * The command line options to pass to the interpreter.
     *
     **/
    Ice::StringSeq interpreterOptions;

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
    string activation;
};
dictionary<string, ServerDescriptor> ServerDescriptorDict;


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
dictionary<string, ServiceDescriptor> ServiceDescriptorDict;

/**
 *
 * An &IceBox; server descriptor.
 *
 **/
class IceBoxDescriptor extends ServerDescriptor
{
    /**
     *
     * The service instances.
     *
     **/
    InstanceDescriptorSeq services;

    /**
     *
     * The endpoints of the &IceBox; service manager interface.
     *
     **/
    string endpoints;
};

struct NodeDescriptor
{
    /**
     *
     * The node name.
     * 
     **/
    string name;

    /**
     *
     * The variables defined for the node.
     *
     **/
    StringStringDict variables;
};
sequence<NodeDescriptor> NodeDescriptorSeq;

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
     * The variables defined in the application descriptor.
     *
     **/
    StringStringDict variables;

    /**
     *
     * The server templates.
     *
     **/
    TemplateDescriptorDict serverTemplates;

    /**
     *
     * The service templates.
     *
     **/
    TemplateDescriptorDict serviceTemplates;

    /**
     *
     * The application nodes.
     *
     **/
    NodeDescriptorSeq nodes;
    
    /**
     *
     * The server instances.
     *
     **/
    InstanceDescriptorSeq servers;

    /**
     *
     * The targets used to deploy the application.
     *
     **/
    Ice::StringSeq targets;

    /**
     *
     * Some comments on the application.
     *
     **/ 
    string comment;
};

struct ApplicationUpdateDescriptor
{
    /**
     *
     * The name of the application to update.
     *
     **/
    string name;
    
    /**
     *
     * The variables to update.
     *
     **/
    StringStringDict variables;

    /**
     *
     * The variables to remove.
     *
     **/
    Ice::StringSeq removeVariables;

    /**
     *
     * The server templates to update.
     *
     **/
    TemplateDescriptorDict serverTemplates;

    /**
     *
     * The ids of the server template to remove.
     *
     **/
    Ice::StringSeq removeServerTemplates;

    /**
     *
     * The service templates to update.
     *
     **/
    TemplateDescriptorDict serviceTemplates;

    /**
     *
     * The ids of the service tempate to remove.
     *
     **/
    Ice::StringSeq removeServiceTemplates;

    /**
     *
     * The application nodes to update.
     *
     **/
    NodeDescriptorSeq nodes;
    
    /**
     *
     * The server instances to update.
     *
     **/
    InstanceDescriptorSeq servers;

    /**
     *
     * The name of the server instances to remove.
     *
     **/
    Ice::StringSeq removeServers;
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
 * The &IceGrid; administrative interface. <warning><para>Allowing
 * access to this interface is a security risk! Please see the
 * &IceGrid; documentation for further information.</para></warning>
 *
 **/
interface Admin
{
    /**
     *
     * Add an application to &IceGrid;. An application is a set of servers.
     *
     * @param descriptor The application descriptor.
     *
     * @throws DeploymentException Raised if application deployment failed.
     *
     * @see removeApplication
     *
     **/
    void addApplication(ApplicationDescriptor descriptor)
	throws DeploymentException, ApplicationExistsException;

    /**
     *
     * Synchronize a deployed application with the given application
     * descriptor.
     *
     * @param descriptor The application descriptor.
     *
     * @throws DeploymentException Raised if application deployment
     * failed.
     *
     * @see removeApplication
     *
     **/
    void syncApplication(ApplicationDescriptor descriptor)
	throws DeploymentException, ApplicationNotExistException;

    /**
     *
     * Update a deployed application with the given update application
     * descriptor.
     *
     * @param descriptor The update descriptor.
     *
     * @throws DeploymentException Raised if application deployment
     * failed.
     *
     * @see syncApplication
     * @see removeApplication
     *
     **/
    void updateApplication(ApplicationUpdateDescriptor descriptor)
	throws DeploymentException, ApplicationNotExistException;

    /**
     *
     * Instantiate a server template from the given application.
     *
     * @param application The application where the template is defined.
     *
     * @param template The identifier of the template.
     * 
     * @param node The node where the server will be running.
     * 
     * @param parameters The parameters used to instantiate the template.
     *
     * @throws DeploymentException Raised if the deployment failed.
     *
     **/
    void instantiateServer(string application, string template, string node, StringStringDict parameters)
	throws ApplicationNotExistException, DeploymentException;

    /**
     *
     * Get all the &IceGrid; applications currently registered.
     *
     * @return The application names.
     *
     **/
    nonmutating Ice::StringSeq getAllApplicationNames();

    /**
     *
     * Remove an application from &IceGrid;.
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
     * Get a server descriptor.
     *
     * @param name The server name.
     *
     * @throws ServerNotExistException Raised if the server doesn't exist.
     *
     * @returns The server descriptor.
     *
     **/
    nonmutating InstanceDescriptor getServerDescriptor(string name)
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
     * Get all the server names registered with &IceGrid;.
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
     * Get all the adapter ids registered with &IceGrid;.
     *
     * @return The adapter ids.
     *
     **/
    nonmutating Ice::StringSeq getAllAdapterIds();

    /**
     *
     * Add an object to the object registry. &IceGrid; will get the
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
     * Update an object in the object registry.
     *
     * @param obj The object to be updated to the registry.
     *
     * @throws ObjectNotExistException Raised if the object cannot be
     * found.
     *
     **/
    void updateObject(Object* obj)
	throws ObjectNotExistException;

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
     * Get the descriptors of all registered objects whose stringified
     * identities match the given expression.
     *
     * @param expr The expression to match against the stringified
     * identities of registered objects. The expression may contain
     * a trailing wildcard (<literal>*</literal>) character.
     *
     * @return All the object descriptors with a stringified identity
     * matching the given expression.
     *
     **/
    nonmutating ObjectDescriptorSeq getAllObjectDescriptors(string expr);
    
    /**
     *
     * Ping an &IceGrid; node to see if it is active.
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
     * Shutdown an &IceGrid; node.
     * 
     * @param name The node name.
     *
     **/
    idempotent void shutdownNode(string name)
	throws NodeNotExistException, NodeUnreachableException;

    /**
     *
     * Remove the given node and associated servers from the &IceGrid; registry.
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
     * Get all the &IceGrid; nodes currently registered.
     *
     * @return The node names.
     *
     **/
    nonmutating Ice::StringSeq getAllNodeNames();

    /**
     *
     * Shut down the &IceGrid; registry.
     *
     **/
    idempotent void shutdown();

    /**
     *
     * Returns the checksums for the IceGrid Slice definitions.
     *
     * @return A dictionary mapping Slice type ids to their checksums.
     *
     **/
    nonmutating Ice::SliceChecksumDict getSliceChecksums();
};

};

#endif
