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
#include <IceGrid/Descriptor.ice>

module IceGrid
{

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
     * state when the registered server object adapters are activated.
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
 * A dictionary of proxies.
 *
 **/
dictionary<string, Object*> StringObjectProxyDict;

/**
 *
 * Information on an Ice object.
 *
 **/
struct ObjectInfo
{
    /**
     *
     * The proxy of the object. 
     *
     **/
    Object* proxy;

    /** 
     *
     * The type of the object. 
     *
     **/
    string type;
};

/**
 *
 * A sequence of object information structures.
 *
 **/
sequence<ObjectInfo> ObjectInfoSeq;

/**
 *
 * Information on a server managed by an IceGrid node.
 *
 **/
struct ServerInfo
{
    /**
     *
     * The server application. 
     *
     **/
    string application;

    /** 
     *
     * The server node. 
     *
     **/
    string node;
       
    /**
     *
     * The server descriptor. 
     *
     **/
    ServerDescriptor descriptor;
};

struct NodeInfo
{
    /**
     *
     * The operating system name.
     *
     **/
    string os;

    /**
     *
     * The network name of the host running this node (as defined in
     * uname()).
     *
     **/
    string hostname;

    /**
     *
     * The operation system release level (as defined in uname()).
     * 
     **/
    string release;

    /**
     *
     * The operation system version (as defined in uname()).
     *
     **/
    string version;

    /**
     *
     * The machine harware type (as defined in uname()).
     *
     **/
    string machine;    

    /**
     *
     * The number of processors.
     *
     **/
    int nProcessors;
    
    /**
     *
     * The path to the node data directory.
     *
     **/
    string dataDir;
};

struct LoadInfo
{
    /** The load average over the past minute. */
    float avg1;

    /** The load average over the past 5 minutes. */
    float avg5;

    /** The load average over the past 15 minutes. */
    float avg15;
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
     * @throws DeploymentException Raised if application deployment
     * failed.
     *
     **/
    void addApplication(ApplicationDescriptor descriptor)
	throws DeploymentException;

    /**
     *
     * Synchronize a deployed application with the given application
     * descriptor. This operation will replace the current descriptor
     * with this new descriptor.
     *
     * @param descriptor The application descriptor.
     *
     * @throws DeploymentException Raised if application deployment
     * failed.
     *
     * @throws ApplicationNotExistException Raised if the application
     * doesn't exist.
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
     * @throws ApplicationNotExistException Raised if the application
     * doesn't exist.
     *
     **/
    void updateApplication(ApplicationUpdateDescriptor descriptor)
	throws DeploymentException, ApplicationNotExistException;

    /**
     *
     * Remove an application from &IceGrid;.
     *
     * @param name The application name.
     *
     * @throws ApplicationNotExistException Raised if the application
     * doesn't exist.
     *
     **/
    void removeApplication(string name)
	throws ApplicationNotExistException;

    /**
     *
     * Instantiate a server template from an application.
     *
     **/
    void instantiateServer(string application, string node, ServerInstanceDescriptor desc)
	throws ApplicationNotExistException, DeploymentException;

    /**
     *
     * Patch the given application data. If the patch argument is an
     * empty string, all the application servers depending on patch
     * data will be patched.
     *
     * @param name The application name.
     *
     * @param shutdown If true, the servers depending on the data to
     * patch will be shutdown if necessary.
     *
     * @throws PatchException Raised if the patch failed.
     *
     * @throws ApplicationNotExistException Raised if the application
     * doesn't exist.
     *
     **/
    ["ami"] void patchApplication(string name, bool shutdown)
	throws ApplicationNotExistException, PatchException;

    /**
     *
     * Get an application descriptor.
     *
     * @param name The application name.
     *
     * @returns The application descriptor.
     *
     * @throws ApplicationNotExistException Raised if the application
     * doesn't exist.
     *
     **/
    nonmutating ApplicationDescriptor getApplicationDescriptor(string name)
	throws ApplicationNotExistException;

