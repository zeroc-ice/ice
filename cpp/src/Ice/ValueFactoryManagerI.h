//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_VALUE_FACTORY_MANAGER_I_H
#define ICE_VALUE_FACTORY_MANAGER_I_H

#include <Ice/ValueFactory.h>

#include <list>
#include <mutex>

namespace IceInternal
{

class ValueFactoryManagerI final : public Ice::ValueFactoryManager
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
    mutable std::mutex _mutex;
};

using ValueFactoryManagerIPtr = std::shared_ptr<ValueFactoryManagerI>;

}

#endif
