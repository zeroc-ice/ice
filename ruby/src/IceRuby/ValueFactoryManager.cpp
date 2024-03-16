//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "ValueFactoryManager.h"
#include "Types.h"
#include "Util.h"

#include <Ice/LocalException.h>

using namespace std;
using namespace IceRuby;

static VALUE _valueFactoryManagerClass;

namespace
{
    ClassInfoPtr getClassInfo(string_view id)
    {
        ClassInfoPtr info;

        if (id == Ice::Value::ice_staticId())
        {
            // When the ID is that of Ice::Object, it indicates that the stream has not
            // found a factory and is providing us an opportunity to preserve the object.
            info = lookupClassInfo("::Ice::UnknownSlicedValue");
        }
        else
        {
            info = lookupClassInfo(id);
        }

        return info;
    }
}

extern "C" void
IceRuby_ValueFactoryManager_mark(ValueFactoryManagerPtr* p)
{
    assert(p);
    (*p)->mark();
}

extern "C" void
IceRuby_ValueFactoryManager_free(ValueFactoryManagerPtr* p)
{
    assert(p);
    delete p;
}

/* static */ ValueFactoryManagerPtr
IceRuby::ValueFactoryManager::ValueFactoryManager::create()
{
    // can't use make_shared because constructor is private
    auto vfm = shared_ptr<ValueFactoryManager>(new ValueFactoryManager);

    //
    // Create a Ruby wrapper around this object. Note that this is a cyclic reference.
    //
    vfm->_self = Data_Wrap_Struct(
        _valueFactoryManagerClass,
        IceRuby_ValueFactoryManager_mark,
        IceRuby_ValueFactoryManager_free,
        new ValueFactoryManagerPtr(vfm));

    return vfm;
}

IceRuby::ValueFactoryManager::ValueFactoryManager() : _defaultFactory{make_shared<DefaultValueFactory>()} {}

void
IceRuby::ValueFactoryManager::add(Ice::ValueFactoryFunc, string_view)
{
    // This means a C++ plugin cannot register a value factory with a Ruby application/communicator.
    throw Ice::FeatureNotSupportedException(__FILE__, __LINE__);
}

Ice::ValueFactoryFunc
IceRuby::ValueFactoryManager::find(string_view typeId) const noexcept
{
    ValueFactoryPtr factory;
    {
        std::lock_guard lock(_mutex);

        CustomFactoryMap::const_iterator p = _customFactories.find(typeId);
        if (p != _customFactories.end())
        {
            factory = p->second;
        }
        else if (typeId.empty())
        {
            factory = _defaultFactory;
        }
    }

    if (factory)
    {
        return [factory](string_view id) { return factory->create(id); };
    }
    else
    {
        return nullptr;
    }
}

void
IceRuby::ValueFactoryManager::addValueFactory(VALUE valueFactory, string_view id)
{
    ICE_RUBY_TRY
    {
        CustomValueFactoryPtr f = make_shared<CustomValueFactory>(valueFactory);

        std::lock_guard lock(_mutex);
        CustomFactoryMap::iterator p = _customFactories.find(id);
        if (p != _customFactories.end())
        {
            throw Ice::AlreadyRegisteredException(__FILE__, __LINE__, "value factory", string{id});
        }

        _customFactories.insert(CustomFactoryMap::value_type(string{id}, f));
    }
    ICE_RUBY_CATCH
}

VALUE
IceRuby::ValueFactoryManager::findValueFactory(string_view id) const
{
    std::lock_guard lock(_mutex);

    CustomFactoryMap::const_iterator p = _customFactories.find(id);
    if (p != _customFactories.end())
    {
        return p->second->getObject();
    }

    return Qnil;
}

void
IceRuby::ValueFactoryManager::mark()
{
    std::lock_guard lock(_mutex);
    for (const auto& p : _customFactories)
    {
        p.second->mark();
    }
}

void
IceRuby::ValueFactoryManager::markSelf()
{
    volatile VALUE self;

    {
        std::lock_guard lock(_mutex);
        self = _self;
    }

    if (!NIL_P(self))
    {
        rb_gc_mark(self);
    }
}

VALUE
IceRuby::ValueFactoryManager::getObject() const { return _self; }

void
IceRuby::ValueFactoryManager::destroy()
{
    CustomFactoryMap factories;

    {
        std::lock_guard lock(_mutex);
        if (_self == Qnil)
        {
            //
            // Nothing to do if already destroyed (this can occur if communicator destroy is called multiple times)
            //
            return;
        }

        factories.swap(_customFactories);

        _self = Qnil;
    }
}

IceRuby::CustomValueFactory::CustomValueFactory(VALUE factory) : _factory(factory) {}

shared_ptr<Ice::Value>
IceRuby::CustomValueFactory::create(string_view id)
{
    //
    // Get the type information.
    //
    ClassInfoPtr info = getClassInfo(id);

    if (!info)
    {
        return 0;
    }

    //
    // Invoke the create method on the Ruby factory object.
    //
    volatile VALUE str = createString(id);
    volatile VALUE obj = callRuby(rb_funcall, _factory, rb_intern("create"), 1, str);
    if (NIL_P(obj))
    {
        return 0;
    }

    return make_shared<ValueReader>(obj, info);
}

VALUE
IceRuby::CustomValueFactory::getObject() const { return _factory; }

void
IceRuby::CustomValueFactory::mark()
{
    rb_gc_mark(_factory);
}

shared_ptr<Ice::Value>
IceRuby::DefaultValueFactory::create(string_view id)
{
    //
    // Get the type information.
    //
    ClassInfoPtr info = getClassInfo(id);

    if (!info)
    {
        return 0;
    }

    //
    // NOTE: We don't do this in Ruby because a generated class can be re-opened to define operations.
    //
    ////
    //// If the requested type is an abstract class, then we give up.
    ////
    // if(info->isAbstract)
    //{
    //     return 0;
    // }

    //
    // Instantiate the object.
    //
    volatile VALUE obj = callRuby(rb_class_new_instance, 0, reinterpret_cast<VALUE*>(0), info->rubyClass);
    assert(!NIL_P(obj));

    return make_shared<ValueReader>(obj, info);
}

extern "C" VALUE
IceRuby_ValueFactoryManager_add(VALUE self, VALUE factory, VALUE id)
{
    ICE_RUBY_TRY
    {
        ValueFactoryManagerPtr* p = reinterpret_cast<ValueFactoryManagerPtr*>(DATA_PTR(self));
        assert(p);

        string type = getString(id);
        (*p)->addValueFactory(factory, type);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_ValueFactoryManager_find(VALUE self, VALUE id)
{
    ICE_RUBY_TRY
    {
        ValueFactoryManagerPtr* p = reinterpret_cast<ValueFactoryManagerPtr*>(DATA_PTR(self));
        assert(p);

        string type = getString(id);
        return (*p)->findValueFactory(type);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

bool
IceRuby::initValueFactoryManager(VALUE iceModule)
{
    _valueFactoryManagerClass = rb_define_class_under(iceModule, "ValueFactoryManagerI", rb_cObject);
    rb_undef_alloc_func(_valueFactoryManagerClass);

    //
    // Instance methods.
    //
    rb_define_method(_valueFactoryManagerClass, "add", CAST_METHOD(IceRuby_ValueFactoryManager_add), 2);
    rb_define_method(_valueFactoryManagerClass, "find", CAST_METHOD(IceRuby_ValueFactoryManager_find), 1);

    return true;
}
