//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEPY_VALUE_FACTORY_MANAGER_H
#define ICEPY_VALUE_FACTORY_MANAGER_H

#include <Config.h>
#include <Ice/ValueFactory.h>

#include <mutex>

namespace IcePy
{

extern PyTypeObject ValueFactoryManagerType;

bool initValueFactoryManager(PyObject*);

class FactoryWrapper : public Ice::ValueFactory
{
public:

    FactoryWrapper(PyObject*, PyObject*);
    ~FactoryWrapper();

    virtual std::shared_ptr<Ice::Value> create(const std::string&);

    PyObject* getValueFactory() const;
    void destroy();

protected:

    PyObject* _valueFactory;
    PyObject* _objectFactory;
};

using FactoryWrapperPtr = std::shared_ptr<FactoryWrapper>;

class DefaultValueFactory : public Ice::ValueFactory
{
public:

    virtual std::shared_ptr<Ice::Value> create(const std::string&);

    void setDelegate(const Ice::ValueFactoryPtr&);
    Ice::ValueFactoryPtr getDelegate() const { return _delegate; }

    PyObject* getValueFactory() const;

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

    void add(PyObject*, PyObject*, const std::string&);
    PyObject* findValueFactory(const std::string&) const;

    PyObject* getObject() const;

    void destroy();

private:

    typedef std::map<std::string, Ice::ValueFactoryPtr> FactoryMap;

    ValueFactoryManager();
    Ice::ValueFactoryPtr findCore(const std::string&) const noexcept;

    PyObject* _self;
    FactoryMap _factories;
    DefaultValueFactoryPtr _defaultFactory;

    mutable std::mutex _mutex;
};

using ValueFactoryManagerPtr = std::shared_ptr<ValueFactoryManager>;

}

#endif
