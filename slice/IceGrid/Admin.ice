// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICEGRID_API"]]
[["cpp:doxygen:include:IceGrid/IceGrid.h"]]
[["cpp:header-ext:h"]]

[["cpp:include:IceGrid/Config.h"]]

[["js:module:@zeroc/ice"]]
[["python:pkgdir:IceGrid"]]

#include "Descriptor.ice"
#include "Exception.ice"
#include "Glacier2/Session.ice"
#include "Ice/BuiltinSequences.ice"
#include "Ice/Identity.ice"

["java:identifier:com.zeroc.IceGrid"]
module IceGrid
{
    interface Registry; // So that doc-comments can link to `IceGrid::Registry`.

    /// Represents the state of a server.
    enum ServerState
    {
        /// The server is not running.
        ["swift:identifier:inactive"]
        Inactive,

        /// The server is being activated and will change to the active state when the registered server object adapters
        /// are activated or to the activation timed out state if the activation timeout expires.
        ["swift:identifier:activating"]
        Activating,

        /// The server activation timed out.
        ["swift:identifier:activationTimedOut"]
        ActivationTimedOut,

        /// The server is running.
        ["swift:identifier:active"]
        Active,

        /// The server is being deactivated.
        ["swift:identifier:deactivating"]
        Deactivating,

        /// The server is being destroyed.
        ["swift:identifier:destroying"]
        Destroying,

        /// The server is destroyed.
        ["swift:identifier:destroyed"]
        Destroyed
    }

    /// A dictionary of string to proxies.
    dictionary<string, Object*> StringObjectProxyDict;

    /// Information about an Ice well-known object.
    struct ObjectInfo
    {
        /// The proxy of the object.
        Object* proxy;

        /// The type of the object.
        string type;
    }

    /// A sequence of ObjectInfo.
    sequence<ObjectInfo> ObjectInfoSeq;

    /// Information about an adapter registered with the IceGrid registry.
    struct AdapterInfo
    {
        /// The ID of the adapter.
        string id;

        /// A dummy direct proxy that contains the adapter endpoints.
        Object* proxy;

        /// The replica group ID of the object adapter, or empty if the adapter doesn't belong to a replica group.
        string replicaGroupId;
    }

    /// A sequence of AdapterInfo.
    sequence<AdapterInfo> AdapterInfoSeq;

    /// Information about a server managed by an IceGrid node.
    struct ServerInfo
    {
        /// The application to which this server belongs.
        string application;

        /// The application UUID.
        string uuid;

        /// The application revision.
        int revision;

        /// The IceGrid node where this server is deployed.
        string node;

        /// The server descriptor.
        ServerDescriptor descriptor;

        /// The ID of the session which allocated the server.
        string sessionId;
    }

    /// Information about an IceGrid node.
    struct NodeInfo
    {
        /// The name of the node.
        string name;

        /// The operating system name.
        string os;

        /// The network name of the host running this node.
        string hostname;

        /// The operation system release level.
        string release;

        /// The operation system version.
        string version;

        /// The machine hardware type.
        string machine;

        /// The number of processor threads on the node. For example, nProcessors is 8 on a computer with a single
        /// quad-core processor and two threads per core.
        int nProcessors;

        /// The path to the node data directory.
        string dataDir;
    }

    /// Information about an IceGrid registry replica.
    struct RegistryInfo
    {
        /// The name of the registry.
        string name;

        /// The network name of the host running this registry.
        string hostname;
    }

    /// A sequence of {@link RegistryInfo}.
    sequence<RegistryInfo> RegistryInfoSeq;

    /// Information about the load of a node.
    struct LoadInfo
    {
        /// The load average over the past minute.
        float avg1;

        /// The load average over the past 5 minutes.
        float avg5;

        /// The load average over the past 15 minutes.
        float avg15;
    }

    /// Information about an IceGrid application.
    struct ApplicationInfo
    {
        /// Unique application identifier.
        string uuid;

        /// The creation time.
        long createTime;

        /// The user who created the application.
        string createUser;

        /// The last update time.
        long updateTime;

        /// The user who updated the application.
        string updateUser;

        /// The application revision number.
        int revision;

        /// The application descriptor.
        ApplicationDescriptor descriptor;
    }

    /// A sequence of {@link ApplicationInfo}.
    ["java:type:java.util.LinkedList<ApplicationInfo>"]
    sequence<ApplicationInfo> ApplicationInfoSeq;

    /// Information about updates to an IceGrid application.
    struct ApplicationUpdateInfo
    {
        /// The update time.
        long updateTime;

