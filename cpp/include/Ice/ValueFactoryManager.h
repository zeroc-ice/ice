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

class ValueFactoryManager : public Shared, public JTCMutex
{
public:

    void install(const ::Ice::ValueFactory_ptr&, const std::string&);
    ::Ice::ValueFactory_ptr lookup(const std::string&);

private:

    ValueFactoryManager();
    void destroy();
    friend class Instance;

    std::map<std::string, ::Ice::ValueFactory_ptr> factories_;
};

}

#endif
