// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
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
#include <Ice/ObjectFactory.h>

namespace IceInternal
{

class ObjectFactoryManager : public ::IceUtil::Shared, public ::IceUtil::Mutex
{
public:

    void add(const ICE_OBJECT_FACTORY&, const std::string&);
    ICE_OBJECT_FACTORY find(const std::string&) const;
    typedef std::map<std::string, ICE_OBJECT_FACTORY> FactoryMap;
    
    void remove(const std::string&);

private:

    ObjectFactoryManager();
    void destroy();
    friend class Instance;

    FactoryMap _factoryMap;
    mutable FactoryMap::iterator _factoryMapHint;
};

}

#endif
