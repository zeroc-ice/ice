// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h", "objc:header-dir:objc", "js:ice-build"]]
[["cpp:include:IceGrid/Config.h"]]

#include <Ice/Identity.ice>
#include <Ice/BuiltinSequences.ice>
#include <Ice/Properties.ice>
#include <Ice/SliceChecksumDict.ice>
#include <Glacier2/Session.ice>
#include <IceGrid/Exception.ice>
#include <IceGrid/Descriptor.ice>

["objc:prefix:ICEGRID"]
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
     * state when the registered server object adapters are activated
     * or to the activation timed out state if the activation timeout
     * expires.
     *
     **/
    Activating,

    /**
     *
     * The activation timed out state indicates that the server
     * activation timed out.
     *
     **/
    ActivationTimedOut,

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
["cpp:comparable"]
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
 * Information about an adapter registered with the IceGrid registry.
 *
 **/
["cpp:comparable"]
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
 * Information about a server managed by an IceGrid node.
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
     * The application uuid.
     *
     **/
    string uuid;

    /**
     *
     * The application revision.
     *
     **/
    int revision;

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

    /**
     *
     * The id of the session which allocated the server.
     *
     **/
    string sessionId;
};

/**
 *
 * Information about an IceGrid node.
 *
 **/
struct NodeInfo
{
    /**
     *
     * The name of the node.
     *
     **/
    string name;

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
     * The number of processor threads on the node.
     * For example, nProcessors is 8 on a computer with a single quad-core
     * processor and two HT threads per core.
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
 * Information about an IceGrid registry replica.
 *
 **/
struct RegistryInfo
{
    /**
     *
     * The name of the registry.
     *
     **/
    string name;

    /**
     *
     * The network name of the host running this registry (as defined in
     * uname()).
     *
     **/
    string hostname;
};

/**
 *
 * A sequence of {@link RegistryInfo} structures.
 *
 **/
sequence<RegistryInfo> RegistryInfoSeq;

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
 * Information about an IceGrid application.
 *
 **/
struct ApplicationInfo
{
    /** Unique application identifier. */
    string uuid;

    /** The creation time. */
    long createTime;

    /** The user who created the application. */
    string createUser;

    /** The update time. */
    long updateTime;

    /** The user who updated the application. */
    string updateUser;

    /** The application revision number. */
    int revision;

    /** The application descriptor */
    ApplicationDescriptor descriptor;
};

/**
 *
 * A sequence of {@link ApplicationInfo} structures.
 *
 **/
["java:type:java.util.LinkedList<ApplicationInfo>"] sequence<ApplicationInfo> ApplicationInfoSeq;

/**
 *
 * Information about updates to an IceGrid application.
 *
 **/
struct ApplicationUpdateInfo
{
    /** The update time. */
    long updateTime;

    /** The user who updated the application. */
    string updateUser;

    /** The application revision number. */
    int revision;

    /** The update descriptor. */
    ApplicationUpdateDescriptor descriptor;
};

/**
 *
 * The IceGrid administrative interface. </p>
 * <p class="Warning">Allowing access to this interface
 * is a security risk! Please see the IceGrid documentation
 * for further information.
 *
 **/
interface Admin
{
    /**
     *
     * Add an application to IceGrid.
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
     * Synchronize a deployed application with the given application
     * descriptor. This operation will replace the current descriptor
     * with this new descriptor only if no server restarts are
     * necessary for the update of the application. If some servers
     * need to be restarted, the synchronization is rejected with a
     * DeploymentException.
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
    void syncApplicationWithoutRestart(ApplicationDescriptor descriptor)
        throws AccessDeniedException, DeploymentException, ApplicationNotExistException;

    /**
     *
     * Update a deployed application with the given update application
     * descriptor only if no server restarts are necessary for the
     * update of the application. If some servers need to be
     * restarted, the synchronization is rejected with a
     * DeploymentException.
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
    void updateApplicationWithoutRestart(ApplicationUpdateDescriptor descriptor)
        throws AccessDeniedException, DeploymentException, ApplicationNotExistException;

    /**
     *
     * Remove an application from IceGrid.
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
        throws AccessDeniedException, DeploymentException, ApplicationNotExistException;

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
     * patch will be shut down if necessary.
     *
     * @throws ApplicationNotExistException Raised if the application
     * doesn't exist.
     *
     * @throws PatchException Raised if the patch failed.
     *
     **/
    ["amd"] void patchApplication(string name, bool shutdown)
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
    ["nonmutating", "cpp:const"] idempotent ApplicationInfo getApplicationInfo(string name)
        throws ApplicationNotExistException;

