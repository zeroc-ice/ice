// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <ImplicitContext.h>
#include <Util.h>
#include <Ice/Initialize.h>
#include <Ice/ImplicitContext.h>

using namespace std;
using namespace IceRuby;

static VALUE _implicitContextClass;

extern "C"
void
IceRuby_ImplicitContext_free(Ice::ImplicitContextPtr* p)
{
    assert(p);
    delete p;
}

extern "C"
VALUE
IceRuby_ImplicitContext_getContext(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ImplicitContextPtr p = getImplicitContext(self);
        return contextToHash(p->getContext());
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ImplicitContext_setContext(VALUE self, VALUE context)
{
    ICE_RUBY_TRY
    {
        Ice::Context ctx;
        if(!hashToContext(context, ctx))
        {
            throw RubyException(rb_eTypeError, "argument must be a hash");
        }
        Ice::ImplicitContextPtr p = getImplicitContext(self);
        p->setContext(ctx);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ImplicitContext_containsKey(VALUE self, VALUE key)
{
    ICE_RUBY_TRY
    {
        Ice::ImplicitContextPtr p = getImplicitContext(self);
        string k = getString(key);
        if(p->containsKey(k))
        {
            return Qtrue;
        }
        else
        {
            return Qfalse;
        }
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ImplicitContext_get(VALUE self, VALUE key)
{
    ICE_RUBY_TRY
    {
        Ice::ImplicitContextPtr p = getImplicitContext(self);
        string k = getString(key);
        string v = p->get(k);
        return createString(v);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ImplicitContext_put(VALUE self, VALUE key, VALUE value)
{
    ICE_RUBY_TRY
    {
        Ice::ImplicitContextPtr p = getImplicitContext(self);
        string k = getString(key);
        string v = getString(value);
        return createString(p->put(k, v));
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ImplicitContext_remove(VALUE self, VALUE key)
{
    ICE_RUBY_TRY
    {
        Ice::ImplicitContextPtr p = getImplicitContext(self);
        string k = getString(key);
        return createString(p->remove(k));
    }
    ICE_RUBY_CATCH
    return Qnil;
}

void
IceRuby::initImplicitContext(VALUE iceModule)
{
    _implicitContextClass = rb_define_class_under(iceModule, "ImplicitContextI", rb_cObject);
    rb_define_method(_implicitContextClass, "getContext", CAST_METHOD(IceRuby_ImplicitContext_getContext), 0);
    rb_define_method(_implicitContextClass, "setContext", CAST_METHOD(IceRuby_ImplicitContext_setContext), 1);
    rb_define_method(_implicitContextClass, "containsKey", CAST_METHOD(IceRuby_ImplicitContext_containsKey), 1);
    rb_define_method(_implicitContextClass, "get", CAST_METHOD(IceRuby_ImplicitContext_get), 1);
    rb_define_method(_implicitContextClass, "put", CAST_METHOD(IceRuby_ImplicitContext_put), 2);
    rb_define_method(_implicitContextClass, "remove", CAST_METHOD(IceRuby_ImplicitContext_remove), 1);
}

Ice::ImplicitContextPtr
IceRuby::getImplicitContext(VALUE v)
{
    Ice::ImplicitContextPtr* p = reinterpret_cast<Ice::ImplicitContextPtr*>(DATA_PTR(v));
    assert(p);
    return *p;
}

VALUE
IceRuby::createImplicitContext(const Ice::ImplicitContextPtr& p)
{
    return Data_Wrap_Struct(_implicitContextClass, 0, IceRuby_ImplicitContext_free, new Ice::ImplicitContextPtr(p));
}
