// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PACK_REGISTRY_H
#define ICE_PACK_REGISTRY_H

namespace IcePack
{

class Registry : public IceUtil::Shared
{
public:

    Registry(const Ice::CommunicatorPtr&);
    ~Registry();

    bool start(bool);

    virtual void shutdown();

private:

    Ice::CommunicatorPtr _communicator;
};
typedef IceUtil::Handle<Registry> RegistryPtr;

}

#endif