    /**
     *
     * Get the default application descriptor.
     *
     **/
    nonmutating ApplicationDescriptor getDefaultApplicationDescriptor()
	throws DeploymentException;

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
     * Get the server information for the server with the given id.
     *
     * @param id The server id.
     *
     * @throws ServerNotExistException Raised if the server doesn't exist.
     *
     * @returns The server information.
     *
     **/
    nonmutating ServerInfo getServerInfo(string id)
	throws ServerNotExistException;

    /**
     *
     * Get a server's state.
     *
     * @param id The id of the server.
     *
     * @return The server state.
     * 
     * @throws ServerNotExistException Raised if the server doesn't exist.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     **/
    nonmutating ServerState getServerState(string id)
	throws ServerNotExistException, NodeUnreachableException;
    
    /**
     *
     * Get a server's system process id. The process id is operating
     * system dependent.
     *
     * @param id The id of the server.
     *
     * @return The server process id.
     * 
     * @throws ServerNotExistException Raised if the server doesn't exist.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     **/
    nonmutating int getServerPid(string id)
	throws ServerNotExistException, NodeUnreachableException;

    /**
     *
     * Enable or disable a server. A disabled server can't be started
     * on demand or administratively. The enable state of the server
     * is not persistent, if the node is shutdown and restarted, the
     * server will be enabled by default.
     *
     * @param id The id of the server.
     *
     * @param enabled True to enable the server, false to disable it.
     *
     * @throws ServerNotExistException Raised if the server doesn't exist.
     *
     * @throws NodeUnreachableException Raised if the node could not
     * be reached.
     *
     **/
    ["ami"] idempotent void enableServer(string id, bool enabled)
	throws ServerNotExistException, NodeUnreachableException;

    /**
     *
     * Check if the server is enabled or disabled.
     *
     * @param id The id of the server.
     *
     * @throws ServerNotExistException Raised if the server doesn't
     * exist.
     *
     * @throws NodeUnreachableException Raised if the node could not
     * be reached.
     * 
     **/
    nonmutating bool isServerEnabled(string id)
	throws ServerNotExistException, NodeUnreachableException;

    /**
     *
     * Start a server and wait for its activation.
     *
     * @param id The id of the server.
     *
     * @return True if the server was successfully started, false
     * otherwise.
     *
     * @throws ServerNotExistException Raised if the server doesn't
     * exist.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     **/
    ["ami"] void startServer(string id)
	throws ServerNotExistException, ServerStartException, NodeUnreachableException;

    /**
     *
     * Stop a server.
     *
     * @param id The id of the server.
     *
     * @throws ServerNotExistException Raised if the server doesn't
     * exist.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     **/
    ["ami"] void stopServer(string id)
	throws ServerNotExistException, NodeUnreachableException;

    /**
     *
     * Patch a server.
     *
     * @param id The id of the server.
     *
     * @param shutdown If true, servers depending on the data to patch
     * will be shutdown if necessary.
     *
     * @throws ServerNotExistException Raised if the server doesn't
     * exist.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     * @throws PatchException Raised if the patch failed.
     *
     **/
    ["ami"] void patchServer(string id, bool shutdown)
	throws ServerNotExistException, NodeUnreachableException, PatchException;

    /**
     *
     * Send signal to a server.
     *
     * @param id The id of the server.
     *
     * @param signal The signal, for example SIGTERM or 15.
     *
     * @throws ServerNotExistException Raised if the server doesn't
     * exist.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     * @throws BadSignalException Raised if the signal is not recognized 
     * by the target server.
     *
     **/
    void sendSignal(string id, string signal)
	throws ServerNotExistException, NodeUnreachableException, BadSignalException;

