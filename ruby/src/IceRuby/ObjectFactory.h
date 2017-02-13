// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_RUBY_OBJECT_FACTORY_H
#define ICE_RUBY_OBJECT_FACTORY_H

#include <Config.h>
#include <Ice/ObjectF.h>
#include <Ice/ObjectFactory.h>
#include <IceUtil/Mutex.h>

namespace IceRuby
{

//
// Each communicator registers an instance of ObjectFactory as its
// default object factory. This instance delegates to registered Ruby
// objects, and instantiates concrete classes when no factory is present.
//
class ObjectFactory : public Ice::ObjectFactory, public IceUtil::Mutex
{
public:

    ObjectFactory();
    ~ObjectFactory();

    virtual Ice::ObjectPtr create(const std::string&);

    virtual void destroy();

    void add(VALUE, const std::string&);
    VALUE find(const std::string&);
    void mark();

private:

    typedef std::map<std::string, VALUE> FactoryMap;
    FactoryMap _factoryMap;
};
typedef IceUtil::Handle<ObjectFactory> ObjectFactoryPtr;

}

#endif
