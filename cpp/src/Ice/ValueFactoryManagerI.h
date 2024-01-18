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

class ValueFactoryManagerI final : public Ice::ValueFactoryManager,
                                   public IceUtil::Mutex
{
public:

    ValueFactoryManagerI();

    void add(Ice::ValueFactoryFunc, const std::string&) final;
    void add(const Ice::ValueFactoryPtr&, const std::string&) final;
    Ice::ValueFactoryFunc find(const std::string&) const noexcept final;

private:

    using FactoryFuncMap = std::map<std::string, Ice::ValueFactoryFunc>;

    FactoryFuncMap _factoryFuncMap;
    mutable FactoryFuncMap::iterator _factoryFuncMapHint;
};

using ValueFactoryManagerIPtr = ::std::shared_ptr<ValueFactoryManagerI>;

}

#endif