    /**
     *
     * Get the default application descriptor.
     *
     * @throws DeploymentException Raised if the default application
     * descriptor can't be accessed or is invalid.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent ApplicationDescriptor getDefaultApplicationDescriptor()
        throws DeploymentException;

    /**
     *
     * Get all the IceGrid applications currently registered.
     *
     * @return The application names.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent Ice::StringSeq getAllApplicationNames();

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
    ["nonmutating", "cpp:const"] idempotent ServerInfo getServerInfo(string id)
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
    ["nonmutating", "cpp:const"] idempotent ServerState getServerState(string id)
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
    ["nonmutating", "cpp:const"] idempotent int getServerPid(string id)
        throws ServerNotExistException, NodeUnreachableException, DeploymentException;

    /**
     *
     * Get the category for server admin objects. You can manufacture a server admin
     * proxy from the admin proxy by changing its identity: use the server ID as name
     * and the returned category as category.
     *
     * @return The category for server admin objects.
     *
     **/
    ["cpp:const"]
    idempotent string getServerAdminCategory();

    /**
     *
     * Get a proxy to the server's admin object.
     *
     * @param id The server id.
     *
     * @return A proxy to the server's admin object
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
    ["cpp:const"]
    idempotent Object* getServerAdmin(string id)
        throws ServerNotExistException, NodeUnreachableException, DeploymentException;


    /**
     *
     * Enable or disable a server. A disabled server can't be started
     * on demand or administratively. The enable state of the server
     * is not persistent: if the node is shut down and restarted, the
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
    idempotent void enableServer(string id, bool enabled)
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
    ["nonmutating", "cpp:const"] idempotent bool isServerEnabled(string id)
        throws ServerNotExistException, NodeUnreachableException, DeploymentException;

    /**
     *
     * Start a server and wait for its activation.
     *
     * @param id The server id.
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
    ["amd"] void startServer(string id)
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
    ["amd"] void stopServer(string id)
        throws ServerNotExistException, ServerStopException, NodeUnreachableException, DeploymentException;

    /**
     *
     * Patch a server.
     *
     * @param id The server id.
     *
     * @param shutdown If true, servers depending on the data to patch
     * will be shut down if necessary.
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
    ["amd"] void patchServer(string id, bool shutdown)
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
     * Get all the server ids registered with IceGrid.
     *
     * @return The server ids.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent Ice::StringSeq getAllServerIds();

    /**
     *
     * Get the adapter information for the replica group or adapter
     * with the given id.
     *
     * @param id The adapter id.
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
    ["nonmutating", "cpp:const"] idempotent AdapterInfoSeq getAdapterInfo(string id)
        throws AdapterNotExistException;

    /**
     *
     * Remove the adapter with the given id.
     *
     * @param id The adapter id.
     * @throws AdapterNotExistException Raised if the adapter doesn't
     * exist.
     *
     **/
    void removeAdapter(string id)
        throws AdapterNotExistException, DeploymentException;

    /**
     *
     * Get all the adapter ids registered with IceGrid.
     *
     * @return The adapter ids.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent Ice::StringSeq getAllAdapterIds();

    /**
     *
     * Add an object to the object registry. IceGrid will get the
     * object type by calling <tt>ice_id</tt> on the given proxy. The object
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
        throws ObjectExistsException, DeploymentException;

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
     * @throws ObjectNotRegisteredException Raised if the object isn't
     * registered with the registry.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent ObjectInfo getObjectInfo(Ice::Identity id)
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
    ["nonmutating", "cpp:const"] idempotent ObjectInfoSeq getObjectInfosByType(string type);

    /**
     *
     * Get the object info of all the registered objects whose stringified
     * identities match the given expression.
     *
     * @param expr The expression to match against the stringified
     * identities of registered objects. The expression may contain
     * a trailing wildcard (<tt>*</tt>) character.
     *
     * @return All the object infos with a stringified identity
     * matching the given expression.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent ObjectInfoSeq getAllObjectInfos(string expr);

    /**
     *
     * Ping an IceGrid node to see if it is active.
     *
     * @param name The node name.
     *
     * @return true if the node ping succeeded, false otherwise.
     *
     * @throws NodeNotExistException Raised if the node doesn't exist.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent bool pingNode(string name)
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
    ["nonmutating", "cpp:const"] idempotent LoadInfo getNodeLoad(string name)
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
    ["nonmutating", "cpp:const"] idempotent NodeInfo getNodeInfo(string name)
        throws NodeNotExistException, NodeUnreachableException;


    /**
     *
     * Get a proxy to the IceGrid node's admin object.
     *
     * @param name The IceGrid node name
     *
     * @return A proxy to the IceGrid node's admin object
     *
     * @throws NodeNotExistException Raised if the node doesn't exist.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     **/
    ["cpp:const"] idempotent Object* getNodeAdmin(string name)
         throws NodeNotExistException, NodeUnreachableException;

