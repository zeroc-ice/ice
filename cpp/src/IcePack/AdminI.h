// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PACK_ADMIN_I_H
#define ICE_PACK_ADMIN_I_H

#include <IcePack/Admin.h>

namespace IcePack
{

class AdminI : public Admin, public JTCMutex
{
public:

    AdminI(const Ice::CommunicatorPtr&);

    virtual void add(const ServerDescription&);
    virtual void remove(const std::string&);
    virtual ServerDescription find(const std::string&);
    virtual ServerDescriptions getAll();
    virtual void shutdown();

private:

    Ice::CommunicatorPtr _communicator;
};

}

#endif
