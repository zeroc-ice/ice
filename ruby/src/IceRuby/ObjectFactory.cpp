// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <ObjectFactory.h>
#include <Types.h>
#include <Util.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace IceRuby;

IceRuby::ObjectFactory::ObjectFactory()
{
}

IceRuby::ObjectFactory::~ObjectFactory()
{
    assert(_valueFactoryMap.empty());
    assert(_objectFactoryMap.empty());
}

Ice::ObjectPtr
IceRuby::ObjectFactory::create(const string& id)
{
    Lock sync(*this);

    //
    // Get the type information.
    //
    ClassInfoPtr info;
    if(id == Ice::Object::ice_staticId())
    {
        //
        // When the ID is that of Ice::Object, it indicates that the stream has not
        // found a factory and is providing us an opportunity to preserve the object.
        //
        info = lookupClassInfo("::Ice::UnknownSlicedObject");
    }
    else
    {
        info = lookupClassInfo(id);
    }

    if(!info)
    {
        return 0;
    }

    //
    // Check if the application has registered a factory for this id.
    //
    FactoryMap::iterator p = _valueFactoryMap.find(id);
    if(p != _valueFactoryMap.end())
    {
        //
        // Invoke the create method on the Ruby factory object.
        //
        volatile VALUE str = createString(id);
        volatile VALUE obj = callRuby(rb_funcall, p->second, rb_intern("create"), 1, str);
        if(NIL_P(obj))
        {
            return 0;
        }
        return new ObjectReader(obj, info);
    }

    //
    // Instantiate the object.
    //
    volatile VALUE obj = callRuby(rb_class_new_instance, 0, reinterpret_cast<VALUE*>(0), info->rubyClass);
    assert(!NIL_P(obj));
    return new ObjectReader(obj, info);
}

void
IceRuby::ObjectFactory::destroy()
{
    Lock sync(*this);

    for(FactoryMap::iterator p = _objectFactoryMap.begin(); p != _objectFactoryMap.end(); ++p)
    {
        //
        // Invoke the destroy method on each registered Ruby object factory.
        //
        try
        {
            callRuby(rb_funcall, p->second, rb_intern("destroy"), 0);
        }
        catch(const RubyException&)
        {
            // Ignore.
        }
    }

    _valueFactoryMap.clear();
    _objectFactoryMap.clear();
}

void
IceRuby::ObjectFactory::addValueFactory(VALUE factory, const string& id)
{
    Lock sync(*this);

    FactoryMap::iterator p = _valueFactoryMap.find(id);
    if(p != _valueFactoryMap.end())
    {
        Ice::AlreadyRegisteredException ex(__FILE__, __LINE__);
        ex.kindOfObject = "value factory";
        ex.id = id;
        throw ex;
    }

    _valueFactoryMap.insert(FactoryMap::value_type(id, factory));
}

void
IceRuby::ObjectFactory::addObjectFactory(VALUE factory, const string& id)
{
    Lock sync(*this);

    FactoryMap::iterator p = _valueFactoryMap.find(id);
    if(p != _valueFactoryMap.end())
    {
        Ice::AlreadyRegisteredException ex(__FILE__, __LINE__);
        ex.kindOfObject = "value factory";
        ex.id = id;
        throw ex;
    }

    _valueFactoryMap.insert(FactoryMap::value_type(id, factory));
    _objectFactoryMap.insert(FactoryMap::value_type(id, factory));
}

VALUE
IceRuby::ObjectFactory::findValueFactory(const string& id)
{
    Lock sync(*this);

    FactoryMap::iterator p = _valueFactoryMap.find(id);
    if(p == _valueFactoryMap.end())
    {
        return Qnil;
    }

    return p->second;
}

VALUE
IceRuby::ObjectFactory::findObjectFactory(const string& id)
{
    Lock sync(*this);

    FactoryMap::iterator p = _objectFactoryMap.find(id);
    if(p == _objectFactoryMap.end())
    {
        return Qnil;
    }

    return p->second;
}

void
IceRuby::ObjectFactory::mark()
{
    for(FactoryMap::iterator p = _valueFactoryMap.begin(); p != _valueFactoryMap.end(); ++p)
    {
        rb_gc_mark(p->second);
    }
    for(FactoryMap::iterator p = _objectFactoryMap.begin(); p != _objectFactoryMap.end(); ++p)
    {
        rb_gc_mark(p->second);
    }
}
