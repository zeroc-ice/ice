// Copyright (c) ZeroC, Inc.

#ifndef ICE_VALUE_FACTORY_MANAGER_I_H
#define ICE_VALUE_FACTORY_MANAGER_I_H

#include "Ice/ValueFactory.h"

#include <list>
#include <map>
#include <mutex>

namespace IceInternal
{
    class ValueFactoryManagerI final : public Ice::ValueFactoryManager
    {
    public:
        ValueFactoryManagerI();

        void add(Ice::ValueFactory, std::string_view) final;
        [[nodiscard]] Ice::ValueFactory find(std::string_view) const noexcept final;

    private:
        using FactoryFuncMap = std::map<std::string, Ice::ValueFactory, std::less<>>;

        FactoryFuncMap _factoryFuncMap;
        mutable FactoryFuncMap::iterator _factoryFuncMapHint;
        mutable std::mutex _mutex;
    };

    using ValueFactoryManagerIPtr = std::shared_ptr<ValueFactoryManagerI>;
}

#endif
