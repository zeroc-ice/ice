// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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

    Ice::CommunicatorPtr _locatorComm;
    Ice::CommunicatorPtr _locatorRegistryComm;
    Ice::CommunicatorPtr _adminComm;

    Freeze::DBEnvironmentPtr _dbEnv;
};

}

#endif
