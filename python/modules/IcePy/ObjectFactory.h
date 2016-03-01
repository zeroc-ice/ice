// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPY_OBJECT_FACTORY_H
#define ICEPY_OBJECT_FACTORY_H

#include <Config.h>
#include <Ice/ObjectF.h>
#include <Ice/ObjectFactory.h>
#include <IceUtil/Mutex.h>

namespace IcePy
{

//
// Each communicator registers an instance of ObjectFactory as its
// default object factory. This instance delegates to registered Python
// objects, and instantiates concrete classes when no factory is present.
//
class ObjectFactory : public Ice::ObjectFactory, public IceUtil::Mutex
{
public:

    ObjectFactory();
    ~ObjectFactory();

    virtual Ice::ObjectPtr create(const std::string&);

    virtual void destroy();

    bool add(PyObject*, const std::string&);
    bool remove(const std::string&);
    PyObject* find(const std::string&);

private:

    typedef std::map<std::string, PyObject*> FactoryMap;
    FactoryMap _factoryMap;
};
typedef IceUtil::Handle<ObjectFactory> ObjectFactoryPtr;

}

#endif
