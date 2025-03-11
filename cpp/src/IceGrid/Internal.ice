// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:header-ext:h"]]

#include "Ice/Identity.ice"
#include "Ice/BuiltinSequences.ice"
#include "Ice/Process.ice"
#include "Glacier2/Session.ice"
#include "IceGrid/Admin.ice"
#include "IceGrid/Registry.ice"

module IceGrid
{
    // This class is no longer used. We keep it only for interop with IceGrid 3.7.
    class InternalDbEnvDescriptor
    {
        /// The name of the database environment.
        string name;

        /// The database properties.
        PropertyDescriptorSeq properties;
    }
    sequence<InternalDbEnvDescriptor> InternalDbEnvDescriptorSeq;

    class InternalAdapterDescriptor
    {
        /// The identifier of the server.
        string id;

        /// Specifies if the lifetime of the adapter is the same as the server.
        bool serverLifetime;
    }
    sequence<InternalAdapterDescriptor> InternalAdapterDescriptorSeq;

    dictionary<string, PropertyDescriptorSeq> PropertyDescriptorSeqDict;

    class InternalServerDescriptor
    {
        /// The server ID.
        string id;

        /// The server application
        string application;

        /// The application uuid.
        string uuid;

        /// The application revision.
        int revision;

        /// The id of the session which allocated the server.
        string sessionId;

        /// The server executable.
        string exe;

        /// The server working directory.
        string pwd;

        /// The user ID to use to run the server.
        string user;

        /// The server activation mode.
        string activation;

        /// The server activation timeout.
        string activationTimeout;

        /// The server deactivation timeout.
        string deactivationTimeout;

        /// Specifies if a process object is registered.
        bool processRegistered;

        /// The server command line options.
        Ice::StringSeq options;

        /// The server environment variables.
        Ice::StringSeq envs;

        /// The path of the server logs.
        Ice::StringSeq logs;

        /// The indirect object adapters.
        InternalAdapterDescriptorSeq adapters;

        // Not used, always empty. Kept only for interop with IceGrid 3.7.
        InternalDbEnvDescriptorSeq dbEnvs;

        /// The configuration files of the server.
        PropertyDescriptorSeqDict properties;

        /// IceBox service names
        optional(1) Ice::StringSeq services;
    }

    /// This exception is raised if an adapter is active.
    exception AdapterActiveException
    {
    }

    exception AdapterNotActiveException
    {
        /// True if the adapter can be activated on demand.
        bool activatable;
    }

    interface Adapter
    {
        /// Activate this adapter. If this adapter can be activated, this will activate the adapter and return the direct
        /// proxy of the adapter once it's active. If this adapter can be activated on demand, this will return 0 if the
        /// adapter is inactive or the adapter direct proxy it's active.
        ["amd"] Object* activate();

        /// Get the adapter direct proxy. The adapter direct proxy is a proxy created with the object adapter. The proxy
        /// contains the last known adapter endpoints.
        /// @return A direct proxy containing the last known adapter endpoints if the adapter is already active.
        ["cpp:const"] idempotent Object* getDirectProxy()
            throws AdapterNotActiveException;

        /// Set the direct proxy for this adapter.
        /// @param proxy The direct proxy. The direct proxy should be created with the object adapter and should contain the
        /// object adapter endpoints.
        /// @throws AdapterActiveException The adapter is already active. It's not possible to override the direct proxy of
        /// an active adapter.
        void setDirectProxy(Object* proxy)
            throws AdapterActiveException;
    }

    /// This exception is raised if an adapter with the same name already exists.
    exception AdapterExistsException
    {
        string id;
    }

    dictionary<string, Adapter*> AdapterPrxDict;

    interface FileReader
    {
        /// Count the number of given lines from the end of the file and return the file offset.
        ["cpp:const"] idempotent long getOffsetFromEnd(string filename, int lines)
            throws FileNotAvailableException;

        /// Read lines (or size bytes) at the specified position from the given file.
        ["cpp:const"] idempotent bool read(string filename, long pos, int size, out long newPos, out Ice::StringSeq lines)
            throws FileNotAvailableException;
    }

    interface Server extends FileReader
    {
        /// Start the server.
        ["amd"] void start()
            throws ServerStartException;

        /// Stop the server. This methods returns only when the server is deactivated. If the server doesn't stop after a
        /// configurable amount of time, it will be killed.
        ["amd"] void stop()
            throws ServerStopException;

        /// Check if the given server can be loaded on this node.
        /// @return True if the server is inactive.
        /// @throws DeploymentException Raised if the server can't be updated.
        bool checkUpdate(InternalServerDescriptor svr, bool noRestart)
            throws DeploymentException;

