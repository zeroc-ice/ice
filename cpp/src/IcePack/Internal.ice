// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PACK_INTERNAL_ICE
#define ICE_PACK_INTERNAL_ICE

#include <Ice/Identity.ice>
#include <Ice/BuiltinSequences.ice>
#include <Ice/ProcessF.ice>
#include <IcePack/Admin.ice>

module IcePack
{

/**
 *
 * The object registry interface.
 *
 **/
interface ObjectRegistry
{
    /**
     *
     * Add an object to the registry.
     *
     **/
    void add(ObjectDescriptor desc)
	throws ObjectExistsException;

    /**
     *
     * Remove an object from the registry.
     *
     **/
    void remove(Ice::Identity id)
	throws ObjectNotExistException;

    /**
     *
     * Find an object by identity and returns its description.
     *
     **/
    nonmutating ObjectDescriptor getObjectDescriptor(Ice::Identity id)
	throws ObjectNotExistException;

    /**
     *
     * Find an object by identity and return its proxy.
     *
     **/
    nonmutating Object* findById(Ice::Identity id)
	throws ObjectNotExistException;

    /**
     *
     * Find an object by type and return its proxy.
     *
     **/
    nonmutating Object* findByType(string type);

    /**
     *
     * Find all the objects with the given type.
     *
     **/
    nonmutating Ice::ObjectProxySeq findAllWithType(string type);

    /**
     *
     * Find all the objects matching the given expression.
     *
     **/
    nonmutating ObjectDescriptorSeq findAll(string expression);
};

/**
 *
 * This exception is raised if an adapter is active.
 *
 **/
exception AdapterActiveException
{
};

exception AdapterNotActiveException
{
    /** True if the adapter can be activated on demand. */
    bool activatable;
    
    /** How long to wait for the adapter to become active. */
    int timeout;
};

interface Adapter
{
    /**
     *
     * Activate this adapter. If this adapter can be activated, this 
     * will activate the adapter and return the direct proxy of the 
     * adapter once it's active. If this adapter can be activated on
     * demand, this will return 0 if the adapter is inactive or the
     * adapter direct proxy it's active.
     *
     **/
    ["ami", "amd"] Object* activate();    

    /**
     *
     * Get the adapter direct proxy. The adapter direct proxy is a
     * proxy created with the object adapter. The proxy contains the
     * last known adapter endpoints.
     *
     * @return A direct proxy containing the last known adapter
     * endpoints if the adapter is already active.
     *
     **/
     ["ami"] nonmutating Object* getDirectProxy()
	throws AdapterNotActiveException;

    /**
     *
     * Set the direct proxy for this adapter.
     *
     * @param The direct proxy. The direct proxy should be created
     * with the object adapter and should contain the object adapter
     * endpoints.
     *
     * @throws AdapterActiveException The adapter is already
     * active. It's not possible to override the direct proxy of an
     * active adapter.
     *
     **/
    ["ami"] void setDirectProxy(Object* proxy)
	throws AdapterActiveException;

    /**
     *
     * Destroy the adapter.
     *
     **/
    void destroy();
};

/**
 *
 * This exception is raised if an adapter with the same name already
 * exists.
 *
 **/
exception AdapterExistsException
{
};

/**
 *
 * The adapter registry interface.
 *
 **/
interface AdapterRegistry
{
    /**
     *
     * Add an adapter to the registry.
     *
     **/
    void add(string id, Adapter* adpt)
	throws AdapterExistsException;

    /**
     *
     * Remove an adapter from the registry. If the given adapter proxy is not null, the adapter will
     * be removed from the registry only if the proxy matches.
     *
     **/
    Adapter* remove(string id, Adapter* adpt)
	throws AdapterNotExistException;
    
    /**
     *
     * Find an adapter and return its proxy.
     *
     **/
    Adapter* findById(string id)
	throws AdapterNotExistException;

    /**
     *
     * Get all adapter ids.
     *
     **/
    nonmutating Ice::StringSeq getAll();
};

/**
 *
 * A standalone adapter doesn't provide on demand activation. It just
 * store the adapter endpoints in the proxy attribute.
 *
 **/
class StandaloneAdapter implements Adapter
{
    /**
     *
     * The adapter direct proxy.
     *
     **/
    Object* proxy;    
};

class Server;

/**
 *
 * This class implements the [Adapter] interface and provides on
 * demand server activation when the adapter endpoints are requested
 * through the [getDirectProxy] method.
 *
 **/
class ServerAdapter implements Adapter
{
    /**
     *
     * Returns the adaper id.
     *
     **/
    string getId();