        /// The user who updated the application.
        string updateUser;

        /// The application revision number.
        int revision;

        /// The update descriptor.
        ApplicationUpdateDescriptor descriptor;
    }

    /// Provides administrative access to an IceGrid deployment.
    interface Admin
    {
        /// Adds an application to IceGrid.
        /// @param descriptor The application descriptor.
        /// @throws AccessDeniedException Thrown when the session doesn't hold the exclusive lock or when another
        /// session is holding the lock.
        /// @throws DeploymentException Thrown when the application deployment failed.
        void addApplication(ApplicationDescriptor descriptor)
            throws AccessDeniedException, DeploymentException;

        /// Synchronizes a deployed application. This operation replaces the current descriptor with a new descriptor.
        /// @param descriptor The new application descriptor.
        /// @throws AccessDeniedException Thrown when the session doesn't hold the exclusive lock or when another
        /// session is holding the lock.
        /// @throws DeploymentException Thrown when the application deployment failed.
        /// @throws ApplicationNotExistException Thrown when the application doesn't exist.
        void syncApplication(ApplicationDescriptor descriptor)
            throws AccessDeniedException, DeploymentException, ApplicationNotExistException;

        /// Updates a deployed application.
        /// @param descriptor The update descriptor.
        /// @throws AccessDeniedException Thrown when the session doesn't hold the exclusive lock or when another
        /// session is holding the lock.
        /// @throws DeploymentException Thrown when the application deployment failed.
        /// @throws ApplicationNotExistException Thrown when the application doesn't exist.
        void updateApplication(ApplicationUpdateDescriptor descriptor)
            throws AccessDeniedException, DeploymentException, ApplicationNotExistException;

        /// Synchronizes a deployed application. This operation replaces the current descriptor with a new descriptor
        /// only if no server restarts are necessary for the update of the application. If some servers need to be
        /// restarted, the synchronization is rejected with a DeploymentException.
        /// @param descriptor The application descriptor.
        /// @throws AccessDeniedException Thrown when the session doesn't hold the exclusive lock or when another
        /// session is holding the lock.
        /// @throws DeploymentException Thrown when the application deployment failed.
        /// @throws ApplicationNotExistException Thrown when the application doesn't exist.
        void syncApplicationWithoutRestart(ApplicationDescriptor descriptor)
            throws AccessDeniedException, DeploymentException, ApplicationNotExistException;

        /// Updates a deployed application. This operation succeeds only when no server restarts are necessary for the
        /// update of the application. If some servers need to be restarted, the synchronization is rejected with a
        /// DeploymentException.
        /// @param descriptor The update descriptor.
        /// @throws AccessDeniedException Thrown when the session doesn't hold the exclusive lock or when another
        /// session is holding the lock.
        /// @throws DeploymentException Thrown when the application deployment failed.
        /// @throws ApplicationNotExistException Thrown when the application doesn't exist.
        void updateApplicationWithoutRestart(ApplicationUpdateDescriptor descriptor)
            throws AccessDeniedException, DeploymentException, ApplicationNotExistException;

        /// Removes an application from IceGrid.
        /// @param name The application name.
        /// @throws AccessDeniedException Thrown when the session doesn't hold the exclusive lock or when another
        /// session is holding the lock.
        /// @throws DeploymentException Thrown when the application deployment failed.
        /// @throws ApplicationNotExistException Thrown when the application doesn't exist.
        void removeApplication(string name)
            throws AccessDeniedException, DeploymentException, ApplicationNotExistException;

        /// Instantiates a server template.
        /// @param application The application name.
        /// @param node The name of the node where the server will be deployed.
        /// @param desc The descriptor of the server instance to deploy.
        /// @throws AccessDeniedException Thrown when the session doesn't hold the exclusive lock or when another
        /// session is holding the lock.
        /// @throws DeploymentException Thrown when the application deployment failed.
        /// @throws ApplicationNotExistException Thrown when the application doesn't exist.
        void instantiateServer(string application, string node, ServerInstanceDescriptor desc)
            throws AccessDeniedException, ApplicationNotExistException, DeploymentException;

        /// Gets an application descriptor.
        /// @param name The application name.
        /// @return The application descriptor.
        /// @throws ApplicationNotExistException Thrown when the application doesn't exist.
        ["cpp:const"]
        idempotent ApplicationInfo getApplicationInfo(string name)
            throws ApplicationNotExistException;

        /// Gets the default application descriptor.
        /// @return The default application descriptor.
        /// @throws DeploymentException Thrown when the default application descriptor is invalid or unreachable.
        ["cpp:const"]
        idempotent ApplicationDescriptor getDefaultApplicationDescriptor()
            throws DeploymentException;

