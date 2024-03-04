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

ClassInfoPtr
getClassInfo(string_view id)
{
    ClassInfoPtr info;

    if(id == Ice::Value::ice_staticId())
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

extern "C"
void
IceRuby_ValueFactoryManager_mark(ValueFactoryManagerPtr* p)
{
    assert(p);
    (*p)->mark();
}

extern "C"
void
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
    vfm->_self = Data_Wrap_Struct(_valueFactoryManagerClass, IceRuby_ValueFactoryManager_mark,
                             IceRuby_ValueFactoryManager_free, new ValueFactoryManagerPtr(vfm));

    return vfm;
}

IceRuby::ValueFactoryManager::ValueFactoryManager()
{
    _defaultFactory = make_shared<DefaultValueFactory>();
}

IceRuby::ValueFactoryManager::~ValueFactoryManager()
{
    assert(_factories.empty());
}

void
IceRuby::ValueFactoryManager::add(Ice::ValueFactoryFunc, string_view)
{
    throw Ice::FeatureNotSupportedException(__FILE__, __LINE__);
}

void
IceRuby::ValueFactoryManager::add(Ice::ValueFactoryPtr f, string_view id)
{
    std::lock_guard lock(_mutex);

    if(id.empty())
    {
        if(_defaultFactory->getDelegate())
        {
            throw Ice::AlreadyRegisteredException(__FILE__, __LINE__, "value factory", string{id});
        }

        _defaultFactory->setDelegate(f);
    }
    else
    {
        FactoryMap::iterator p = _factories.find(id);
        if(p != _factories.end())
        {
            throw Ice::AlreadyRegisteredException(__FILE__, __LINE__, "value factory", string{id});
        }

        _factories.insert(FactoryMap::value_type(string{id}, f));
    }
}

Ice::ValueFactoryFunc
IceRuby::ValueFactoryManager::find(string_view id) const noexcept
{
    Ice::ValueFactoryPtr factory = findCore(id);

    if (factory)
    {
        return [factory](string_view type) { return factory->create(type); };
    }
    else
    {
        return nullptr;
    }
}

Ice::ValueFactoryPtr
IceRuby::ValueFactoryManager::findCore(string_view id) const noexcept
{
    std::lock_guard lock(_mutex);

    if(id.empty())
    {
        return _defaultFactory;
    }
    else
    {
        FactoryMap::const_iterator p = _factories.find(id);
        if(p != _factories.end())
        {
            return p->second;
        }
    }

    return nullptr;
}

void
IceRuby::ValueFactoryManager::addValueFactory(VALUE f, string_view id)
{
    ICE_RUBY_TRY
    {
        add(make_shared<FactoryWrapper>(f), id);
    }
    ICE_RUBY_CATCH
}

VALUE
IceRuby::ValueFactoryManager::findValueFactory(string_view id) const
{
    Ice::ValueFactoryPtr f = findCore(id);
    if(f)
    {
        auto w = dynamic_pointer_cast<FactoryWrapper>(f);
        if(w)
        {
            return w->getObject();
        }
    }

    return Qnil;
}

void
IceRuby::ValueFactoryManager::mark()
{
    std::lock_guard lock(_mutex);

    for(FactoryMap::iterator p = _factories.begin(); p != _factories.end(); ++p)
    {
        auto w = dynamic_pointer_cast<FactoryWrapper>(p->second);
        if(w)
        {
            w->mark();
        }
    }

    _defaultFactory->mark();
}

void
IceRuby::ValueFactoryManager::markSelf()
{
    volatile VALUE self;

    {
        std::lock_guard lock(_mutex);

        self = _self;
    }

    if(!NIL_P(self))
    {
        rb_gc_mark(self);
    }
}

VALUE
IceRuby::ValueFactoryManager::getObject() const
{
    return _self;
}

void
IceRuby::ValueFactoryManager::destroy()
{
    FactoryMap factories;

    {
        std::lock_guard lock(_mutex);
        if(_self == Qnil)
        {
            //
            // Nothing to do if already destroyed (this can occur if communicator destroy is called multiple times)
            //
            return;
        }

        factories.swap(_factories);

        _self = Qnil;
    }

    for(FactoryMap::iterator p = factories.begin(); p != factories.end(); ++p)
    {
        auto w = dynamic_pointer_cast<FactoryWrapper>(p->second);
        if(w)
        {
            w->destroy();
        }
    }

    _defaultFactory->destroy();
}

IceRuby::FactoryWrapper::FactoryWrapper(VALUE factory) :
    _factory(factory)
{
}

shared_ptr<Ice::Value>
IceRuby::FactoryWrapper::create(string_view id)
{
    //
    // Get the type information.
    //
    ClassInfoPtr info = getClassInfo(id);

    if(!info)
    {
        return 0;
    }

    //
    // Invoke the create method on the Ruby factory object.
    //
    volatile VALUE str = createString(id);
    volatile VALUE obj = callRuby(rb_funcall, _factory, rb_intern("create"), 1, str);
    if(NIL_P(obj))
    {
        return 0;
    }

    return make_shared<ValueReader>(obj, info);
}

VALUE
IceRuby::FactoryWrapper::getObject() const
{
    return _factory;
}

void
IceRuby::FactoryWrapper::mark()
{
    rb_gc_mark(_factory);
}

void
IceRuby::FactoryWrapper::destroy()
{
}

shared_ptr<Ice::Value>
IceRuby::DefaultValueFactory::create(string_view id)
{
    shared_ptr<Ice::Value> v;

    //
    // Give the application-provided default factory a chance to create the object first.
    //
    if(_delegate)
    {
        v = _delegate->create(id);
        if(v)
        {
            return v;
        }
    }

    //
    // Get the type information.
    //
    ClassInfoPtr info = getClassInfo(id);

    if(!info)
    {
        return 0;
    }

    //
    // NOTE: We don't do this in Ruby because a generated class can be re-opened to define operations.
    //
    ////
    //// If the requested type is an abstract class, then we give up.
    ////
    //if(info->isAbstract)
    //{
    //    return 0;
    //}

    //
    // Instantiate the object.
    //
    volatile VALUE obj = callRuby(rb_class_new_instance, 0, reinterpret_cast<VALUE*>(0), info->rubyClass);
    assert(!NIL_P(obj));

    return make_shared<ValueReader>(obj, info);
}

void
IceRuby::DefaultValueFactory::setDelegate(const Ice::ValueFactoryPtr& d)
{
    _delegate = d;
}

VALUE
IceRuby::DefaultValueFactory::getObject() const
{
    if(_delegate)
    {
        auto w = dynamic_pointer_cast<FactoryWrapper>(_delegate);
        if(w)
        {
            return w->getObject();
        }
    }

    return Qnil;
}

void
IceRuby::DefaultValueFactory::mark()
{
    if(_delegate)
    {
        auto w = dynamic_pointer_cast<FactoryWrapper>(_delegate);
        if(w)
        {
            w->mark();
        }
    }
}

void
IceRuby::DefaultValueFactory::destroy()
{
    if(_delegate)
    {
        auto w = dynamic_pointer_cast<FactoryWrapper>(_delegate);
        if(w)
        {
            w->destroy();
        }
    }

    _delegate = 0;
}

extern "C"
VALUE
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

extern "C"
VALUE
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
