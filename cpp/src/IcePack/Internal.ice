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

#ifndef ICE_PACK_INTERNAL_ICE
#define ICE_PACK_INTERNAL_ICE

#include <Ice/BuiltinSequences.ice>
#include <IcePack/Admin.ice>

module IcePack
{

/**
 *
 * This exception is raised if an adapter is active.
 *
 **/
exception AdapterActiveException
{
};

interface Adapter
{
    /**
     *
     * Get the adapter direct proxy. The adapter direct proxy is a
     * proxy created with the object adapter. The proxy contains the
     * last known adapter endpoints.
     *
     * @param activate If true and if the adapter is not active,
     * [getDirectProxy] will activate the adapter and wait for its
     * activation.
     *
     * @return A direct proxy containing the last known adapter
     * endpoints.
     *
     **/
    Object* getDirectProxy(bool activate);

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
    void setDirectProxy(Object* proxy)
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
    void add(string name, Adapter* adpt)
	throws AdapterExistsException;

    /**
     *
     * Remove an adapter from the registry.
     *
     **/
    void remove(string name)
	throws AdapterNotExistException;
    
    /**
     *
     * Find an adapter and return its proxy.
     *
     * @param name Name of the adapter.
     *
     * @return Adapter proxy.
     *
     **/
    Adapter* findByName(string name)
	throws AdapterNotExistException;

    /**
     *
     * Get all adapter names.
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
     * The adapter server.
     *
     **/
    Server* theServer;

    /**
     *
     * The adapter name.
     *
     **/
    string name;
};

/**
 *
 * A sequence of server adapter proxies.
 *
 **/
sequence<ServerAdapter*> ServerAdapters;

class Server
{
    /**
     *
     * Server description.
     *
     * @return The server description.
     *
     **/
    ServerDescription getServerDescription();

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
     * The description of this server.
     *
     * @return The server description.
     *
     */
    ServerDescription description;

    /**
     *
     * The server adapter proxies.
     *
     **/
    ServerAdapters adapters;

    /**
     *
     * The server activation mode.
     *
     **/
    ServerActivation activation;
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
    void add(string name, Server* svr)
	throws ServerExistsException;

    /**
     *
     * Remove a server from the registry.
     *
     **/
    void remove(string name)
	throws ServerNotExistException;
    
    /**
     *
     * Find a server.
     *
     * @param name Name of the server.
     *
     * @return Server proxy or a null proxy if the server is not
     * found.
     *
     **/
    Server* findByName(string name)
	throws ServerNotExistException;

    /**
     *
     * Get all the server names.
     *
     **/
    nonmutating Ice::StringSeq getAll();
};

interface ServerDeployer
{
    /**
     *
     * Deploy a new server. [add] will create and register a new
     * server with the server registry. The server components
     * described in the given XML descriptor file will also be created
     * or registered with the appropriate services.
     *
     * @param name The server name.
     *
     * @param xmlfile Path to the XML file containing the server
     * deployment descriptor.
     *
     * @param binPath The server path. For C++ servers, this is the
     * path of the executable. For C++ icebox, this is the path of the
     * C++ icebox executable or if empty IcePack will rely on the path
     * to find it. For Java server or Java icebox, this is the path of
     * the java command or if empty IcePack will rely on the path to
     * find it.
     *
     * @param libPath Specify the CLASSPATH value for Java servers,
     * ignored for C++ servers.
     *
     * @param tgts The optional targets to be executed during the
     * deployment.
     *
     **/
    void add(string name, string xmlfile, string binPath, string libPath, Targets tgts)
	throws DeploymentException;

    /**
     *
     * Remove a server. [remove] will destroy the server and
     * unregister it from the server registry. The server components
     * described in the given XML descriptor file will also be
     * destroyed or unregistered with the appropriate services.
     *
     * @param name The server name.
     *
     **/
    void remove(string name)
	throws DeploymentException;
};

interface Node
{
    /**
     *
     * Get the node name.
     *
     **/
    nonmutating string getName();    

    /**
     *
     * Get the node server deployer.
     *
     **/
    nonmutating ServerDeployer* getServerDeployer();

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