        /// Gets all the IceGrid applications currently registered.
        /// @return The application names.
        ["cpp:const"]
        idempotent Ice::StringSeq getAllApplicationNames();

        /// Gets information about a server.
        /// @param id The server ID.
        /// @throws ServerNotExistException Thrown when the server doesn't exist.
        /// @return The server information.
        ["cpp:const"]
        idempotent ServerInfo getServerInfo(string id)
            throws ServerNotExistException;

        /// Gets the state of a server.
        /// @param id The server ID.
        /// @return The server state.
        /// @throws ServerNotExistException Thrown when the server doesn't exist.
        /// @throws NodeUnreachableException Thrown when the node is unreachable.
        /// @throws DeploymentException Thrown when the deployment of the server failed.
        ["cpp:const"]
        idempotent ServerState getServerState(string id)
            throws ServerNotExistException, NodeUnreachableException, DeploymentException;

        /// Gets the system process ID of a server. The process ID is operating system dependent.
        /// @param id The server ID.
        /// @return The process ID.
        /// @throws ServerNotExistException Thrown when the server doesn't exist.
        /// @throws NodeUnreachableException Thrown when the node is unreachable.
        /// @throws DeploymentException Thrown when the deployment of the server failed.
        ["cpp:const"]
        idempotent int getServerPid(string id)
            throws ServerNotExistException, NodeUnreachableException, DeploymentException;

        /// Gets the category for server admin objects. You can manufacture a server admin proxy from the admin proxy by
        /// changing its identity: use the server ID as name and the returned category as category.
        /// @return The category for server admin objects.
        ["cpp:const"]
        idempotent string getServerAdminCategory();

        /// Gets a proxy to the admin object of a server.
        /// @param id The server ID.
        /// @return A proxy to the admin object of the server. This proxy is never null.
        /// @throws ServerNotExistException Thrown when the server doesn't exist.
        /// @throws NodeUnreachableException Thrown when the node is unreachable.
        /// @throws DeploymentException Thrown when the deployment of the server failed.
        ["cpp:const"]
        idempotent Object* getServerAdmin(string id)
            throws ServerNotExistException, NodeUnreachableException, DeploymentException;

        /// Enables or disables a server. A disabled server can't be started on demand or administratively. The enable
        /// state of the server is not persistent: if the node is shut down and restarted, the server will be enabled by
        /// default.
        /// @param id The server ID.
        /// @param enabled `true` to enable the server, `false` to disable it.
        /// @throws ServerNotExistException Thrown when the server doesn't exist.
        /// @throws NodeUnreachableException Thrown when the node is unreachable.
        /// @throws DeploymentException Thrown when the deployment of the server failed.
        idempotent void enableServer(string id, bool enabled)
            throws ServerNotExistException, NodeUnreachableException, DeploymentException;

        /// Checks if the server is enabled or disabled.
        /// @param id The server ID.
        /// @return `true` if the server is enabled, `false` otherwise.
        /// @throws ServerNotExistException Thrown when the server doesn't exist.
        /// @throws NodeUnreachableException Thrown when the node is unreachable.
        /// @throws DeploymentException Thrown when the deployment of the server failed.
        ["cpp:const"]
        idempotent bool isServerEnabled(string id)
            throws ServerNotExistException, NodeUnreachableException, DeploymentException;

        /// Starts a server and waits for its activation.
        /// @param id The server id.
        /// @throws ServerNotExistException Thrown when the server doesn't exist.
        /// @throws ServerStartException Thrown when the server startup failed.
        /// @throws NodeUnreachableException Thrown when the node is unreachable.
        /// @throws DeploymentException Thrown when the deployment of the server failed.
        ["amd"]
        void startServer(string id)
            throws ServerNotExistException, ServerStartException, NodeUnreachableException, DeploymentException;

        /// Stops a server.
        /// @param id The server ID.
        /// @throws ServerNotExistException Thrown when the server doesn't exist.
        /// @throws ServerStopException Thrown when the server stop failed.
        /// @throws NodeUnreachableException Thrown when the node is unreachable.
        /// @throws DeploymentException Thrown when the deployment of the server failed.
        ["amd"]
        void stopServer(string id)
            throws ServerNotExistException, ServerStopException, NodeUnreachableException, DeploymentException;

