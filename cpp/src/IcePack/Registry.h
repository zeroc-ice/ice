// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PACK_REGISTRY_H
#define ICE_PACK_REGISTRY_H

namespace IcePack
{

class Registry
{
public:

    Registry(const Ice::CommunicatorPtr&);
    ~Registry();

    bool start(bool);

private:

    Ice::CommunicatorPtr _communicator;
    Freeze::DBEnvironmentPtr _dbEnv;
};

}

#endif
