// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PACK_SERVER_MANAGER_ICE
#define ICE_PACK_SERVER_MANAGER_ICE

#include <IcePack/Admin.ice>
#include <IcePack/AdapterManager.ice>

module IcePack
{

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
     * @return True if the server was successfully started, false
     * otherwise.
     *
     **/
    bool start();

    /**
     *
     * This method is called by the activator when it detects that the
     * server has terminated.
     *
     **/
    void terminationCallback();
    
    /**
     *
     * Return the server state.
     *
     **/
    ServerState getState();

    /**
     * 
     * The description of this server.
     *
     */
    ServerDescription _description;

    /**
     *
     * The server state.
     *
     */    
    ServerState _state;

    /**
     *
     * The adapter proxies.
     *
     **/
    Adapters _adapters;
};

class ServerManager
{
    /**
     *
     * Create a server.
     *
     **/
    Server* create(ServerDescription description)
	throws ServerExistsException, AdapterExistsException;

    /**
     *
     * Find an adapter and return its proxy.
     *
     * @param name Name of the adapter.
     *
     * @return Server proxy.
     *
     **/
     Server* findByName(string name);

    /**
     *
     * Remove a server.
     *
     **/
    void remove(string name)
	throws ServerNotExistException, ServerNotInactiveException;
    
    /**
     *
     * Get all server names.
     *
     **/
    ServerNames getAll();
};

};

#endif