    /**
     *
     * Get the number of physical processor sockets for the machine
     * running the node with the given name.
     *
     * Note that this method will return 1 on operating systems where
     * this can't be automatically determined and where the
     * IceGrid.Node.ProcessorSocketCount property for the node is not
     * set.
     *
     * @param name The node name.
     *
     * @return The number of processor sockets or 1 if the number of
     * sockets can't determined.
     *
     * @throws NodeNotExistException Raised if the node doesn't exist.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent int getNodeProcessorSocketCount(string name)
        throws NodeNotExistException, NodeUnreachableException;

    /**
     *
     * Shutdown an IceGrid node.
     *
     * @param name The node name.
     *
     * @throws NodeNotExistException Raised if the node doesn't exist.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     **/
    void shutdownNode(string name)
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
    ["nonmutating", "cpp:const"] idempotent string getNodeHostname(string name)
        throws NodeNotExistException, NodeUnreachableException;

    /**
     *
     * Get all the IceGrid nodes currently registered.
     *
     * @return The node names.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent Ice::StringSeq getAllNodeNames();

    /**
     *
     * Ping an IceGrid registry to see if it is active.
     *
     * @param name The registry name.
     *
     * @return true if the registry ping succeeded, false otherwise.
     *
     * @throws RegistryNotExistException Raised if the registry doesn't exist.
     *
     **/
    ["cpp:const"] idempotent bool pingRegistry(string name)
        throws RegistryNotExistException;

    /**
     *
     * Get the registry information for the registry with the given name.
     *
     * @param name The registry name.
     *
     * @return The registry information.
     *
     * @throws RegistryNotExistException Raised if the registry doesn't exist.
     *
     * @throws RegistryUnreachableException Raised if the registry could not be
     * reached.
     *
     **/
    ["cpp:const"] idempotent RegistryInfo getRegistryInfo(string name)
        throws RegistryNotExistException, RegistryUnreachableException;

    /**
     *
     * Get a proxy to the IceGrid registry's admin object.
     *
     * @param name The registry name
     *
     * @return A proxy to the IceGrid registry's admin object
     *
     * @throws RegistryNotExistException Raised if the registry doesn't exist.
     *
     **/
    ["cpp:const"] idempotent Object* getRegistryAdmin(string name)
        throws RegistryNotExistException;

    /**
     *
     * Shutdown an IceGrid registry.
     *
     * @param name The registry name.
     *
     * @throws RegistryNotExistException Raised if the registry doesn't exist.
     *
     * @throws RegistryUnreachableException Raised if the registry could not be
     * reached.
     *
     **/
    idempotent void shutdownRegistry(string name)
        throws RegistryNotExistException, RegistryUnreachableException;

    /**
     *
     * Get all the IceGrid registries currently registered.
     *
     * @return The registry names.
     *
     **/
    ["cpp:const"] idempotent Ice::StringSeq getAllRegistryNames();

    /**
     *
     * Shut down the IceGrid registry.
     *
     **/
    void shutdown();

