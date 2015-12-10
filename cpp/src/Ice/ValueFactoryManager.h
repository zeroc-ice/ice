// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_VALUE_FACTORY_MANAGER_H
#define ICE_VALUE_FACTORY_MANAGER_H

#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <Ice/ValueFactoryManagerF.h>
#include <Ice/ValueFactory.h>
#include <Ice/ObjectFactory.h>

namespace IceInternal
{

class ValueFactoryManager : public ::IceUtil::Shared, public ::IceUtil::Mutex
{
public:

    void add(const ICE_VALUE_FACTORY&, const std::string&);
    void add(const Ice::ObjectFactoryPtr&, const std::string&);
    ICE_VALUE_FACTORY find(const std::string&) const;
    Ice::ObjectFactoryPtr findObjectFactory(const std::string&) const;
    typedef std::map<std::string, ICE_VALUE_FACTORY> FactoryMap;
    typedef std::map<std::string, Ice::ObjectFactoryPtr> ObjectFactoryMap;

private:

    ValueFactoryManager();
    void destroy();
    friend class Instance;

    FactoryMap _factoryMap;
    mutable FactoryMap::iterator _factoryMapHint;

    ObjectFactoryMap _objectFactoryMap;
    mutable ObjectFactoryMap::iterator _objectFactoryMapHint;
};

}

#endif
