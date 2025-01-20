// Copyright (c) ZeroC, Inc.

#include "Logger.h"
#include "Ice/Initialize.h"
#include "Util.h"

using namespace std;
using namespace IceRuby;

static VALUE _loggerClass;

extern "C" void
IceRuby_Logger_free(void* p)
{
    delete static_cast<Ice::LoggerPtr*>(p);
}

static const rb_data_type_t IceRuby_LoggerType = {
    .wrap_struct_name = "Ice::Logger",
    .function =
        {
            .dfree = IceRuby_Logger_free,
        },
    .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

VALUE
IceRuby::createLogger(const Ice::LoggerPtr& p)
{
    return TypedData_Wrap_Struct(_loggerClass, &IceRuby_LoggerType, new Ice::LoggerPtr(p));
}

extern "C" VALUE
IceRuby_Logger_print(VALUE self, VALUE message)
{
    ICE_RUBY_TRY
    {
        Ice::LoggerPtr* p = reinterpret_cast<Ice::LoggerPtr*>(DATA_PTR(self));
        assert(p);

        string msg = getString(message);
        (*p)->print(msg);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_Logger_trace(VALUE self, VALUE category, VALUE message)
{
    ICE_RUBY_TRY
    {
        Ice::LoggerPtr* p = reinterpret_cast<Ice::LoggerPtr*>(DATA_PTR(self));
        assert(p);

        string cat = getString(category);
        string msg = getString(message);
        (*p)->trace(cat, msg);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_Logger_warning(VALUE self, VALUE message)
{
    ICE_RUBY_TRY
    {
        Ice::LoggerPtr* p = reinterpret_cast<Ice::LoggerPtr*>(DATA_PTR(self));
        assert(p);

        string msg = getString(message);
        (*p)->warning(msg);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_Logger_error(VALUE self, VALUE message)
{
    ICE_RUBY_TRY
    {
        Ice::LoggerPtr* p = reinterpret_cast<Ice::LoggerPtr*>(DATA_PTR(self));
        assert(p);

        string msg = getString(message);
        (*p)->error(msg);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_Logger_cloneWithPrefix(VALUE self, VALUE prefix)
{
    ICE_RUBY_TRY
    {
        Ice::LoggerPtr* p = reinterpret_cast<Ice::LoggerPtr*>(DATA_PTR(self));
        assert(p);

        string pfx = getString(prefix);
        Ice::LoggerPtr clone = (*p)->cloneWithPrefix(pfx);
        return createLogger(clone);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_getProcessLogger()
{
    ICE_RUBY_TRY
    {
        Ice::LoggerPtr logger = Ice::getProcessLogger();
        return createLogger(logger);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

bool
IceRuby::initLogger(VALUE iceModule)
{
    //
    // Logger.
    //
    _loggerClass = rb_define_class_under(iceModule, "LoggerI", rb_cObject);
    rb_undef_alloc_func(_loggerClass);

    //
    // Instance methods.
    //
    rb_define_method(_loggerClass, "print", CAST_METHOD(IceRuby_Logger_print), 1);
    rb_define_method(_loggerClass, "trace", CAST_METHOD(IceRuby_Logger_trace), 2);
    rb_define_method(_loggerClass, "warning", CAST_METHOD(IceRuby_Logger_warning), 1);
    rb_define_method(_loggerClass, "error", CAST_METHOD(IceRuby_Logger_error), 1);
    rb_define_method(_loggerClass, "cloneWithPrefix", CAST_METHOD(IceRuby_Logger_cloneWithPrefix), 1);

    //
    // Global methods.
    //
    rb_define_module_function(iceModule, "getProcessLogger", CAST_METHOD(IceRuby_getProcessLogger), 0);

    return true;
}