        /// Sends a signal to a server.
        /// @param id The server ID.
        /// @param signal The signal, for example SIGTERM or 15.
        /// @throws ServerNotExistException Thrown when the server doesn't exist.
        /// @throws NodeUnreachableException Thrown when the node is unreachable.
        /// @throws DeploymentException Thrown when the deployment of the server failed.
        /// @throws BadSignalException Thrown when the signal is not recognized by the target server.
        void sendSignal(string id, string signal)
            throws ServerNotExistException, NodeUnreachableException, DeploymentException, BadSignalException;

        /// Gets the IDs of all the servers registered with IceGrid.
        /// @return The server IDs.
        ["cpp:const"]
        idempotent Ice::StringSeq getAllServerIds();

        /// Gets adapter information for the replica group or adapter with the given ID.
        /// @param id The adapter or replica group ID.
        /// @return A sequence of AdapterInfo. If @p id refers to an adapter, this sequence contains a single element.
        /// If @p id refers to a replica group, this sequence contains adapter information for each member of the
        /// replica group.
        /// @throws AdapterNotExistException Thrown when the adapter or replica group doesn't exist.
        ["cpp:const"]
        idempotent AdapterInfoSeq getAdapterInfo(string id)
            throws AdapterNotExistException;

        /// Removes the adapter with the given ID.
        /// @param id The adapter ID.
        /// @throws AdapterNotExistException Thrown when the adapter doesn't exist.
        /// @throws DeploymentException Thrown when the application deployment failed.
        void removeAdapter(string id)
            throws AdapterNotExistException, DeploymentException;

        /// Gets the IDs of all adapters registered with IceGrid.
        /// @return The adapter IDs.
        ["cpp:const"]
        idempotent Ice::StringSeq getAllAdapterIds();

        /// Adds an object to the object registry. IceGrid gets the object type by calling `ice_id` on @p obj. The
        /// object must be reachable.
        /// @param obj A proxy to the object. This proxy is never null.
        /// @throws ObjectExistsException Thrown when the object is already registered.
        /// @throws DeploymentException Thrown when the object can't be added.
        void addObject(Object* obj)
            throws ObjectExistsException, DeploymentException;

        /// Updates an object in the object registry. Only objects added with this interface can be updated with this
        /// operation. Objects added with deployment descriptors should be updated with the deployment mechanism.
        /// @param obj A proxy to the object. This proxy is never null.
        /// @throws ObjectNotRegisteredException Thrown when the object isn't registered with the registry.
        /// @throws DeploymentException Thrown when the object can't be updated.
        void updateObject(Object* obj)
            throws ObjectNotRegisteredException, DeploymentException;

        /// Adds an object to the object registry and explicitly specifies its type.
        /// @param obj The object to be added to the registry. The proxy is never null.
        /// @param type The type name.
        /// @throws ObjectExistsException Thrown when the object is already registered.
        /// @throws DeploymentException Thrown when the application deployment failed.
        void addObjectWithType(Object* obj, string type)
            throws ObjectExistsException, DeploymentException;

        /// Removes an object from the object registry. Only objects added with this interface can be removed with this
        /// operation. Objects added with deployment descriptors should be removed with the deployment mechanism.
        /// @param id The identity of the object to remove.
        /// @throws ObjectNotRegisteredException Thrown when the object isn't registered with the registry.
        /// @throws DeploymentException Thrown when the object can't be removed.
        void removeObject(Ice::Identity id)
            throws ObjectNotRegisteredException, DeploymentException;

        /// Gets the object info for the object.
        /// @param id The identity of the object.
        /// @return The object info.
        /// @throws ObjectNotRegisteredException Thrown when the object isn't registered with the registry.
        ["cpp:const"]
        idempotent ObjectInfo getObjectInfo(Ice::Identity id)
            throws ObjectNotRegisteredException;

        /// Gets the object info of all the registered objects with a given type.
        /// @param type The type name.
        /// @return The object infos.
        ["cpp:const"]
        idempotent ObjectInfoSeq getObjectInfosByType(string type);

        /// Gets the object info of all the registered objects whose stringified identities match the given expression.
        /// @param expr The expression to match against the stringified identities of registered objects. The expression
        /// may contain a trailing wildcard (`*`) character.
        /// @return All the object infos with a stringified identity matching the given expression.
        ["cpp:const"]
        idempotent ObjectInfoSeq getAllObjectInfos(string expr);

        /// Pings an IceGrid node to see if it is active.
        /// @param name The node name.
        /// @return `true` if the node ping succeeded, `false` otherwise.
        /// @throws NodeNotExistException Thrown when the node doesn't exist.
        ["cpp:const"]
        idempotent bool pingNode(string name)
            throws NodeNotExistException;