    /**
     *
     * The adapter server.
     *
     **/
    Server* svr;

    /**
     *
     * The adapter id.
     *
     **/
    string id;
};

/**
 *
 * A sequence of server adapter proxies.
 *
 **/
dictionary<Ice::Identity, ServerAdapter*> ServerAdapterPrxDict;

class Server
{
    /**
     *
     * Start the server.
     *
     * @param mode The activation mode requested, start returns false
     * if the activation mode requested is not compatible with the
     * server activation mode. For example if mode is Automatic and
     * the server activation mode is Manual the start will return
     * false.
     *
     * @return True if the server was successfully started, false
     * otherwise.
     *
     **/
    bool start(ServerActivation mode);

    /**
     *
     * Stop the server. This methods returns only when the server is
     * deactivated. If the server doesn't stop after a configurable
     * amount of time, it will be killed.
     *
     **/
    void stop();

    /**
     *
     * Send signal to the server
     *
     **/
    void sendSignal(string signal) 
	throws BadSignalException;
    
    /**
     *
     * Write message on servers' stdout or stderr.
     *
     **/
    void writeMessage(string message, int fd);

    /**
     *
     * Destroy the server. This method destroys the server and
     * eventually deactivates if it's still active.
     *
     **/
    void destroy();
    
    /**
     *
     * This method is called by the activator when it detects that the
     * server has terminated.
     *
     **/
    void terminated();

    /**
     *
     * Return the server state.
     *
     * @return The server state.
     *
     * @see ServerState
     *
     **/
    ServerState getState();

    /**
     *
     * Get the server pid. Note that the value returned by this method
     * is system dependant. On Unix operating systems, it's the pid
     * value returned by the fork() system call and converted to an
     * integer.
     *
     **/
    int getPid();

    /**
     *
     * Get the descriptor used to deploy this server.
     *
     **/
    ServerDescriptor getDescriptor();

    /**
     *
     * Set the server activation mode.
     *
     **/
    void setActivationMode(ServerActivation mode);

    /**
     *
     * Get the server activation mode.
     *
     **/
    ServerActivation getActivationMode();

    /**
     *
     * Set the process proxy.
     *
     **/
    ["ami"] void setProcess(Ice::Process* proc);

    /**
     *
     * Set the server executable path.
     *
     **/
    void setExePath(string name);
    
    /**
     *
     * Set the path of the server working directory.
     *
     **/ 
    void setPwd(string path);
    
    /**
     *
     * Set the server environment variables.
     *
     **/
    void setEnvs(Ice::StringSeq envs);

    /**
     *
     * Set the server command line options.
     *
     **/
    void setOptions(Ice::StringSeq options);
    
    /**
     *
     * Add an adapter to this server.
     *
     **/
    void addAdapter(ServerAdapter* adapter, bool registerProcess)
	throws DeploymentException;

    /**
     *
     * Remove an adapter from this server.
     *
     **/
    void removeAdapter(ServerAdapter* adapter);

    /**
     *
     * Add a configuration file.
     *
     **/
    string addConfigFile(string name, PropertyDescriptorSeq properties)
	throws DeploymentException;

    /**
     *
     * Remove a configuration file.
     *
     **/
    void removeConfigFile(string name);

    /**
     *
     * Add a database environment.
     *
     **/
    string addDbEnv(DbEnvDescriptor dbEnv, string path)
	throws DeploymentException;

    /**
     *
     * Remove a database environment.
     *
     **/
    void removeDbEnv(DbEnvDescriptor dbEnv, string path);
    
    /** The server name. */
    string name;

    /** The path of the server executable. */
    string exePath;

    /** The server environment variables. */
    Ice::StringSeq envs;

    /** The server command line options. */
    Ice::StringSeq options;

    /** The path to the server working directory. */
    string pwd;

    /** The server adapter proxies. */
    ServerAdapterPrxDict adapters;

