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
#include <map>

namespace IcePack
{

class AdminI : public Admin, public JTCMutex
{
public:

    AdminI(const Ice::CommunicatorPtr&);

    virtual void add(const ServerDescriptionPtr&);
    virtual void remove(const std::string&);
    virtual ServerDescriptionPtr find(const std::string&);
    virtual ServerDescriptions getAll();
    virtual void shutdown();

private:

    Ice::CommunicatorPtr _communicator;
    std::map<std::string, ServerDescriptionPtr> _map;
};

}

#endif
