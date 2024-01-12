//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_VALUE_FACTORY_MANAGER_I_H
#define ICE_VALUE_FACTORY_MANAGER_I_H

#include <Ice/ValueFactory.h>
#include <IceUtil/Mutex.h>
#include <list>

namespace IceInternal
{

class ValueFactoryManagerI : public Ice::ValueFactoryManager,
                             public IceUtil::Mutex
{
public:

    ValueFactoryManagerI();

    virtual void add(Ice::ValueFactoryFunc, const std::string&);
    virtual void add(const Ice::ValueFactoryPtr&, const std::string&);
    virtual Ice::ValueFactoryFunc find(const std::string&) const noexcept;

private:

    using FactoryFuncMap = std::map<std::string, Ice::ValueFactoryFunc>;

    FactoryFuncMap _factoryFuncMap;
    mutable FactoryFuncMap::iterator _factoryFuncMapHint;

    std::list<Ice::ValueFactoryPtr> _factories; // keep factories to prevent them from being destroyed
};

using ValueFactoryManagerIPtr = ::std::shared_ptr<ValueFactoryManagerI>;

}

#endif
