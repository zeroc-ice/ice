//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_RUBY_OBJECT_FACTORY_H
#define ICE_RUBY_OBJECT_FACTORY_H

#include <Config.h>
#include <Ice/ValueFactory.h>

#include <mutex>

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

    protected:
        VALUE _factory;
    };
    using CustomValueFactoryPtr = std::shared_ptr<CustomValueFactory>;

    class DefaultValueFactory final : public ValueFactory
    {
    public:
        std::shared_ptr<Ice::Value> create(std::string_view) final;

        void setDelegate(const ValueFactoryPtr&);
        ValueFactoryPtr getDelegate() const { return _delegate; }

        VALUE getObject() const;

        void mark();

        void destroy();

    private:
        ValueFactoryPtr _delegate;
    };
    using DefaultValueFactoryPtr = std::shared_ptr<DefaultValueFactory>;

    class ValueFactoryManager final : public Ice::ValueFactoryManager
    {
    public:
        static std::shared_ptr<ValueFactoryManager> create();
        ~ValueFactoryManager();

        void add(Ice::ValueFactoryFunc, std::string_view) final;
        void add(ValueFactoryPtr, std::string_view);
        Ice::ValueFactoryFunc find(std::string_view) const noexcept final;

        void addValueFactory(VALUE, std::string_view);
        VALUE findValueFactory(std::string_view) const;

        void mark();
        void markSelf();

        VALUE getObject() const;

        void destroy();

    private:
        using CustomFactoryMap = std::map<std::string, ValueFactoryPtr, std::less<>>;

        ValueFactoryManager();

        ValueFactoryPtr findCore(std::string_view) const noexcept;

        VALUE _self;
        CustomFactoryMap _customFactories;
        DefaultValueFactoryPtr _defaultFactory;

        mutable std::mutex _mutex;
    };
    using ValueFactoryManagerPtr = std::shared_ptr<ValueFactoryManager>;
}

#endif