    /**
     *
     * Write message on server stdout or stderr
     *
     * @param id The id of the server.
     *
     * @param message The message.
     *
     * @param fd 1 for stdout, 2 for stderr.
     *
     * @throws ServerNotExistException Raised if the server doesn't
     * exist.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     **/
    void writeMessage(string id, string message, int fd)
	throws ServerNotExistException, NodeUnreachableException;

    /**
     *
     * Get all the server ids registered with &IceGrid;.
     *
     * @return The server ids.
     *
     **/
    nonmutating Ice::StringSeq getAllServerIds();

    /**
     *
     * Get the list of endpoints for an adapter.
     *
     * @param adapterId The adapter id.
     *
     * @return A dictionary of adapter direct proxy classified by
     * server id.
     *
     * @throws AdapterNotExistException Raised if the adapter doesn't
     * exist.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     **/
    nonmutating StringObjectProxyDict getAdapterEndpoints(string adapterId)
	throws AdapterNotExistException, NodeUnreachableException;

    /**
     *
     * Remove the adapter with the given id.
     *
     * @throws AdapterNotExistException Raised if the adapter doesn't
     * exist.
     *
     **/
    idempotent void removeAdapter(string adapterId)
	throws AdapterNotExistException, DeploymentException;

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
     * @throws ObjectNotRegisteredException Raised if the object doesn't
     * exist.
     *
     **/
    void updateObject(Object* obj)
	throws ObjectNotRegisteredException, DeploymentException;

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
	throws ObjectExistsException, DeploymentException;

    /**
     *
     * Remove an object from the object registry.
     *
     * @param id The identity of the object to be removed from the
     * registry.
     *
     * @throws ObjectNotRegisteredException Raised if the object doesn't
     * exist.
     *
     **/
    void removeObject(Ice::Identity id) 
	throws ObjectNotRegisteredException, DeploymentException;

    /**
     *
     * Get the object info for the object with the given identity.
     *
     * @param id The identity of the object.
     *
     * @return The object info.
     *
     * @throws ObjectNotRegisteredException Raised if the object doesn't
     * exist.
     *
     **/
    nonmutating ObjectInfo getObjectInfo(Ice::Identity id)
	throws ObjectNotRegisteredException;

    /**
     *
     * Get the object info of all the registered objects whose stringified
     * identities match the given expression.
     *
     * @param expr The expression to match against the stringified
     * identities of registered objects. The expression may contain
     * a trailing wildcard (<literal>*</literal>) character.
     *
     * @return All the object infos with a stringified identity
     * matching the given expression.
     *
     **/
    nonmutating ObjectInfoSeq getAllObjectInfos(string expr);
    
    /**
     *
     * Ping an &IceGrid; node to see if it is active.
     *
     * @param name The node name.
     *
     * @return true if the node ping succeeded, false otherwise.
     * 
     * @throws NodeNotExistException Raised if the node doesn't exist.
     *
     **/
    nonmutating bool pingNode(string name)
	throws NodeNotExistException;

    /**
     *
     * Get the load averages of the node.
     *
     * @param name The node name.
     *
     * @return The node load information.
     * 
     * @throws NodeNotExistException Raised if the node doesn't exist.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     **/
    nonmutating LoadInfo getNodeLoad(string name)
	throws NodeNotExistException, NodeUnreachableException;

    /**
     *
     * Get the node information for the node with the given name.
     *
     * @param name The node name.
     *
     * @return The node information.
     * 
     * @throws NodeNotExistException Raised if the node doesn't exist.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     **/
    nonmutating NodeInfo getNodeInfo(string name)
	throws NodeNotExistException, NodeUnreachableException;
    
    /**
     *
     * Shutdown an &IceGrid; node.
     * 
     * @param name The node name.
     *
     * @throws NodeNotExistException Raised if the node doesn't exist.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     **/
    idempotent void shutdownNode(string name)
	throws NodeNotExistException, NodeUnreachableException;

    /**
     *
     * Get the hostname of this node.
     *
     * @param name The node name.
     *
     * @throws NodeNotExistException Raised if the node doesn't exist.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
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
