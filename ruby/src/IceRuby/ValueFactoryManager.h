// Copyright (c) ZeroC, Inc.

#ifndef ICE_RUBY_OBJECT_FACTORY_H
#define ICE_RUBY_OBJECT_FACTORY_H

#include "Config.h"
#include "Ice/ValueFactory.h"

#include <map>

namespace IceRuby
{
    bool initValueFactoryManager(VALUE);

    // The IceRuby C++ value factory abstract base class.
    struct ValueFactory
    {
        virtual std::shared_ptr<Ice::Value> create(std::string_view) = 0;
    };
    using ValueFactoryPtr = std::shared_ptr<ValueFactory>;

    class CustomValueFactory final : public ValueFactory
    {
    public:
        CustomValueFactory(VALUE);

        std::shared_ptr<Ice::Value> create(std::string_view) final;

        VALUE getObject() const;
        void mark();

    private:
        VALUE _factory;
    };
    using CustomValueFactoryPtr = std::shared_ptr<CustomValueFactory>;

    class ValueFactoryManager final : public Ice::ValueFactoryManager
    {
    public:
        static std::shared_ptr<ValueFactoryManager> create();

        void add(Ice::ValueFactory, std::string_view) final;
        Ice::ValueFactory find(std::string_view) const noexcept final;

        void addValueFactory(VALUE, std::string_view);
        VALUE findValueFactory(std::string_view) const;

        void mark();
        void markSelf();
        VALUE getObject() const;

        void destroy();

    private:
        using CustomFactoryMap = std::map<std::string, CustomValueFactoryPtr, std::less<>>;

        ValueFactoryManager();

        VALUE _self;
        CustomFactoryMap _customFactories;
    };
    using ValueFactoryManagerPtr = std::shared_ptr<ValueFactoryManager>;
}

#endif
