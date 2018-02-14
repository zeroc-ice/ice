// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
    assert(_factoryMap.empty());
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
    FactoryMap::iterator p = _factoryMap.find(id);
    if(p != _factoryMap.end())
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

    for(FactoryMap::iterator p = _factoryMap.begin(); p != _factoryMap.end(); ++p)
    {
        //
        // Invoke the destroy method on each registered Ruby factory.
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
    _factoryMap.clear();
}

void
IceRuby::ObjectFactory::add(VALUE factory, const string& id)
{
    Lock sync(*this);

    FactoryMap::iterator p = _factoryMap.find(id);
    if(p != _factoryMap.end())
    {
        Ice::AlreadyRegisteredException ex(__FILE__, __LINE__);
        ex.kindOfObject = "object factory";
        ex.id = id;
        throw ex;
    }

    _factoryMap.insert(FactoryMap::value_type(id, factory));
}

VALUE
IceRuby::ObjectFactory::find(const string& id)
{
    Lock sync(*this);

    FactoryMap::iterator p = _factoryMap.find(id);
    if(p == _factoryMap.end())
    {
        return Qnil;
    }

    return p->second;
}

void
IceRuby::ObjectFactory::mark()
{
    for(FactoryMap::iterator p = _factoryMap.begin(); p != _factoryMap.end(); ++p)
    {
        rb_gc_mark(p->second);
    }
}
