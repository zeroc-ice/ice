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
    virtual void remove(const Ice::ObjectPrx&);
    virtual ServerDescriptionPtr find(const Ice::ObjectPrx&);
    virtual ServerDescriptions getAll();
    virtual void shutdown();

private:

    Ice::CommunicatorPtr _communicator;
    std::map<Ice::ObjectPrx, ServerDescriptionPtr> _map;
};

}

#endif
