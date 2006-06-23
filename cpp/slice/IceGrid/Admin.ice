// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
#include <Glacier2/Session.ice>
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
 * Information about an Ice object.
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
 * Information about an adapter registered with the &IceGrid; registry.
 *
 **/
struct AdapterInfo
{
    /** 
     *
     * The id of the adapter.
     *
     **/
    string id;

    /**
     *
     * A dummy direct proxy that contains the adapter endpoints. 
     *
     **/
    Object* proxy;

    /** 
     *
     * The replica group id of the object adapter, or empty if the
     * adapter doesn't belong to a replica group.
     *
     **/
    string replicaGroupId;
};

/**
 *
 * A sequence of adapter information structures.
 *
 **/
sequence<AdapterInfo> AdapterInfoSeq;

/**
 *
 * Information about a server managed by an &IceGrid; node.
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

/**
 *
 * Information about an &IceGrid; node.
 *
 **/
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
     * The machine hardware type (as defined in uname()).
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

/**
 *
 * Information about the load of a node.
 *
 **/
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
     * Add an application to &IceGrid;.
     *
     * @param descriptor The application descriptor.
     *
     * @throws AccessDeniedException Raised if the session doesn't
     * hold the exclusive lock or if another session is holding the
     * lock.
     *
     * @throws DeploymentException Raised if application deployment
     * failed.
     *
     **/
    void addApplication(ApplicationDescriptor descriptor)
	throws AccessDeniedException, DeploymentException;

    /**
     *
     * Synchronize a deployed application with the given application
     * descriptor. This operation will replace the current descriptor
     * with this new descriptor.
     *
     * @param descriptor The application descriptor.
     *
     * @throws AccessDeniedException Raised if the session doesn't
     * hold the exclusive lock or if another session is holding the
     * lock.
     *
     * @throws DeploymentException Raised if application deployment
     * failed.
     *
     * @throws ApplicationNotExistException Raised if the application
     * doesn't exist.
     *
     **/
    void syncApplication(ApplicationDescriptor descriptor)
	throws AccessDeniedException, DeploymentException, ApplicationNotExistException;

    /**
     *
     * Update a deployed application with the given update application
     * descriptor.
     *
     * @param descriptor The update descriptor.
     *
     * @throws AccessDeniedException Raised if the session doesn't
     * hold the exclusive lock or if another session is holding the
     * lock.
     *
     * @throws DeploymentException Raised if application deployment
     * failed.
     *
     * @throws ApplicationNotExistException Raised if the application
     * doesn't exist.
     *
     **/
    void updateApplication(ApplicationUpdateDescriptor descriptor)
	throws AccessDeniedException, DeploymentException, ApplicationNotExistException;

    /**
     *
     * Remove an application from &IceGrid;.
     *
     * @param name The application name.
     *
     * @throws AccessDeniedException Raised if the session doesn't
     * hold the exclusive lock or if another session is holding the
     * lock.
     *
     * @throws ApplicationNotExistException Raised if the application
     * doesn't exist.
     *
     **/
    void removeApplication(string name)
	throws AccessDeniedException, ApplicationNotExistException;

    /**
     *
     * Instantiate a server template from an application on the given
     * node.
     *
     * @param application The application name.
     * 
     * @param node The name of the node where the server will be
     * deployed.
     *
     * @param desc The descriptor of the server instance to deploy.
     * 
     * @throws AccessDeniedException Raised if the session doesn't
     * hold the exclusive lock or if another session is holding the
     * lock.
     *
     * @throws DeploymentException Raised if server instantiation
     * failed.
     *
     * @throws ApplicationNotExistException Raised if the application
     * doesn't exist.
     *
     **/
    void instantiateServer(string application, string node, ServerInstanceDescriptor desc)
	throws AccessDeniedException, ApplicationNotExistException, DeploymentException;

    /**
     *
     * Patch the given application data.
     *
     * @param name The application name.
     *
     * @param shutdown If true, the servers depending on the data to
     * patch will be shutdown if necessary.
     *
     * @throws ApplicationNotExistException Raised if the application
     * doesn't exist.
     *
     * @throws PatchException Raised if the patch failed.
     *
     **/
    ["ami", "amd"] void patchApplication(string name, bool shutdown)
	throws ApplicationNotExistException, PatchException;

    /**
     *
     * Get an application descriptor.
     *
     * @param name The application name.
     *
     * @return The application descriptor.
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
     * @throws DeploymentException Raised if the default application
     * descriptor can't be accessed or is invalid.
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
     * @return The server information.
     *
     **/
    nonmutating ServerInfo getServerInfo(string id)
	throws ServerNotExistException;

    /**
     *
     * Get a server's state.
     *
     * @param id The server id.
     *
     * @return The server state.
     * 
     * @throws ServerNotExistException Raised if the server doesn't exist.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     * @throws DeploymentException Raised if the server couldn't be
     * deployed on the node.
     *
     **/
    nonmutating ServerState getServerState(string id)
	throws ServerNotExistException, NodeUnreachableException, DeploymentException;
    
    /**
     *
     * Get a server's system process id. The process id is operating
     * system dependent.
     *
     * @param id The server id.
     *
     * @return The server's process id.
     * 
     * @throws ServerNotExistException Raised if the server doesn't exist.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     * @throws DeploymentException Raised if the server couldn't be
     * deployed on the node.
     *
     **/
    nonmutating int getServerPid(string id)
	throws ServerNotExistException, NodeUnreachableException, DeploymentException;

    /**
     *
     * Enable or disable a server. A disabled server can't be started
     * on demand or administratively. The enable state of the server
     * is not persistent: if the node is shutdown and restarted, the
     * server will be enabled by default.
     *
     * @param id The server id.
     *
     * @param enabled True to enable the server, false to disable it.
     *
     * @throws ServerNotExistException Raised if the server doesn't exist.
     *
     * @throws NodeUnreachableException Raised if the node could not
     * be reached.
     *
     * @throws DeploymentException Raised if the server couldn't be
     * deployed on the node.
     *
     **/
    ["ami"] idempotent void enableServer(string id, bool enabled)
	throws ServerNotExistException, NodeUnreachableException, DeploymentException;

    /**
     *
     * Check if the server is enabled or disabled.
     *
     * @param id The server id.
     *
     * @throws ServerNotExistException Raised if the server doesn't
     * exist.
     *
     * @throws NodeUnreachableException Raised if the node could not
     * be reached.
     * 
     * @throws DeploymentException Raised if the server couldn't be
     * deployed on the node.
     *
     **/
    nonmutating bool isServerEnabled(string id)
	throws ServerNotExistException, NodeUnreachableException, DeploymentException;

    /**
     *
     * Start a server and wait for its activation.
     *
     * @param id The server id.
     *
     * @return True if the server was successfully started, false
     * otherwise.
     *
     * @throws ServerNotExistException Raised if the server doesn't
     * exist.
     *
     * @throws ServerStartException Raised if the server couldn't be
     * started.
     *
     * @throws NodeUnreachableException Raised if the node could not
     * be reached.
     *
     * @throws DeploymentException Raised if the server couldn't be
     * deployed on the node.
     *
     **/
    ["ami"] void startServer(string id)
	throws ServerNotExistException, ServerStartException, NodeUnreachableException, DeploymentException;

    /**
     *
     * Stop a server.
     *
     * @param id The server id.
     *
     * @throws ServerNotExistException Raised if the server doesn't
     * exist.
     *
     * @throws ServerStopException Raised if the server couldn't be
     * stopped.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     * @throws DeploymentException Raised if the server couldn't be
     * deployed on the node.
     *
     **/
    ["ami"] void stopServer(string id)
	throws ServerNotExistException, ServerStopException, NodeUnreachableException, DeploymentException;

    /**
     *
     * Patch a server.
     *
     * @param id The server id.
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
     * @throws DeploymentException Raised if the server couldn't be
     * deployed on the node.
     *
     * @throws PatchException Raised if the patch failed.
     *
     **/
    ["ami", "amd"] void patchServer(string id, bool shutdown)
	throws ServerNotExistException, NodeUnreachableException, DeploymentException, PatchException;

    /**
     *
     * Send signal to a server.
     *
     * @param id The server id.
     *
     * @param signal The signal, for example SIGTERM or 15.
     *
     * @throws ServerNotExistException Raised if the server doesn't
     * exist.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     * @throws DeploymentException Raised if the server couldn't be
     * deployed on the node.
     *
     * @throws BadSignalException Raised if the signal is not recognized 
     * by the target server.
     *
     **/
    void sendSignal(string id, string signal)
	throws ServerNotExistException, NodeUnreachableException, DeploymentException, BadSignalException;

    /**
     *
     * Write message on server stdout or stderr.
     *
     * @param id The server id.
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
     * @throws DeploymentException Raised if the server couldn't be
     * deployed on the node.
     *
     **/
    void writeMessage(string id, string message, int fd)
	throws ServerNotExistException, NodeUnreachableException, DeploymentException;

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
     * Get the adapter information for the replica group or adapter
     * with the given id.
     *
     * @param adapterId The adapter id.
     *
     * @return A sequence of adapter information structures. If the
     * given id refers to an adapter, this sequence will contain only
     * one element. If the given id refers to a replica group, the
     * sequence will contain the adapter information of each member of
     * the replica group.
     *
     * @throws AdapterNotExistException Raised if the adapter or
     * replica group doesn't exist.
     *
     **/
    nonmutating AdapterInfoSeq getAdapterInfo(string id)
	throws AdapterNotExistException;

    /**
     *
     * Remove the adapter with the given id.
     *
     * @throws AdapterNotExistException Raised if the adapter doesn't
     * exist.
     *
     **/
    ["ami"] void removeAdapter(string adapterId)
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
     * @throws DeploymentException Raised if the object can't be
     * added. This might be raised if the invocation on the proxy to
     * get the object type failed.
     *
     **/
    void addObject(Object* obj)
	throws ObjectExistsException, DeploymentException;

    /**
     *
     * Update an object in the object registry. Only objects added
     * with this interface can be updated with this operation. Objects
     * added with deployment descriptors should be updated with the
     * deployment mechanism.
     *
     * @param obj The object to be updated to the registry.
     *
     * @throws ObjectNotRegisteredException Raised if the object isn't
     * registered with the registry.
     *
     * @throws DeploymentException Raised if the object can't be
     * updated. This might happen if the object was added with a
     * deployment descriptor.
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
	throws ObjectExistsException;

    /**
     *
     * Remove an object from the object registry. Only objects added
     * with this interface can be removed with this operation. Objects
     * added with deployment descriptors should be removed with the
     * deployment mechanism.
     *
     * @param id The identity of the object to be removed from the
     * registry.
     *
     * @throws ObjectNotRegisteredException Raised if the object isn't
     * registered with the registry.
     *
     * @throws DeploymentException Raised if the object can't be
     * removed. This might happen if the object was added with a
     * deployment descriptor.
     *
     **/
    ["ami"] void removeObject(Ice::Identity id) 
	throws ObjectNotRegisteredException, DeploymentException;

    /**
     *
     * Get the object info for the object with the given identity.
     *
     * @param id The identity of the object.
     *
     * @return The object info.
     *
     * @throws ObjectNotRegisteredException Raised if the object isn't
     * registered with the registry.
     *
     **/
    nonmutating ObjectInfo getObjectInfo(Ice::Identity id)
	throws ObjectNotRegisteredException;

    /**
     *
     * Get the object info of all the registered objects with the
     * given type.
     *
     * @param type The type of the object.
     *
     * @return The object infos.
     *
     **/
    nonmutating ObjectInfoSeq getObjectInfosByType(string type);

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
    ["ami"] nonmutating LoadInfo getNodeLoad(string name)
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
    ["ami"] idempotent void shutdownNode(string name)
	throws NodeNotExistException, NodeUnreachableException;

    /**
     *
     * Get the hostname of this node.
     *
     * @param name The node name.
     *
     * @return The node hostname.
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

interface RegistryObserver;
interface NodeObserver;

/**
 *
 * An admin session object used by administrative clients to view,
 * update and receive observer updates from the &IceGrid;
 * registry. Admin sessions are created either with the [Registry]
 * object or the registry admin [Glacier2::SessionManager] object.
 * 
 * @see Registry
 * @see Glacier2::SessionManager
 *
 **/
