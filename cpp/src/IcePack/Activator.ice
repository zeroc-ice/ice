// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PACK_ACTIVATOR_ICE
#define ICE_PACK_ACTIVATOR_ICE

#include <IcePack/Internal.ice>

module IcePack
{

local interface Activator
{
    /**
     *
     * Activate a server and returns the pid of the server or 0 if the
     * server couldn't be activated.
     *
     * @param server The server to activate.
     *
     **/
    bool activate(Server theServer);

    /**
     *
     * Deactivate a server.
     *
     **/
    void deactivate(Server theServer);

    /**
     *
     * Kill a server.
     *
     **/
    void kill(Server theServer);

    /**
     *
     * Returns the server pid.
     *
     **/
     int getServerPid(Server theServer);
    
    /**
     *
     * Start the activator.
     *
     **/
    void start();

    /**
     *
     * Wait for the activator to be shutdown.
     *
     **/
    void waitForShutdown();

    /**
     *
     * Shutdown the activator.
     *
     **/
    void shutdown();

    /**
     *
     * Destroy the activator. This cause all active server to be
     * stopped. This method returns only once all the servers have
     * been deactivated.
     *
     **/
    void destroy();

};

};

#endif