    /** The server activation mode. */
    ServerActivation activation;

    /** True if an adapter is configured to register a process object. */
    bool processRegistered;

    /** The descriptor used to deploy this server. */
    ServerDescriptor descriptor;
};

/**
 *
 * This exception is raised if a server with the same name already
 * exists.
 *
 **/
exception ServerExistsException
{
};

interface ServerRegistry
{
    /**
     *
     * Add a server to the registry.
     *
     **/
    void add(string name, Server* svr, ServerDescriptor descriptor)
	throws ServerExistsException;

    /**
     *
     * Remove a server from the registry.
     *
     **/
    Server* remove(string name)
	throws ServerNotExistException;
    
    /**
     *
     * Find a server.
     *
     * @param name Name of the server.
     *
     * @return Server proxy.
     *
     **/
    Server* findByName(string name)
	throws ServerNotExistException;

    /**
     *
     * Get a server descriptor.
     *
     **/
    ServerDescriptor getDescriptor(string name)
        throws ServerNotExistException;

    /**
     *
     * Get all the server names.
     *
     **/
    nonmutating Ice::StringSeq getAll();

    /**
     *
     * Get all the server descriptors for servers deployed on the given node.
     *
     **/
    nonmutating ServerDescriptorSeq getAllDescriptorsOnNode(string node);
};

/**
 *
 * This exception is raised if a server with the same name already
 * exists.
 *
 **/
exception ApplicationExistsException
{
};

interface ApplicationRegistry
{
    /**
     *
     * Add an application to the registry.
     *
     **/
    void add(string name)
	throws ApplicationExistsException;

    /**
     *
     * Remove an application from the registry.
     *
     **/
    void remove(string name)
	throws ApplicationNotExistException;

    /**
     *
     * Register a server with the given application.
     *
     **/
    void registerServer(string application, string name)
	throws ApplicationNotExistException;

    /**
     *
     * Unregister a server from the given application.
     *
     **/
    void unregisterServer(string application, string name)
	throws ApplicationNotExistException;

    /**
     *
     * Get an application descriptor.
     *
     **/
    ApplicationDescriptor getDescriptor(string name)
        throws ApplicationNotExistException;

    /**
     *
     * Get all the application names.
     *
     **/
    nonmutating Ice::StringSeq getAll();
};

interface Node
{
    /**
     *
     * Create a new server on this node.
     *
     * @param The name of the server.
     *
     * @param The descriptor of the server.
     *
     **/
    nonmutating Server* createServer(string name, ServerDescriptor desc)
	throws DeploymentException;

    /**
     *
     * Create a new adapter of a given server on this node.
     *
     * @param server The server associted to the adapter.
     *
     * @param id The id of the adapter.
     *
     **/
    nonmutating ServerAdapter* createServerAdapter(Server* srv, string id);

    /**
     *
     * Create a temporary directory.
     *
     **/
    nonmutating string createTmpDir();

    /**
     *
     * Destroy a temporary directory.
     *
     **/
    nonmutating void destroyTmpDir(string path);

    /**
     *
     * Get the node name.
     *
     **/
    nonmutating string getName();    

    /**
     *
     * Get the node hostname.
     *
     **/
    nonmutating string getHostname();    

    /**
     *
     * Shutdown the node.
     *
     **/
    nonmutating void shutdown();
};

/**
 *
 * This exception is raised if a node is already registered and
 * active.
 *
 **/
exception NodeActiveException
{
};

interface NodeRegistry
{
    /**
     *
     * Add a node to the registry. If a node with the same name is
     * already registered, [add] will overide the previous node only
     * if it's not active.
     *
     * @throws NodeActiveException Raised if the node is already
     * registered and currently active.
     *
     **/
    void add(string name, Node* nd)
	throws NodeActiveException;

    /**
     *
     * Remove a node from the registry.
     *
     **/
    void remove(string name)
	throws NodeNotExistException;
    
    /**
     *
     * Find a node.
     *
     * @param name Name of the node.
     *
     * @return Node proxy or a null proxy if the node is not found.
     *
     **/
    Node* findByName(string name)
	throws NodeNotExistException;

    /**
     *
     * Get all the node names.
     *
     **/
    nonmutating Ice::StringSeq getAll();
};

};

#endif