        /// Gets the load averages of a node.
        /// @param name The node name.
        /// @return The node load information.
        /// @throws NodeNotExistException Thrown when the node doesn't exist.
        /// @throws NodeUnreachableException Thrown when the node is unreachable.
        ["cpp:const"]
        idempotent LoadInfo getNodeLoad(string name)
            throws NodeNotExistException, NodeUnreachableException;

        /// Gets the node information of a node.
        /// @param name The node name.
        /// @return The node information.
        /// @throws NodeNotExistException Thrown when the node doesn't exist.
        /// @throws NodeUnreachableException Thrown when the node is unreachable.
        ["cpp:const"]
        idempotent NodeInfo getNodeInfo(string name)
            throws NodeNotExistException, NodeUnreachableException;

        /// Gets a proxy to the admin object of an IceGrid node.
        /// @param name The IceGrid node name.
        /// @return A proxy to the IceGrid node's admin object. This proxy is never null.
        /// @throws NodeNotExistException Thrown when the node doesn't exist.
        /// @throws NodeUnreachableException Thrown when the node is unreachable.
        ["cpp:const"]
        idempotent Object* getNodeAdmin(string name)
            throws NodeNotExistException, NodeUnreachableException;

        /// Gets the number of physical processor sockets in the computer where an IceGrid node is deployed.
        /// Note that this operation returns 1 on operating systems where this can't be automatically determined and
        /// where the `IceGrid.Node.ProcessorSocketCount` property for the node is not set.
        /// @param name The node name.
        /// @return The number of processor sockets or 1 if the number of sockets can't be determined.
        /// @throws NodeNotExistException Thrown when the node doesn't exist.
        /// @throws NodeUnreachableException Thrown when the node is unreachable.
        ["cpp:const"]
        idempotent int getNodeProcessorSocketCount(string name)
            throws NodeNotExistException, NodeUnreachableException;

        /// Shuts down an IceGrid node.
        /// @param name The node name.
        /// @throws NodeNotExistException Thrown when the node doesn't exist.
        /// @throws NodeUnreachableException Thrown when the node is unreachable.
        void shutdownNode(string name)
            throws NodeNotExistException, NodeUnreachableException;

        /// Get the hostname of a node.
        /// @param name The node name.
        /// @return The node hostname.
        /// @throws NodeNotExistException Thrown when the node doesn't exist.
        /// @throws NodeUnreachableException Thrown when the node is unreachable.
        ["cpp:const"]
        idempotent string getNodeHostname(string name)
            throws NodeNotExistException, NodeUnreachableException;

        /// Gets the names of all IceGrid nodes currently registered.
        /// @return The node names.
        ["cpp:const"]
        idempotent Ice::StringSeq getAllNodeNames();

        /// Pings an IceGrid registry to see if it is active.
        /// @param name The registry name.
        /// @return `true` if the registry ping succeeded, `false` otherwise.
        /// @throws RegistryNotExistException Thrown when the registry doesn't exist.
        ["cpp:const"]
        idempotent bool pingRegistry(string name)
            throws RegistryNotExistException;

        /// Gets the registry information of an IceGrid registry.
        /// @param name The registry name.
        /// @return The registry information.
        /// @throws RegistryNotExistException Thrown when the registry doesn't exist.
        /// @throws RegistryUnreachableException Thrown when the registry is unreachable.
        ["cpp:const"]
        idempotent RegistryInfo getRegistryInfo(string name)
            throws RegistryNotExistException, RegistryUnreachableException;

        /// Gets a proxy to the admin object of an IceGrid registry.
        /// @param name The registry name.
        /// @return A proxy to the admin object of an IceGrid registry. This proxy is never null.
        /// @throws RegistryNotExistException Thrown when the registry doesn't exist.
        ["cpp:const"]
        idempotent Object* getRegistryAdmin(string name)
            throws RegistryNotExistException;

        /// Shuts down an IceGrid registry.
        /// @param name The registry name.
        /// @throws RegistryNotExistException Thrown when the registry doesn't exist.
        /// @throws RegistryUnreachableException Thrown when the registry is unreachable.
        idempotent void shutdownRegistry(string name)
            throws RegistryNotExistException, RegistryUnreachableException;

        /// Gets the names of all the IceGrid registries currently registered.
        /// @return The registry names.
        ["cpp:const"]
        idempotent Ice::StringSeq getAllRegistryNames();

        /// Shuts down the IceGrid registry.
        void shutdown();
    }

