// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SERVANT_FACTORY_MANAGER_H
#define ICE_SERVANT_FACTORY_MANAGER_H

#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <Ice/ObjectFactoryManagerF.h>
#include <Ice/ObjectFactoryF.h>

namespace IceInternal
{

class ObjectFactoryManager : public ::IceUtil::Shared, public ::IceUtil::Mutex
{
public:

    void add(const ::Ice::ObjectFactoryPtr&, const std::string&);
    void remove(const std::string&);
    ::Ice::ObjectFactoryPtr find(const std::string&);

private:

    ObjectFactoryManager();
    void destroy();
    friend class Instance;

    std::map<std::string, ::Ice::ObjectFactoryPtr> _factoryMap;
    std::map<std::string, ::Ice::ObjectFactoryPtr>::iterator _factoryMapHint;
};

}

#endif
