// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
};

}

#endif