    /// Iterates over an IceGrid log file.
    interface FileIterator
    {
        /// Read lines from the log file.
        /// @param size Specifies the maximum number of bytes to be received. The server will ensure that the returned
        /// message doesn't exceed the given size.
        /// @param lines The lines read from the file. If there was nothing to read from the file since the last call to
        /// read, an empty sequence is returned. The last line of the sequence is always incomplete (and therefore no
        /// newline character should be added when writing the last line to the to the output device).
        /// @return `true` if EOF is encountered.
        /// @throws FileNotAvailableException Thrown when the implementation failed to read from the file.
        bool read(int size, out Ice::StringSeq lines)
            throws FileNotAvailableException;

        /// Destroys the iterator.
        void destroy();
    }

    /// Dynamic information about the state of a server.
    struct ServerDynamicInfo
    {
        /// The ID of the server.
        string id;

        /// The state of the server.
        ServerState state;

        /// The process ID of the server.
        int pid;

        /// Indicates whether the server is enabled.
        bool enabled;
    }

    /// A sequence of ServerDynamicInfo.
    ["java:type:java.util.LinkedList<ServerDynamicInfo>"]
    sequence<ServerDynamicInfo> ServerDynamicInfoSeq;

    /// Dynamic information about the state of an adapter.
    struct AdapterDynamicInfo
    {
        /// The id of the adapter.
        string id;

        /// The direct proxy containing the adapter endpoints. This proxy is never null.
        Object* proxy;
    }

    /// A sequence of AdapterDynamicInfo.
    ["java:type:java.util.LinkedList<AdapterDynamicInfo>"]
    sequence<AdapterDynamicInfo> AdapterDynamicInfoSeq;

    /// Dynamic information about the state of a node.
    struct NodeDynamicInfo
    {
        /// Some static information about the node.
        NodeInfo info;

        /// The dynamic information of the servers deployed on this node.
        ServerDynamicInfoSeq servers;

        /// The dynamic information of the adapters deployed on this node.
        AdapterDynamicInfoSeq adapters;
    }

    /// Monitors changes to the state of the registries.
    interface RegistryObserver
    {
        /// Provides the initial state of the registries to the observer.
        /// @param registries The current state of the registries.
        void registryInit(RegistryInfoSeq registries);

        /// Notifies the observer that a registry replica came up.
        /// @param registryReplica The registry state.
        void registryUp(RegistryInfo registryReplica);

        /// Notifies the observer that a registry replica went down.
        /// @param name The registry name.
        void registryDown(string name);
    }

    /// A sequence of NodeDynamicInfo.
    sequence<NodeDynamicInfo> NodeDynamicInfoSeq;

    /// Monitors changes to the state of the nodes.
    interface NodeObserver
    {
        /// Provides the initial state of the nodes to the observer.
        /// @param nodes The current state of the nodes.
        void nodeInit(NodeDynamicInfoSeq nodes);

        /// Notifies the observer that a node came up.
        /// @param node The node state.
        void nodeUp(NodeDynamicInfo node);

        /// Notifies the observer that a node went down.
        /// @param name The node name.
        void nodeDown(string name);

        /// Notifies the observer that the state of a server changed.
        /// @param node The node hosting the server.
        /// @param updatedInfo The new server state.
        void updateServer(string node, ServerDynamicInfo updatedInfo);

        /// Notifies the observer that the state of an object adapter changed.
        /// @param node The node hosting the adapter.
        /// @param updatedInfo The new adapter state.
        void updateAdapter(string node, AdapterDynamicInfo updatedInfo);
    }

    /// Monitors applications.
    interface ApplicationObserver
    {
        /// Provides the initial application infos to the observer.
        /// @param serial The current serial number of the registry database. This serial number allows observers to
        /// make sure that their internal state is synchronized with the registry.
        /// @param applications The applications currently registered with the registry.
        void applicationInit(int serial, ApplicationInfoSeq applications);

        /// Notifies the observer that an application was added.
        /// @param serial The new serial number of the registry database.
        /// @param desc The descriptor of the new application.
        void applicationAdded(int serial, ApplicationInfo desc);

        /// Notifies the observer that an application was removed.
        /// @param serial The new serial number of the registry database.
        /// @param name The name of the application that was removed.
        void applicationRemoved(int serial, string name);

        /// Notifies the observer that an application was updated.
        /// @param serial The new serial number of the registry database.
        /// @param desc The descriptor of the update.
        void applicationUpdated(int serial, ApplicationUpdateInfo desc);
    }

