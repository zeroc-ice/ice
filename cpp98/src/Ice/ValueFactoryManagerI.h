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

    virtual void add(ICE_IN(ICE_DELEGATE(::Ice::ValueFactory)), const std::string&);
    virtual ICE_DELEGATE(::Ice::ValueFactory) find(const std::string&) const ICE_NOEXCEPT;

private:

    typedef std::map<std::string, ICE_DELEGATE(::Ice::ValueFactory)> FactoryMap;

    FactoryMap _factoryMap;
    mutable FactoryMap::iterator _factoryMapHint;
};

}

#endif
