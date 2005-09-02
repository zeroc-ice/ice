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

dictionary<string, Object*> StringObjectProxyDict;

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

struct ObjectInfo
{
    /** The proxy of the object. */
    Object* proxy;

    /** The type of the object. */
    string type;
};
sequence<ObjectInfo> ObjectInfoSeq;

struct ServerInfo
{
    /** The server application. */
    string application;

    /** The server node. */
    string node;
       
    /** The server descriptor. */
    ServerDescriptor descriptor;
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
	throws DeploymentException;

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
     * Patch the given application data. If the patch argument is an
     * empty string, all the application servers depending on patch
     * data will be patched.
     *
     * @param name The application name.
     *
     * @param patch The patch identifier or an empty string.
     *
     * @param shutdown If true, the servers depending on the data to
     * patch will be shutdown if necessary.
     *
     * @throws PatchException Raised if the patch failed.
     *
     **/
    void patchApplication(string name, string patch, bool shutdown)
	throws ApplicationNotExistException, PatchException;

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
     * @throws ServerNotExistException Raised if the server is not
     * found.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     * @see getServerPid
     * @see getAllServerIds
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
     * @throws ServerNotExistException Raised if the server is not
     * found.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     * @see getServerState
     * @see getAllServerIds
     *
     **/
    nonmutating int getServerPid(string id)
	throws ServerNotExistException, NodeUnreachableException;

    /**
     *
     * Get the server's activation mode.
     *
     * @param id The id of the server.
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
     * @see getAllServerIds
     *
     **/
    nonmutating ServerActivation getServerActivation(string id)
	throws ServerNotExistException, NodeUnreachableException;

    /**
     *
     * Set the server's activation mode.
     *
     * @param id The id of the server.
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
     * @see getAllServerIds
     *
     **/
    void setServerActivation(string id, ServerActivation mode)
	throws ServerNotExistException, NodeUnreachableException;

    /**
     *
     * Start a server.
     *
     * @param id The id of the server.
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
    bool startServer(string id)
	throws ServerNotExistException, NodeUnreachableException;

    /**
     *
     * Stop a server.
     *
     * @param id The id of the server.
     *
     * @throws ServerNotExistException Raised if the server is not
     * found.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     **/
    void stopServer(string id)
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
     * @throws ServerNotExistException Raised if the server is not
     * found.
     *
     * @throws NodeUnreachableException Raised if the node could not be
     * reached.
     *
     * @throws PatchException Raised if the patch failed.
     *
     **/
    void patchServer(string id, bool shutdown)
	throws ServerNotExistException, NodeUnreachableException, PatchException;

    /**
     *
     * Send signal to a server.
     *
     * @param id The id of the server.
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
     * @throws ServerNotExistException Raised if the server is not
     * found.
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
     * @see getServerState
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
     * @throws AdapterNotExistException Raised if the adapter is not
     * found.
     *
     **/
    nonmutating StringObjectProxyDict getAdapterEndpoints(string adapterId)
	throws AdapterNotExistException, NodeUnreachableException;

    /**
     *
     * Remove the adapter with the given adapter id and server id.
     *
     * @throws AdapterNotExistException Raised if the adapter is not
     * found.
     *
     * @throws ServerNotExistException Raised if the server is not
     * found.
     *
     **/
    idempotent void removeAdapterWithServerId(string adapterId, string serverId)
	throws AdapterNotExistException, ServerNotExistException;

    /**
     *
     * Remove the adapter with the given id. If the adapter is
     * replicated, all the replicas are removed.
     *
     * @throws AdapterNotExistException Raised if the adapter is not
     * found.
     *
     **/
    idempotent void removeAdapter(string adapterId)
	throws AdapterNotExistException;

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
     * Get the object info for the object with the given identity.
     *
     * @param id The identity of the object.
     *
     * @return The object info.
     *
     * @throws ObjectNotExistExcpetion Raised if the object cannot be
     * found.
     *
     **/
    nonmutating ObjectInfo getObjectInfo(Ice::Identity id)
	throws ObjectNotExistException;

    /**
     *
     * Get the object info of all the registered objects whose stringified
     * identities match the given expression.
     *
     * @param expr The expression to match against the stringified
     * identities of registered objects. The expression may contain
     * a trailing wildcard (<literal>*</literal>) character.
     *
     * @return All the object info with a stringified identity
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