    /// Monitors dynamically-registered object adapters.
    interface AdapterObserver
    {
        /// Provides the initial list of dynamically registered adapters to the observer.
        /// @param adpts The adapters that were dynamically registered with the registry.
        void adapterInit(AdapterInfoSeq adpts);

        /// Notifies the observer that a dynamically-registered adapter was added.
        /// @param info The details of the new adapter.
        void adapterAdded(AdapterInfo info);

        // Notifies the observer that a dynamically-registered adapter was updated.
        /// @param info The details of the updated adapter.
        void adapterUpdated(AdapterInfo info);

        /// Notifies the observer that a dynamically-registered adapter was removed.
        /// @param id The ID of the removed adapter.
        void adapterRemoved(string id);
    }

    /// Monitors well-known objects that are added, updated or removed using {@link Admin}.
    interface ObjectObserver
    {
        /// Provides the initial list of well-known objects to the observer.
        /// @param objects The well-known objects registered using {@link Admin}.
        void objectInit(ObjectInfoSeq objects);

        /// Notifies the observer that a well-known object was added.
        /// @param info The details of the new object.
        void objectAdded(ObjectInfo info);

        /// Notifies the observer that a well-known object was updated.
        /// @param info The details of the updated object.
        void objectUpdated(ObjectInfo info);

       /// Notifies the observer that a well-known object was removed.
        /// @param id The identity of the removed object.
        void objectRemoved(Ice::Identity id);
    }

    /// Represents an administrative session between an admin tool and an IceGrid registry.
    /// @see Registry
    interface AdminSession extends Glacier2::Session
    {
        /// Keeps the session alive.
        ["deprecated:As of Ice 3.8, there is no need to call this operation, and its implementation does nothing."]
        idempotent void keepAlive();

        /// Gets a proxy to the IceGrid admin object. The admin object returned by this operation can only be accessed
        /// by the session.
        /// @return A proxy to the IceGrid admin object. This proxy is never null.
        ["cpp:const"]
        idempotent Admin* getAdmin();

        /// Gets a "template" proxy for admin callback objects. An Admin client uses this proxy to set the category of
        /// its callback objects, and the published endpoints of the object adapter hosting the admin callback objects.
        /// @return A template proxy. The returned proxy is null when the Admin session was established using Glacier2.
        ["cpp:const"]
        idempotent Object* getAdminCallbackTemplate();

        /// Sets the observer proxies that receive notifications when the state of the registry or nodes changes.
        /// @param registryObs The registry observer.
        /// @param nodeObs The node observer.
        /// @param appObs The application observer.
        /// @param adptObs The adapter observer.
        /// @param objObs The object observer.
        /// @throws ObserverAlreadyRegisteredException Thrown when an observer is already registered with this registry.
        idempotent void setObservers(
            RegistryObserver* registryObs,
            NodeObserver* nodeObs,
            ApplicationObserver* appObs,
            AdapterObserver* adptObs,
            ObjectObserver* objObs)
            throws ObserverAlreadyRegisteredException;

        /// Sets the observer identities that receive notifications when the state of the registry or nodes changes.
        /// This operation should be used by clients that are using a bidirectional connection to communicate with the
        /// session.
        /// @param registryObs The registry observer identity.
        /// @param nodeObs The node observer identity.
        /// @param appObs The application observer.
        /// @param adptObs The adapter observer.
        /// @param objObs The object observer.
        /// @throws ObserverAlreadyRegisteredException Thrown when an observer is already registered with this registry.
        idempotent void setObserversByIdentity(
            Ice::Identity registryObs,
            Ice::Identity nodeObs,
            Ice::Identity appObs,
            Ice::Identity adptObs,
            Ice::Identity objObs)
            throws ObserverAlreadyRegisteredException;

        /// Acquires an exclusive lock to start updating the registry applications.
        /// @return The current serial.
        /// @throws AccessDeniedException Thrown when the exclusive lock can't be acquired. This might happen if the
        /// lock is currently acquired by another session.
        int startUpdate()
            throws AccessDeniedException;

        /// Finishes updating the registry and releases the exclusive lock.
        /// @throws AccessDeniedException Thrown when the session doesn't hold the exclusive lock.
        void finishUpdate()
            throws AccessDeniedException;

        /// Gets the name of the registry replica hosting this session.
        /// @return The replica name of the registry.
        ["cpp:const"]
        idempotent string getReplicaName();

