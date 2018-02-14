// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_RUBY_OBJECT_FACTORY_H
#define ICE_RUBY_OBJECT_FACTORY_H

#include <Config.h>
#include <Ice/ValueFactory.h>
#include <IceUtil/Mutex.h>

namespace IceRuby
{

bool initValueFactoryManager(VALUE);

class FactoryWrapper : public Ice::ValueFactory
{
public:

    FactoryWrapper(VALUE, bool);

    virtual Ice::ValuePtr create(const std::string&);

    VALUE getObject() const;

    bool isObjectFactory() const;

    void mark();

    void destroy();

protected:

    VALUE _factory;
    bool _isObjectFactory;
};
typedef IceUtil::Handle<FactoryWrapper> FactoryWrapperPtr;

class DefaultValueFactory : public Ice::ValueFactory
{
public:

    virtual Ice::ValuePtr create(const std::string&);

    void setDelegate(const Ice::ValueFactoryPtr&);
    Ice::ValueFactoryPtr getDelegate() const { return _delegate; }

    VALUE getObject() const;

    void mark();

    void destroy();

private:

    Ice::ValueFactoryPtr _delegate;
};
typedef IceUtil::Handle<DefaultValueFactory> DefaultValueFactoryPtr;

class ValueFactoryManager : public Ice::ValueFactoryManager, public IceUtil::Mutex
{
public:

    ValueFactoryManager();
    ~ValueFactoryManager();

    virtual void add(const Ice::ValueFactoryPtr&, const std::string&);
    virtual Ice::ValueFactoryPtr find(const std::string&) const;

    void addValueFactory(VALUE, const std::string&);
    VALUE findValueFactory(const std::string&) const;
    void addObjectFactory(VALUE, const std::string&);
    VALUE findObjectFactory(const std::string&) const;

    void mark();
    void markSelf();

    VALUE getObject() const;

    void destroy();

private:

    typedef std::map<std::string, Ice::ValueFactoryPtr> FactoryMap;

    VALUE _self;
    FactoryMap _factories;
    DefaultValueFactoryPtr _defaultFactory;
};
typedef IceUtil::Handle<ValueFactoryManager> ValueFactoryManagerPtr;

}

#endif
