//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_RUBY_OBJECT_FACTORY_H
#define ICE_RUBY_OBJECT_FACTORY_H

#include <Config.h>
#include <Ice/ValueFactory.h>
namespace IceRuby
{

bool initValueFactoryManager(VALUE);

class FactoryWrapper : public Ice::ValueFactory
{
public:

    FactoryWrapper(VALUE);

    virtual std::shared_ptr<Ice::Value> create(const std::string&);

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

    virtual std::shared_ptr<Ice::Value> create(const std::string&);

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

    void add(Ice::ValueFactoryFunc, const std::string&) final;
    void add(const Ice::ValueFactoryPtr&, const std::string&) final;
    Ice::ValueFactoryFunc find(const std::string&) const noexcept final;

    void addValueFactory(VALUE, const std::string&);
    VALUE findValueFactory(const std::string&) const;

    void mark();
    void markSelf();

    VALUE getObject() const;

    void destroy();

private:

    using FactoryMap = std::map<std::string, Ice::ValueFactoryPtr>;

    ValueFactoryManager();

    Ice::ValueFactoryPtr findCore(const std::string&) const noexcept;

    VALUE _self;
    FactoryMap _factories;
    DefaultValueFactoryPtr _defaultFactory;

    mutable std::mutex _mutex;
};
using ValueFactoryManagerPtr = std::shared_ptr<ValueFactoryManager>;

}

#endif