        /// Opens a server log file for reading.
        /// @param id The server ID.
        /// @param path The path of the log file. A log file can be opened only if it's declared in the server or
        /// service deployment descriptor.
        /// @param count Specifies where to start reading the file. If negative, the file is read from the beginning.
        /// Otherwise, the file is read from the last @p count lines.
        /// @return An iterator to read the file. This proxy is never null.
        /// @throws FileNotAvailableException Thrown when the file can't be read.
        /// @throws ServerNotExistException Thrown when the server doesn't exist.
        /// @throws NodeUnreachableException Thrown when the node is unreachable.
        /// @throws DeploymentException Thrown when the server couldn't be deployed on the node.
        FileIterator* openServerLog(string id, string path, int count)
            throws FileNotAvailableException, ServerNotExistException, NodeUnreachableException, DeploymentException;

        /// Opens a server stderr file for reading.
        /// @param id The server ID.
        /// @param count Specifies where to start reading the file. If negative, the file is read from the beginning.
        /// Otherwise, the file is read from the last @p count lines.
        /// @return An iterator to read the file. This proxy is never null.
        /// @throws FileNotAvailableException Thrown when the file can't be read.
        /// @throws ServerNotExistException Thrown when the server doesn't exist.
        /// @throws NodeUnreachableException Thrown when the node is unreachable.
        /// @throws DeploymentException Thrown when the server couldn't be deployed on the node.
        FileIterator* openServerStdErr(string id, int count)
            throws FileNotAvailableException, ServerNotExistException, NodeUnreachableException, DeploymentException;

        /// Opens a server stdout file for reading.
        /// @param id The server id.
        /// @param count Specifies where to start reading the file. If negative, the file is read from the beginning.
        /// Otherwise, the file is read from the last @p count lines.
        /// @return An iterator to read the file. This proxy is never null.
        /// @throws FileNotAvailableException Thrown when the file can't be read.
        /// @throws ServerNotExistException Thrown when the server doesn't exist.
        /// @throws NodeUnreachableException Thrown when the node is unreachable.
        /// @throws DeploymentException Thrown when the server couldn't be deployed on the node.
        FileIterator* openServerStdOut(string id, int count)
            throws FileNotAvailableException, ServerNotExistException, NodeUnreachableException, DeploymentException;

        /// Opens a node stderr file for reading.
        /// @param name The node name.
        /// @param count Specifies where to start reading the file. If negative, the file is read from the beginning.
        /// Otherwise, the file is read from the last @p count lines.
        /// @return An iterator to read the file. This proxy is never null.
        /// @throws FileNotAvailableException Thrown when the file can't be read.
        /// @throws NodeNotExistException Thrown when the node doesn't exist.
        /// @throws NodeUnreachableException Thrown when the node is unreachable.
        FileIterator* openNodeStdErr(string name, int count)
            throws FileNotAvailableException, NodeNotExistException, NodeUnreachableException;

        /// Opens a node stdout file for reading.
        /// @param name The node name.
        /// @param count Specifies where to start reading the file. If negative, the file is read from the beginning.
        /// Otherwise, the file is read from the last @p count lines.
        /// @return An iterator to read the file. This proxy is never null.
        /// @throws FileNotAvailableException Thrown when the file can't be read.
        /// @throws NodeNotExistException Thrown when the node doesn't exist.
        /// @throws NodeUnreachableException Thrown when the node is unreachable.
        FileIterator* openNodeStdOut(string name, int count)
            throws FileNotAvailableException, NodeNotExistException, NodeUnreachableException;

        /// Opens a registry stderr file for reading.
        /// @param name The registry name.
        /// @param count Specifies where to start reading the file. If negative, the file is read from the beginning.
        /// Otherwise, the file is read from the last @p count lines.
        /// @return An iterator to read the file. This proxy is never null.
        /// @throws FileNotAvailableException Thrown when the file can't be read.
        /// @throws RegistryNotExistException Thrown when the registry doesn't exist.
        /// @throws RegistryUnreachableException Thrown when the registry is unreachable.
        FileIterator* openRegistryStdErr(string name, int count)
            throws FileNotAvailableException, RegistryNotExistException, RegistryUnreachableException;

        /// Opens a registry stdout file for reading.
        /// @param name The registry name.
        /// @param count Specifies where to start reading the file. If negative, the file is read from the beginning.
        /// Otherwise, the file is read from the last @p count lines.
        /// @return An iterator to read the file. This proxy is never null.
        /// @throws FileNotAvailableException Thrown when the file can't be read.
        /// @throws RegistryNotExistException Thrown when the registry doesn't exist.
        /// @throws RegistryUnreachableException Thrown when the registry is unreachable.
        FileIterator * openRegistryStdOut(string name, int count)
            throws FileNotAvailableException, RegistryNotExistException, RegistryUnreachableException;
    }
}
