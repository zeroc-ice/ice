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

#ifndef ICE_USER_EXCEPTION_FACTORY_MANAGER_H
#define ICE_USER_EXCEPTION_FACTORY_MANAGER_H

#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <Ice/UserExceptionFactoryManagerF.h>
#include <Ice/UserExceptionFactoryF.h>

namespace IceInternal
{

class UserExceptionFactoryManager : public IceUtil::Shared, public IceUtil::Mutex
{
public:

    void add(const Ice::UserExceptionFactoryPtr&, const std::string&);
    void remove(const std::string&);
    Ice::UserExceptionFactoryPtr find(const std::string&);

private:

    UserExceptionFactoryManager();
    void destroy();
    friend class Instance;

    std::map<std::string, Ice::UserExceptionFactoryPtr> _factoryMap;
    std::map<std::string, Ice::UserExceptionFactoryPtr>::iterator _factoryMapHint;
};

}

#endif
