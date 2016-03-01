// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_OBJECT_FACTORY_MANAGER_H
#define ICE_OBJECT_FACTORY_MANAGER_H

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
    ::Ice::ObjectFactoryPtr find(const std::string&) const;

    typedef std::map<std::string, ::Ice::ObjectFactoryPtr> FactoryMap;

private:

    ObjectFactoryManager();
    void destroy();
    friend class Instance;

    FactoryMap _factoryMap;
    mutable FactoryMap::iterator _factoryMapHint;
};

}

#endif
