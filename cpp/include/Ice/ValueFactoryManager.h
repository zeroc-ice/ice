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

namespace __Ice
{

class ValueFactoryManagerI : public Shared, public JTCMutex
{
public:

    void install(const ::Ice::ValueFactory&);
    ::Ice::ValueFactory lookup(const std::string&);

private:

    ValueFactoryManagerI();
    void destroy();
    friend class InstanceI;

    std::map<std::string, ::Ice::ValueFactory> factories_;
};

}

#endif
