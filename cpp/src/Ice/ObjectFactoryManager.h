// **********************************************************************
//
// Copyright (c) 2003
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
    static const char * const _kindOfObject;
};

}

#endif
