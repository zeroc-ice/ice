// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_INTERNAL_ICE
#define ICE_GRID_INTERNAL_ICE

#include <Ice/Identity.ice>
#include <Ice/BuiltinSequences.ice>
#include <Ice/ProcessF.ice>
#include <Glacier2/Session.ice>
#include <IceGrid/Admin.ice>
#include <IceGrid/Observer.ice>

module IceGrid
{

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
};

/**
 *
 * This exception is raised if an adapter with the same name already
 * exists.
 *
 **/
exception AdapterExistsException
{
    string id;
};

dictionary<string, Adapter*> AdapterPrxDict;

interface Server
{
    /**
     *
     * Start the server.
     *
     * @return True if the server was successfully started, false
     * otherwise.
     *
     **/
    ["amd"] void start()
	throws ServerStartException;

    /**
     *
     * Stop the server. This methods returns only when the server is
     * deactivated. If the server doesn't stop after a configurable
     * amount of time, it will be killed.
     *
     **/
    ["amd"] void stop()
	throws ServerStopException;
    
    /**
     *
     * Enable or disable the server.
     *
     **/ 
    void setEnabled(bool enable);

    /**
     *
     * Check if the server is enabled.
     *
     **/
    nonmutating bool isEnabled();

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
     * Return the server state.
     *
     * @return The server state.
     *
     * @see ServerState
     *
     **/
    nonmutating ServerState getState();

    /**
     *
     * Get the server pid. Note that the value returned by this method
     * is system dependant. On Unix operating systems, it's the pid
     * value returned by the fork() system call and converted to an
     * integer.
     *
     **/
    nonmutating int getPid();

    /**
     *
     * Set the process proxy.
     *
     **/
    ["ami", "amd"] void setProcess(Ice::Process* proc);
};

interface InternalRegistry;
sequence<InternalRegistry*> InternalRegistryPrxSeq;

interface Node
{
    /**
     *
     * Load the given server. If the server resources weren't already
     * created (database environment directories, property files, etc),
     * they will be created.
     *
     **/
    ["amd", "ami"] idempotent Server* loadServer(string application, 
						 ServerDescriptor desc,
						 string sessionId,
						 out AdapterPrxDict adapters, 
						 out int actTimeout, 
						 out int deactTimeout)
	throws DeploymentException;

    /**
     *
     * Destroy the given server.
     *
     **/
    ["amd", "ami"] idempotent void destroyServer(string name);

    /**
     *
     * Patch application and server distributions. If some servers
     * using a distribution directory to patch are active, this method
     * will raise a PatchException unless shutdown is set to true. In
     * which case the servers will be shutdown.
     * 
     **/
    ["ami"] idempotent void patch(string application, string server, DistributionDescriptor appDistrib, bool shutdown)
	throws  PatchException;

    /**
     *
     * Establish a session to the given replica.
     * 
     **/
    void registerWithReplica(InternalRegistry* replica);

    /**
     *
     * Notification that a replica has been added. The node should 
     * establish a session with this new replica.
     *
     **/
    void replicaAdded(InternalRegistry* replica);

    /**
     *
     * Notification that a replica has been removed. The node should
     * destroy the session to this replica.
     *
     **/
    void replicaRemoved(InternalRegistry* replica);

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
     * Get the node load.
     *
     **/
    nonmutating LoadInfo getLoad();

    /**
     *
     * Shutdown the node.
     *
     **/
    nonmutating void shutdown();
};

sequence<Node*> NodePrxSeq;

/**
 *
 * This exception is raised if a node is already registered and
 * active.
 *
 **/
exception NodeActiveException
{
};

interface NodeSession
{
    /**
     *
     * The node call this method to keep the session alive.
     *
     **/
    void keepAlive(LoadInfo load);

    /**
     *
     * Return the node session timeout.
     *
     **/ 
    nonmutating int getTimeout();

    /**
     *
     * Return the registry observer.
     *
     **/
    nonmutating NodeObserver* getObserver();

    /**
     *
     * Get the name of the servers deployed on the node.
     *
     **/
    nonmutating Ice::StringSeq getServers();

    /**
     *
     * Destroy the session.
     *
     **/
    void destroy();
};

/**
 *
 * This exception is raised if a replica is already registered and
 * active.
 *
 **/
exception ReplicaActiveException
{
};

interface ReplicaSession
{
    /**
     *
     * The replica call this method to keep the session alive.
     *
     **/
    void keepAlive();

    /**
     *
     * Return the replica session timeout.
     *
     **/ 
    nonmutating int getTimeout();

    /**
     *
     * Set the client and server proxies for this replica. This will
     * cause the master registry to update the registry well-known
     * objects endpoints (e.g.: for the IceGrid::Query interface).
     *
     **/
    idempotent void setClientAndServerProxies(Object* serverProxy, Object* clientProxy);

    /**
     *
     * Destroy the session.
     *
     **/
    void destroy();
};

interface InternalRegistry
{
    /**
     *
     * Register a node with the registry. If a node with the same name
     * is already registered, [registerNode] will overide the previous
     * node only if it's not active.
     *
     * @param name The name of the node to register.
     *
     * @param nd The proxy of the node.
     *
     * @param info Some information on the node.
     * 
     * @return The name of the servers currently deployed on the node.
     * 
     * @throws NodeActiveException Raised if the node is already
     * registered and currently active.
     *
     **/
    NodeSession* registerNode(string name, Node* nd, NodeInfo info)
	throws NodeActiveException;

    ReplicaSession* registerReplica(string name, InternalRegistry* prx, RegistryObserver* observer)
	throws ReplicaActiveException;

    void registerWithReplica(InternalRegistry* prx);

    nonmutating NodePrxSeq getNodes();
    nonmutating InternalRegistryPrxSeq getReplicas();
};


};

#endif
