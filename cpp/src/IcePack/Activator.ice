// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
     * Send signal to server.
     *
     **/
    void sendSignal(Server theServer, string signal);

    /**
     *
     * Write message on the server stdout or stderr
     *
     **/
    void writeMessage(Server theServer, string message, int fd);
   
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

