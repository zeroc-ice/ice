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

class FactoryWrapper : public Ice::ValueFactory
{
public:

    FactoryWrapper(VALUE);

    virtual std::shared_ptr<Ice::Value> create(std::string_view);

    VALUE getObject() const;

    void mark();

    void destroy();

protected:

    VALUE _factory;
};
using FactoryWrapperPtr = std::shared_ptr<FactoryWrapper>;

class DefaultValueFactory : public Ice::ValueFactory
{
public:

    virtual std::shared_ptr<Ice::Value> create(std::string_view);

    void setDelegate(const Ice::ValueFactoryPtr&);
    Ice::ValueFactoryPtr getDelegate() const { return _delegate; }

    VALUE getObject() const;

    void mark();

    void destroy();

private:

    Ice::ValueFactoryPtr _delegate;
};
using DefaultValueFactoryPtr = std::shared_ptr<DefaultValueFactory>;

class ValueFactoryManager final : public Ice::ValueFactoryManager
{
public:

    static std::shared_ptr<ValueFactoryManager> create();

    ~ValueFactoryManager();

    void add(Ice::ValueFactoryFunc, std::string_view) final;
    void add(Ice::ValueFactoryPtr, std::string_view) final;
    Ice::ValueFactoryFunc find(std::string_view) const noexcept final;

    void addValueFactory(VALUE, std::string_view);
    VALUE findValueFactory(std::string_view) const;

    void mark();
    void markSelf();

    VALUE getObject() const;

    void destroy();

private:

    using FactoryMap = std::map<std::string, Ice::ValueFactoryPtr, std::less<>>;

    ValueFactoryManager();

    Ice::ValueFactoryPtr findCore(std::string_view) const noexcept;

    VALUE _self;
    FactoryMap _factories;
    DefaultValueFactoryPtr _defaultFactory;

    mutable std::mutex _mutex;
};
using ValueFactoryManagerPtr = std::shared_ptr<ValueFactoryManager>;

}

#endif