        /// Enable or disable the server.
        void setEnabled(bool enable);

        /// Check if the server is enabled.
        ["cpp:const"] idempotent bool isEnabled();

        /// Send signal to the server
        void sendSignal(string signal)
            throws BadSignalException;

        /// Write message on servers' stdout or stderr.
        void writeMessage(string message, int fd);

        /// Return the server state.
        /// @return The server state.
        /// @see ServerState
        ["cpp:const"] idempotent ServerState getState();

        /// Get the server pid. Note that the value returned by this method is system dependant. On Unix operating systems,
        /// it's the pid value returned by the fork() system call and converted to an integer.
        ["cpp:const"] idempotent int getPid();

        /// Set the process proxy.
        ["amd"] void setProcess(Ice::Process* proc);
    }

    interface InternalRegistry;
    sequence<InternalRegistry*> InternalRegistryPrxSeq;

    interface ReplicaObserver
    {
        /// Initialization of the replica observer.
        void replicaInit(InternalRegistryPrxSeq replicas);

        /// Notification that a replica has been added. The node should establish a session with this new replica.
        void replicaAdded(InternalRegistry* replica);

        /// Notification that a replica has been removed. The node should destroy the session to this replica.
        void replicaRemoved(InternalRegistry* replica);
    }

    interface Node extends FileReader, ReplicaObserver
    {
        /// Load the given server. If the server resources weren't already created (database environment directories,
            /// property files, etc), they will be created. The returned proxy is never null.
        ["amd"] idempotent Server* loadServer(
            InternalServerDescriptor svr,
            string replicaName,
            out AdapterPrxDict adapters,
            out int activateTimeout,
            out int deactivateTimeout) throws DeploymentException;

        /// Load the given server and ensure the server won't be restarted. If the server resources weren't already created
        /// (database environment directories, property files, etc), they will be created. If the server can't be updated
        /// without a restart, a DeploymentException is raised. The returned proxy is never null.
        ["amd"] idempotent Server* loadServerWithoutRestart(
            InternalServerDescriptor svr,
            string replicaName,
            out AdapterPrxDict adapters,
            out int activateTimeout,
            out int deactivateTimeout) throws DeploymentException;

        /// Destroy the given server.
        ["amd"] idempotent void destroyServer(string name, string uuid, int revision, string replicaName)
            throws DeploymentException;

        /// Destroy the server if it's not active.
        ["amd"] idempotent void destroyServerWithoutRestart(string name, string uuid, int revision, string replicaName)
            throws DeploymentException;

        /// Establish a session to the given replica, this method only returns once the registration was attempted (unlike
        /// replicaAdded below).
        void registerWithReplica(InternalRegistry* replica);

        /// Get the node name.
        ["cpp:const"] idempotent string getName();

        /// Get the node hostname.
        ["cpp:const"] idempotent string getHostname();

        /// Get the node load.
        ["cpp:const"] idempotent LoadInfo getLoad();

        /// Get the number of processor sockets for the machine where this node is running.
        ["cpp:const"] idempotent int getProcessorSocketCount();

        /// Shutdown the node.
        ["cpp:const"] idempotent void shutdown();
    }

    sequence<Node*> NodePrxSeq;

    /// This exception is raised if a node is already registered and active.
    exception NodeActiveException
    {
    }

    interface NodeSession
    {
        /// The node call this method to keep the session alive.
        void keepAlive(LoadInfo load);

        /// Set the replica observer. The node calls this method when it's ready to receive notifications for the replicas.
        /// It only calls this for the session with the master.
        void setReplicaObserver(ReplicaObserver* observer);

        /// Return the node session timeout.
        ["cpp:const"] idempotent int getTimeout();

        /// Return the node observer.
        ["cpp:const"] idempotent NodeObserver* getObserver();

        /// Ask the registry to load the servers on the node.
        ["amd"] ["cpp:const"] idempotent void loadServers();

        /// Get the name of the servers deployed on the node.
        ["cpp:const"] idempotent Ice::StringSeq getServers();

        /// Wait for the application update to complete (the application is completely updated once all the registry
        /// replicas have been updated). This is used by the node to ensure that before to start a server all the
        /// replicas have the up-to-date descriptor of the server.
        ["amd"] ["cpp:const"] void waitForApplicationUpdate(string application, int revision);

        /// Destroy the session.
        void destroy();
    }

    /// This exception is raised if a replica is already registered and active.
    exception ReplicaActiveException
    {
    }

