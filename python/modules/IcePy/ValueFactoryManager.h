// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPY_VALUE_FACTORY_MANAGER_H
#define ICEPY_VALUE_FACTORY_MANAGER_H

#include <Config.h>
#include <Ice/ValueFactory.h>
#include <IceUtil/Mutex.h>

namespace IcePy
{

extern PyTypeObject ValueFactoryManagerType;

bool initValueFactoryManager(PyObject*);

class FactoryWrapper : public Ice::ValueFactory
{
public:

    FactoryWrapper(PyObject*, PyObject*);
    ~FactoryWrapper();

    virtual Ice::ValuePtr create(const std::string&);

    PyObject* getValueFactory() const;
    PyObject* getObjectFactory() const;

    void destroy();

protected:

    PyObject* _valueFactory;
    PyObject* _objectFactory;
};
typedef IceUtil::Handle<FactoryWrapper> FactoryWrapperPtr;

class DefaultValueFactory : public Ice::ValueFactory
{
public:

    virtual Ice::ValuePtr create(const std::string&);

    void setDelegate(const Ice::ValueFactoryPtr&);
    Ice::ValueFactoryPtr getDelegate() const { return _delegate; }

    PyObject* getValueFactory() const;
    PyObject* getObjectFactory() const;

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
    virtual Ice::ValueFactoryPtr find(const std::string&) const ICE_NOEXCEPT;

    virtual void add(PyObject*, PyObject*, const std::string&);
    PyObject* findValueFactory(const std::string&) const;
    PyObject* findObjectFactory(const std::string&) const;

    PyObject* getObject() const;

    void destroy();

private:

    typedef std::map<std::string, Ice::ValueFactoryPtr> FactoryMap;

    PyObject* _self;
    FactoryMap _factories;
    DefaultValueFactoryPtr _defaultFactory;
};
typedef IceUtil::Handle<ValueFactoryManager> ValueFactoryManagerPtr;

}

#endif
