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

class AdminI : public Admin, public IceUtil::Mutex
{
public:

    AdminI(const Ice::CommunicatorPtr&);

    virtual void add(const ServerDescription&, const Ice::Current&);
    virtual void remove(const Ice::Identity&, const Ice::Current&);
    virtual ServerDescription find(const Ice::Identity&, const Ice::Current&);
    virtual ServerDescriptions getAll(const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

private:

    Ice::CommunicatorPtr _communicator;
};

}

#endif
