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
     * Stop the server. This methods returns only when the server is
     * deactivated. If the server doesn't stop after a configurable
     * amount of time, it will be killed.
     *
     **/
    void stop();

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
     * Get the server pid.
     *
     **/
    int getPid();
    
    /**
     * 
     * The description of this server.
     *
     */
    ServerDescription description;

    /**
     *
     * The adapter proxies.
     *
     **/
    Adapters adapters;
};

class ServerManager
{
    /**
     *
     * Create a server.
     *
     **/
    Server* create(ServerDescription desc)
	throws ServerExistsException;


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