    /**
     *
     * Returns the checksums for the IceGrid Slice definitions.
     *
     * @return A dictionary mapping Slice type ids to their checksums.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent Ice::SliceChecksumDict getSliceChecksums();
};

/**
 *
 * This interface provides access to IceGrid log file contents.
 *
 **/
interface FileIterator
{
    /**
     *
     * Read lines from the log file.
     *
     * @param size Specifies the maximum number of bytes to be
     * received. The server will ensure that the returned message
     * doesn't exceed the given size.
     *
     * @param lines The lines read from the file. If there was nothing to
     * read from the file since the last call to read, an empty
     * sequence is returned. The last line of the sequence is always
     * incomplete (and therefore no '\n' should be added when writing
     * the last line to the to the output device).
     *
     * @return True if EOF is encountered.
     *
     * @throws FileNotAvailableException Raised if there was a problem
     * to read lines from the file.
     *
     **/
    bool read(int size, out Ice::StringSeq lines)
        throws FileNotAvailableException;

    /**
     *
     * Destroy the iterator.
     *
     **/
    void destroy();
};

interface RegistryObserver;
interface NodeObserver;
interface ApplicationObserver;
interface AdapterObserver;
interface ObjectObserver;

/**
 *
 * Used by administrative clients to view,
 * update, and receive observer updates from the IceGrid
 * registry. Admin sessions are created either with the {@link Registry}
 * object or the registry admin {@link Glacier2.SessionManager} object.
 *
 * @see Registry
 * @see Glacier2.SessionManager
 *
 **/
interface AdminSession extends Glacier2::Session
{
    /**
     *
     * Keep the session alive. Clients should call this operation
     * regularly to prevent the server from reaping the session.
     *
     * @see Registry#getSessionTimeout
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
    ["nonmutating", "cpp:const"] idempotent Admin* getAdmin();


    /**
     *
     * Get a "template" proxy for admin callback objects.
     * An Admin client uses this proxy to set the category of its callback
     * objects, and the published endpoints of the object adapter hosting
     * the admin callback objects.
     *
     * @return A template proxy. The returned proxy is null when the Admin
     * session was established using Glacier2.
     *
     **/
    ["cpp:const"] idempotent Object* getAdminCallbackTemplate();


    /**
     *
     * Set the observer proxies that receive
     * notifications when the state of the registry
     * or nodes changes.
     *
     * @param registryObs The registry observer.
     *
     * @param nodeObs The node observer.
     *
     * @param appObs The application observer.
     *
     * @param adptObs The adapter observer.
     *
     * @param objObs The object observer.
     *
     * @throws ObserverAlreadyRegisteredException Raised if an
     * observer is already registered with this registry.
     *
     **/
    idempotent void setObservers(RegistryObserver* registryObs, NodeObserver* nodeObs, ApplicationObserver* appObs,
                                 AdapterObserver* adptObs, ObjectObserver* objObs)
        throws ObserverAlreadyRegisteredException;

    /**
     *
     * Set the observer identities that receive
     * notifications the state of the registry
     * or nodes changes. This operation should be used by clients that
     * are using a bidirectional connection to communicate with the
     * session.
     *
     * @param registryObs The registry observer identity.
     *
     * @param nodeObs The node observer identity.
     *
     * @param appObs The application observer.
     *
     * @param adptObs The adapter observer.
     *
     * @param objObs The object observer.
     *
     * @throws ObserverAlreadyRegisteredException Raised if an
     * observer is already registered with this registry.
     *
     **/
    idempotent void setObserversByIdentity(Ice::Identity registryObs, Ice::Identity nodeObs, Ice::Identity appObs,
                                           Ice::Identity adptObs, Ice::Identity objObs)
        throws ObserverAlreadyRegisteredException;

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

    /**
     *
     * Get the name of the registry replica hosting this session.
     *
     * @return The replica name of the registry.
     *
     **/
    ["cpp:const"] idempotent string getReplicaName();

