// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PACK_ACTIVATOR_ICE
#define ICE_PACK_ACTIVATOR_ICE

#include <IcePack/ServerManagerF.ice>

module IcePack
{

interface Activator
{
    /**
     *
     * Activate a server.
     *
     * @param server The server to activate.
     *
     **/
    bool activate(Server* server);
};

};

#endif

