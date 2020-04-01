//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_VALUE_FACTORY_MANAGER_I_H
#define ICE_VALUE_FACTORY_MANAGER_I_H

#include <Ice/ValueFactory.h>
#include <IceUtil/Mutex.h>

namespace IceInternal
{

class ValueFactoryManagerI;
ICE_DEFINE_PTR(ValueFactoryManagerIPtr, ValueFactoryManagerI);

class ValueFactoryManagerI : public Ice::ValueFactoryManager,
                             public IceUtil::Mutex
{
public:

    ValueFactoryManagerI();

    virtual void add(::Ice::ValueFactory, const std::string&);
    virtual ::Ice::ValueFactory find(const std::string&) const noexcept;

private:

    typedef std::map<std::string, ::Ice::ValueFactory> FactoryMap;

    FactoryMap _factoryMap;
    mutable FactoryMap::iterator _factoryMapHint;
};

}

#endif