    /**
     *
     * Open the given server log file for reading. The file can be
     * read with the returned file iterator.
     *
     * @param id The server id.
     *
     * @param path The path of the log file. A log file can be opened
     * only if it's declared in the server or service deployment
     * descriptor.
     *
     * @param count Specifies where to start reading the file. If
     * negative, the file is read from the begining. If 0 or positive,
     * the file is read from the last <tt>count</tt> lines.
     *
     * @return An iterator to read the file.
     *
     * @throws FileNotAvailableException Raised if the file can't be
     * read.
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
    FileIterator* openServerLog(string id, string path, int count)
        throws FileNotAvailableException, ServerNotExistException, NodeUnreachableException, DeploymentException;

    /**
     *
     * Open the given server stderr file for reading. The file can be
     * read with the returned file iterator.
     *
     * @param id The server id.
     *
     * @param count Specifies where to start reading the file. If
     * negative, the file is read from the begining. If 0 or positive,
     * the file is read from the last <tt>count</tt> lines.
     *
     * @return An iterator to read the file.
     *
     * @throws FileNotAvailableException Raised if the file can't be
     * read.
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
    FileIterator* openServerStdErr(string id, int count)
        throws FileNotAvailableException, ServerNotExistException, NodeUnreachableException, DeploymentException;

    /**
     *
     * Open the given server stdout file for reading. The file can be
     * read with the returned file iterator.
     *
     * @param id The server id.
     *
     * @param count Specifies where to start reading the file. If
     * negative, the file is read from the begining. If 0 or positive,
     * the file is read from the last <tt>count</tt> lines.
     *
     * @return An iterator to read the file.
     *
     * @throws FileNotAvailableException Raised if the file can't be
     * read.
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
    FileIterator* openServerStdOut(string id, int count)
        throws FileNotAvailableException, ServerNotExistException, NodeUnreachableException, DeploymentException;

    /**
     *
     * Open the given node stderr file for reading. The file can be
     * read with the returned file iterator.
     *
     * @param name The node name.
     *
     * @param count Specifies where to start reading the file. If
     * negative, the file is read from the begining. If 0 or positive,
     * the file is read from the last <tt>count</tt> lines.
     *
     * @return An iterator to read the file.
     *
     * @throws FileNotAvailableException Raised if the file can't be
     * read.
     *
     * @throws NodeNotExistException Raised if the node doesn't exist.
     *
     * @throws NodeUnreachableException Raised if the node could not
     * be reached.
     *
     **/
    FileIterator* openNodeStdErr(string name, int count)
        throws FileNotAvailableException, NodeNotExistException, NodeUnreachableException;

    /**
     *
     * Open the given node stdout file for reading. The file can be
     * read with the returned file iterator.
     *
     * @param name The node name.
     *
     * @param count Specifies where to start reading the file. If
     * negative, the file is read from the begining. If 0 or positive,
     * the file is read from the last <tt>count</tt> lines.
     *
     * @return An iterator to read the file.
     *
     * @throws FileNotAvailableException Raised if the file can't be
     * read.
     *
     * @throws NodeNotExistException Raised if the node doesn't exist.
     *
     * @throws NodeUnreachableException Raised if the node could not
     * be reached.
     *
     **/
    FileIterator* openNodeStdOut(string name, int count)
        throws FileNotAvailableException, NodeNotExistException, NodeUnreachableException;

    /**
     *
     * Open the given registry stderr file for reading. The file can be
     * read with the returned file iterator.
     *
     * @param name The registry name.
     *
     * @param count Specifies where to start reading the file. If
     * negative, the file is read from the begining. If 0 or positive,
     * the file is read from the last <tt>count</tt> lines.
     *
     * @return An iterator to read the file.
     *
     * @throws FileNotAvailableException Raised if the file can't be
     * read.
     *
     * @throws RegistryNotExistException Raised if the registry
     * doesn't exist.
     *
     * @throws RegistryUnreachableException Raised if the registry
     * could not be reached.
     *
     **/
    FileIterator* openRegistryStdErr(string name, int count)
        throws FileNotAvailableException, RegistryNotExistException, RegistryUnreachableException;

    /**
     *
     * Open the given registry stdout file for reading. The file can be
     * read with the returned file iterator.
     *
     * @param name The registry name.
     *
     * @param count Specifies where to start reading the file. If
     * negative, the file is read from the begining. If 0 or positive,
     * the file is read from the last <tt>count</tt> lines.
     *
     * @return An iterator to read the file.
     *
     * @throws FileNotAvailableException Raised if the file can't be
     * read.
     *
     * @throws RegistryNotExistException Raised if the registry
     * doesn't exist.
     *
     * @throws RegistryUnreachableException Raised if the registry
     * could not be reached.
     *
     **/
    FileIterator * openRegistryStdOut(string name, int count)
        throws FileNotAvailableException, RegistryNotExistException, RegistryUnreachableException;

};

};