    enum TopicName
    {
        RegistryObserver,
        NodeObserver,
        ApplicationObserver,
        AdapterObserver,
        ObjectObserver
    }

    interface DatabaseObserver extends ApplicationObserver, ObjectObserver, AdapterObserver
    {
    }

    dictionary<string, long> StringLongDict;

    interface ReplicaSession
    {
        /// The replica call this method to keep the session alive.
        void keepAlive();

        /// Return the replica session timeout.
        ["cpp:const"] idempotent int getTimeout();

        /// Set the database observer. Once the observer is subscribed, it will receive the database and database updates.
        idempotent void setDatabaseObserver(DatabaseObserver* dbObs, optional(1) StringLongDict serials)
            throws ObserverAlreadyRegisteredException, DeploymentException;

        /// This method sets the endpoints of the replica. This allows the master to create proxies with multiple endpoints
        /// for replicated objects (e.g.: IceGrid::Query object).
        idempotent void setEndpoints(StringObjectProxyDict endpoints);

        /// Registers the replica well-known objects with the master.
        idempotent void registerWellKnownObjects(ObjectInfoSeq objects);

        /// Set the adapter direct proxy of the given adapter in the master. This is used to support dynamic registration
        /// with the locator registry interface.
        idempotent void setAdapterDirectProxy(string adapterId, string replicaGroupId, Object* proxy)
            throws AdapterNotExistException, AdapterExistsException;

        /// Notify the master that an update was received. The master might wait for replication updates to be received by
        /// all the replicas before to continue.
        void receivedUpdate(TopicName name, int serial, string failure);

        /// Destroy the session.
        void destroy();
    }

    /// Information about an IceGrid node.
    class InternalNodeInfo
    {
        /// The name of the node.
        string name;

        /// The operating system name.
        string os;

        /// The network name of the host running this node (as defined in uname()).
        string hostname;

        /// The operation system release level (as defined in uname()).
        string release;

        /// The operation system version (as defined in uname()).
        string version;

        /// The machine hardware type (as defined in uname()).
        string machine;

        /// The number of processor threads (e.g. 8 on system with 1 quad-core CPU, with 2 threads per core)
        int nProcessors;

        /// The path to the node data directory.
        string dataDir;

        /// The Ice SO version of this node, for example 38. It is typically used to load the same version of the IceStorm
        /// service in IceBox.
        optional(1) string iceSoVersion;
    }

    /// Information about an IceGrid registry replica.
    class InternalReplicaInfo
    {
        /// The name of the registry.
        string name;

        /// The network name of the host running this registry (as defined in uname()).
        string hostname;
    }

    interface InternalRegistry extends FileReader
    {
        /// Register a node with the registry. If a node with the same name is already registered,
        /// <code>registerNode</code> overrides the existing registration only when the previously
        /// registered node is not active.
        /// @param info Some information on the node.
        /// @param prx The proxy of the node.
        /// @param loadInf The load information of the node.
        /// @return The node session proxy.
        /// @throws NodeActiveException Raised if the node is already registered and currently active.
        NodeSession* registerNode(InternalNodeInfo info, Node* prx, LoadInfo loadInf)
            throws NodeActiveException, PermissionDeniedException;

        /// Register a replica with the registry. If a replica with the same name is already registered,
        /// <code>registerReplica</code> overrides the existing registration only when the previously
        /// registered node is not active.
        /// @param info Some information on the replica.
        /// @param prx The proxy of the replica.
        /// @return The replica session proxy.
        /// @throws ReplicaActiveException Raised if the replica is already registered and currently active.
        ReplicaSession* registerReplica(InternalReplicaInfo info, InternalRegistry* prx)
            throws ReplicaActiveException, PermissionDeniedException;

        /// Create a session with the given registry replica. This method returns only once the session creation has been
        /// attempted.
        void registerWithReplica(InternalRegistry* prx);

        /// Return the proxies of all the nodes known by this registry.
        ["cpp:const"] idempotent NodePrxSeq getNodes();

        /// Return the proxies of all the registry replicas known by this registry.
        ["cpp:const"] idempotent InternalRegistryPrxSeq getReplicas();

        /// Return applications, adapters, objects from this replica.
        ["cpp:const"] idempotent ApplicationInfoSeq getApplications(out long serial);
        ["cpp:const"] idempotent AdapterInfoSeq getAdapters(out long serial);
        ["cpp:const"] idempotent ObjectInfoSeq getObjects(out long serial);

        /// Shutdown this registry.
        ["cpp:const"] idempotent void shutdown();
    }
}
