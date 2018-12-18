// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <ValueFactoryManager.h>
#include <Types.h>
#include <Util.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace IceRuby;

static VALUE _valueFactoryManagerClass;

namespace
{

ClassInfoPtr
getClassInfo(const string& id)
{
    ClassInfoPtr info;

    if(id == Ice::Object::ice_staticId())
    {
        //
        // When the ID is that of Ice::Object, it indicates that the stream has not
        // found a factory and is providing us an opportunity to preserve the object.
        //
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

IceRuby::ValueFactoryManager::ValueFactoryManager()
{
    //
    // Create a Ruby wrapper around this object. Note that this is a cyclic reference.
    //
    _self = Data_Wrap_Struct(_valueFactoryManagerClass, IceRuby_ValueFactoryManager_mark,
                             IceRuby_ValueFactoryManager_free, new ValueFactoryManagerPtr(this));

    _defaultFactory = new DefaultValueFactory;
}

IceRuby::ValueFactoryManager::~ValueFactoryManager()
{
    assert(_factories.empty());
}

void
IceRuby::ValueFactoryManager::add(const Ice::ValueFactoryPtr& f, const string& id)
{
    Lock lock(*this);

    if(id.empty())
    {
        if(_defaultFactory->getDelegate())
        {
            throw Ice::AlreadyRegisteredException(__FILE__, __LINE__, "value factory", id);
        }

        _defaultFactory->setDelegate(f);
    }
    else
    {
        FactoryMap::iterator p = _factories.find(id);
        if(p != _factories.end())
        {
            throw Ice::AlreadyRegisteredException(__FILE__, __LINE__, "value factory", id);
        }

        _factories.insert(FactoryMap::value_type(id, f));
    }
}

Ice::ValueFactoryPtr
IceRuby::ValueFactoryManager::find(const string& id) const ICE_NOEXCEPT
{
    Lock lock(*this);

    if(id.empty())
    {
        return _defaultFactory;
    }

    FactoryMap::const_iterator p = _factories.find(id);
    if(p != _factories.end())
    {
        return p->second;
    }

    return 0;
}

void
IceRuby::ValueFactoryManager::addValueFactory(VALUE f, const string& id)
{
    ICE_RUBY_TRY
    {
        add(new FactoryWrapper(f, false), id);
    }
    ICE_RUBY_CATCH
}

VALUE
IceRuby::ValueFactoryManager::findValueFactory(const string& id) const
{
    Ice::ValueFactoryPtr f = find(id);
    if(f)
    {
        FactoryWrapperPtr w = FactoryWrapperPtr::dynamicCast(f);
        if(w)
        {
            return w->getObject();
        }
    }

    return Qnil;
}

void
IceRuby::ValueFactoryManager::addObjectFactory(VALUE f, const string& id)
{
    ICE_RUBY_TRY
    {
        add(new FactoryWrapper(f, true), id);
    }
    ICE_RUBY_CATCH
}

VALUE
IceRuby::ValueFactoryManager::findObjectFactory(const string& id) const
{
    Ice::ValueFactoryPtr f = find(id);
    if(f)
    {
        FactoryWrapperPtr w = FactoryWrapperPtr::dynamicCast(f);
        if(w && w->isObjectFactory())
        {
            return w->getObject();
        }
    }

    return Qnil;
}

void
IceRuby::ValueFactoryManager::mark()
{
    Lock lock(*this);

    for(FactoryMap::iterator p = _factories.begin(); p != _factories.end(); ++p)
    {
        FactoryWrapperPtr w = FactoryWrapperPtr::dynamicCast(p->second);
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
        Lock lock(*this);

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
        Lock lock(*this);
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
        FactoryWrapperPtr w = FactoryWrapperPtr::dynamicCast(p->second);
        if(w)
        {
            w->destroy();
        }
    }

    _defaultFactory->destroy();
}

IceRuby::FactoryWrapper::FactoryWrapper(VALUE factory, bool isObjectFactory) :
    _factory(factory),
    _isObjectFactory(isObjectFactory)
{
}

Ice::ValuePtr
IceRuby::FactoryWrapper::create(const string& id)
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

    return new ObjectReader(obj, info);
}

VALUE
IceRuby::FactoryWrapper::getObject() const
{
    return _factory;
}

bool
IceRuby::FactoryWrapper::isObjectFactory() const
{
    return _isObjectFactory;
}

void
IceRuby::FactoryWrapper::mark()
{
    rb_gc_mark(_factory);
}

void
IceRuby::FactoryWrapper::destroy()
{
    if(_isObjectFactory)
    {
        callRuby(rb_funcall, _factory, rb_intern("destroy"), 0);
    }
}

Ice::ValuePtr
IceRuby::DefaultValueFactory::create(const string& id)
{
    Ice::ValuePtr v;

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
    return new ObjectReader(obj, info);
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
        FactoryWrapperPtr w = FactoryWrapperPtr::dynamicCast(_delegate);
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
        FactoryWrapperPtr w = FactoryWrapperPtr::dynamicCast(_delegate);
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
        FactoryWrapperPtr w = FactoryWrapperPtr::dynamicCast(_delegate);
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

    //
    // Instance methods.
    //
    rb_define_method(_valueFactoryManagerClass, "add", CAST_METHOD(IceRuby_ValueFactoryManager_add), 2);
    rb_define_method(_valueFactoryManagerClass, "find", CAST_METHOD(IceRuby_ValueFactoryManager_find), 1);

    return true;
}
