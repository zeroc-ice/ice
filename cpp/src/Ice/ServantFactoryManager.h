// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_VALUE_FACTORY_MANAGER_H
#define ICE_VALUE_FACTORY_MANAGER_H

#include <IceUtil/Shared.h>
#include <Ice/ServantFactoryManagerF.h>
#include <Ice/ServantFactoryF.h>

namespace IceInternal
{

class ServantFactoryManager : public ::IceUtil::Shared, public JTCMutex
{
public:

    void install(const ::Ice::ServantFactoryPtr&, const std::string&);
    ::Ice::ServantFactoryPtr lookup(const std::string&);

private:

    ServantFactoryManager();
    void destroy();
    friend class Instance;

    std::map<std::string, ::Ice::ServantFactoryPtr> _factories;
};

}

#endif
