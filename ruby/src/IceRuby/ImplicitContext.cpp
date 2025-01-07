//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "ImplicitContext.h"
#include "Ice/ImplicitContext.h"
#include "Ice/Initialize.h"
#include "Util.h"

using namespace std;
using namespace IceRuby;

static VALUE _implicitContextClass;

extern "C" void
IceRuby_ImplicitContext_free(void* p)
{
    delete static_cast<Ice::ImplicitContextPtr*>(p);
}

static const rb_data_type_t IceRuby_ImplicitContextType = {
    .wrap_struct_name = "Ice::ImplicitContext",
    .function =
        {
            .dfree = IceRuby_ImplicitContext_free,

        },
    .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

extern "C" VALUE
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

extern "C" VALUE
IceRuby_ImplicitContext_setContext(VALUE self, VALUE context)
{
    ICE_RUBY_TRY
    {
        Ice::Context ctx;
        if (!hashToContext(context, ctx))
        {
            throw RubyException(rb_eTypeError, "argument must be a hash");
        }
        Ice::ImplicitContextPtr p = getImplicitContext(self);
        p->setContext(ctx);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_ImplicitContext_containsKey(VALUE self, VALUE key)
{
    ICE_RUBY_TRY
    {
        Ice::ImplicitContextPtr p = getImplicitContext(self);
        string k = getString(key);
        if (p->containsKey(k))
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

extern "C" VALUE
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

extern "C" VALUE
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

extern "C" VALUE
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
    rb_undef_alloc_func(_implicitContextClass);
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
    return TypedData_Wrap_Struct(_implicitContextClass, &IceRuby_ImplicitContextType, new Ice::ImplicitContextPtr(p));
}