interface AdminSession extends Glacier2::Session
{
    /**
     *
     * Keep the session alive. Clients should call this operation
     * regularly to prevent the server from reaping the session.
     *
     * @see Registry::getSessionTimeout
     *
     **/
    idempotent void keepAlive();

    /**
     *
     * Get the admin interface. The admin object returned by this
     * operation can only be accessed by the session.
     *
     * @return The admin interface proxy.
     *
     **/
    nonmutating Admin* getAdmin();

    /**
     *
     * Set the proxies of the observer objects that will receive
     * notifications from the servers when the state of the registry
     * or nodes changes.
     *
     * @param registryObs The registry observer.
     *
     * @param nodeObs The node observer.
     *
     **/
    idempotent void setObservers(RegistryObserver* registryObs, NodeObserver* nodeObs);

    /**
     *
     * Set the identities of the observer objects that will receive
     * notifications from the servers when the state of the registry
     * or nodes changes. This operation should be used by clients that
     * are using a bidirectional connection to communicate with the
     * session.
     *
     * @param registryObs The registry observer identity.
     *
     * @param nodeObs The node observer identity.
     *
     **/
    idempotent void setObserversByIdentity(Ice::Identity registryObs, Ice::Identity nodeObs);

    /**
     *
     * Acquires an exclusive lock to start updating the registry applications.
     *
     * @return The current serial.
     * 
     * @throws AccessDeniedException Raised if the exclusive lock can't be
     * acquired. This might happen if the lock is currently acquired by
     * another session.
     *
     **/
    int startUpdate()
	throws AccessDeniedException;
    
    /**
     *
     * Finish updating the registry and release the exclusive lock.
     *
     * @throws AccessDeniedException Raised if the session doesn't hold the
     * exclusive lock.
     *
     **/
    void finishUpdate()
	throws AccessDeniedException;
};

};

#endif
