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

#include <Ice/ValueFactoryManagerF.h>
#include <Ice/ValueFactoryF.h>
#include <Ice/Shared.h>
#include <map>

namespace IceInternal
{

class ValueFactoryManager : public ::Ice::Shared, public JTCMutex
{
public:

    void install(const ::Ice::ValueFactoryPtr&, const std::string&);
    ::Ice::ValueFactoryPtr lookup(const std::string&);

private:

    ValueFactoryManager();
    void destroy();
    friend class Instance;

    std::map<std::string, ::Ice::ValueFactoryPtr> _factories;
};

}

#endif
