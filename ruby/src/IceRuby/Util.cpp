// Copyright (c) ZeroC, Inc.

#include "Util.h"
#include "Ice/LocalExceptions.h"
#include "Ice/VersionFunctions.h"
#include <stdarg.h>

using namespace std;
using namespace IceRuby;

namespace
{
    template<typename T> bool setVersion(VALUE p, const T& version)
    {
        volatile VALUE major = callRuby(rb_int2inum, version.major);
        volatile VALUE minor = callRuby(rb_int2inum, version.minor);
        rb_ivar_set(p, rb_intern("@major"), major);
        rb_ivar_set(p, rb_intern("@minor"), minor);

        return true;
    }

    template<typename T> bool getVersion(VALUE p, T& v)
    {
        volatile VALUE major = callRuby(rb_ivar_get, p, rb_intern("@major"));
        volatile VALUE minor = callRuby(rb_ivar_get, p, rb_intern("@minor"));

        long m;

        m = getInteger(major);
        if (m < 0 || m > 255)
        {
            throw RubyException(rb_eTypeError, "version major must be a value between 0 and 255");
            return false;
        }
        v.major = m;

        m = getInteger(minor);
        if (m < 0 || m > 255)
        {
            throw RubyException(rb_eTypeError, "version minor must be a value between 0 and 255");
            return false;
        }
        v.minor = m;

        return true;
    }

    template<typename T> VALUE createVersion(const T& version, const char* type)
    {
        volatile VALUE rbType = callRuby(rb_path2class, type);
        assert(!NIL_P(rbType));

        volatile VALUE obj = callRuby(rb_class_new_instance, 0, static_cast<VALUE*>(0), rbType);

        if (!setVersion<T>(obj, version))
        {
            return Qnil;
        }

        return obj;
    }

    template<typename T> VALUE versionToString(VALUE p, const char* type)
    {
        volatile VALUE rbType = callRuby(rb_path2class, type);
        assert(!NIL_P(rbType));
        if (callRuby(rb_obj_is_instance_of, p, rbType) != Qtrue)
        {
            throw RubyException(rb_eTypeError, "argument is not an instance of %s", type);
        }

        T v;
        if (!getVersion<T>(p, v))
        {
            return Qnil;
        }

        ICE_RUBY_TRY
        {
            string s = IceInternal::versionToString<T>(v);
            return createString(s);
        }
        ICE_RUBY_CATCH
        return Qnil;
    }

    template<typename T> VALUE stringToVersion(VALUE p, const char* type)
    {
        string str = getString(p);

        ICE_RUBY_TRY
        {
            T v = IceInternal::stringToVersion<T>(str);
            return createVersion<T>(v, type);
        }
        ICE_RUBY_CATCH
        return Qnil;
    }

    char Ice_ProtocolVersion[] = "Ice::ProtocolVersion";
    char Ice_EncodingVersion[] = "Ice::EncodingVersion";
}

extern "C" VALUE
IceRuby_stringVersion(int /*argc*/, VALUE* /*argv*/, VALUE /*self*/)
{
    ICE_RUBY_TRY
    {
        string s = ICE_STRING_VERSION;
        return createString(s);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_intVersion(int /*argc*/, VALUE* /*argv*/, VALUE /*self*/)
{
    ICE_RUBY_TRY { return INT2FIX(ICE_INT_VERSION); }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_currentProtocol(int /*argc*/, VALUE* /*argv*/, VALUE /*self*/)
{
    ICE_RUBY_TRY { return createProtocolVersion(Ice::currentProtocol); }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_currentProtocolEncoding(int /*argc*/, VALUE* /*argv*/, VALUE /*self*/)
{
    ICE_RUBY_TRY { return createEncodingVersion(Ice::currentProtocolEncoding); }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_currentEncoding(int /*argc*/, VALUE* /*argv*/, VALUE /*self*/)
{
    ICE_RUBY_TRY { return createEncodingVersion(Ice::currentEncoding); }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_protocolVersionToString(VALUE /*self*/, VALUE v)
{
    return versionToString<Ice::ProtocolVersion>(v, Ice_ProtocolVersion);
}

extern "C" VALUE
IceRuby_stringToProtocolVersion(VALUE /*self*/, VALUE v)
{
    return stringToVersion<Ice::ProtocolVersion>(v, Ice_ProtocolVersion);
}

extern "C" VALUE
IceRuby_encodingVersionToString(VALUE /*self*/, VALUE v)
{
    return versionToString<Ice::EncodingVersion>(v, Ice_EncodingVersion);
}

extern "C" VALUE
IceRuby_stringToEncodingVersion(VALUE /*self*/, VALUE v)
{
    return stringToVersion<Ice::EncodingVersion>(v, Ice_EncodingVersion);
}

void
IceRuby::initUtil(VALUE iceModule)
{
    rb_define_module_function(iceModule, "stringVersion", CAST_METHOD(IceRuby_stringVersion), -1);
    rb_define_module_function(iceModule, "intVersion", CAST_METHOD(IceRuby_intVersion), -1);
    rb_define_module_function(iceModule, "currentProtocol", CAST_METHOD(IceRuby_currentProtocol), -1);
    rb_define_module_function(iceModule, "currentProtocolEncoding", CAST_METHOD(IceRuby_currentProtocolEncoding), -1);
    rb_define_module_function(iceModule, "currentEncoding", CAST_METHOD(IceRuby_currentEncoding), -1);
    rb_define_module_function(iceModule, "protocolVersionToString", CAST_METHOD(IceRuby_protocolVersionToString), 1);
    rb_define_module_function(iceModule, "stringToProtocolVersion", CAST_METHOD(IceRuby_stringToProtocolVersion), 1);
    rb_define_module_function(iceModule, "encodingVersionToString", CAST_METHOD(IceRuby_encodingVersionToString), 1);
    rb_define_module_function(iceModule, "stringToEncodingVersion", CAST_METHOD(IceRuby_stringToEncodingVersion), 1);
}

IceRuby::RubyException::RubyException() { ex = rb_gv_get("$!"); }

IceRuby::RubyException::RubyException(VALUE exv) : ex(exv) {}

IceRuby::RubyException::RubyException(VALUE exClass, const char* fmt, ...)
{
    va_list args;
    char buf[BUFSIZ];

    va_start(args, fmt);
    vsnprintf(buf, BUFSIZ, fmt, args);
    buf[BUFSIZ - 1] = '\0';
    va_end(args);

    ex = callRuby(rb_exc_new2, exClass, buf);
}

ostream&
IceRuby::RubyException::operator<<(ostream& ostr) const
{
    volatile VALUE cls = rb_class_path(CLASS_OF(ex));
    volatile VALUE msg = rb_obj_as_string(ex);
    ostr << string_view{RSTRING_PTR(cls), static_cast<size_t>(RSTRING_LEN(cls))} << ": "
         << string_view{RSTRING_PTR(msg), static_cast<size_t>(RSTRING_LEN(msg))};
    return ostr;
}

bool
IceRuby::isString(VALUE val)
{
    return TYPE(val) == T_STRING || callRuby(rb_respond_to, val, rb_intern("to_str")) != 0;
}

bool
IceRuby::isArray(VALUE val)
{
    return TYPE(val) == T_ARRAY || callRuby(rb_respond_to, val, rb_intern("to_arr")) != 0;
}

bool
IceRuby::isHash(VALUE val)
{
    return TYPE(val) == T_HASH || callRuby(rb_respond_to, val, rb_intern("to_hash")) != 0;
}

string
IceRuby::getString(VALUE val)
{
    volatile VALUE result = callRuby(rb_string_value, &val);
    return string(RSTRING_PTR(result), RSTRING_LEN(result));
}

VALUE
IceRuby::createString(string_view str)
{
    return callRuby(rb_enc_str_new, str.data(), static_cast<long>(str.size()), rb_utf8_encoding());
}

namespace
{
    template<typename T> struct RubyCallArgs
    {
        volatile VALUE val;
        T ret;
    };

    //
    // Wrapper function to call rb_num2long with rb_protect
    //
    VALUE
    rb_num2long_wrapper(VALUE val)
    {
        RubyCallArgs<long>* data = (RubyCallArgs<long>*)val;
        data->ret = rb_num2long(data->val);
        return val;
    }

    //
    // Wrapper function to call rb_num2ll with rb_protect
    //
    VALUE
    rb_num2ll_wrapper(VALUE val)
    {
        RubyCallArgs<int64_t>* data = (RubyCallArgs<int64_t>*)val;
        data->ret = rb_num2ll(data->val);
        return val;
    }
}

long
IceRuby::getInteger(VALUE val)
{
    RubyCallArgs<long> arg = {val, -1};
    int error = 0;
    rb_protect(rb_num2long_wrapper, (VALUE)&arg, &error);
    if (error)
    {
        throw RubyException(rb_eTypeError, "unable to convert value to an int");
    }
    return arg.ret;
}

int64_t
IceRuby::getLong(VALUE val)
{
    RubyCallArgs<int64_t> arg = {val, -1};
    int error = 0;
    rb_protect(rb_num2ll_wrapper, (VALUE)&arg, &error);
    if (error)
    {
        throw RubyException(rb_eTypeError, "unable to convert value to a long");
    }
    return arg.ret;
}

bool
IceRuby::arrayToStringSeq(VALUE val, vector<string>& seq)
{
    volatile VALUE arr = callRuby(rb_check_array_type, val);
    if (NIL_P(arr))
    {
        return false;
    }
    for (long i = 0; i < RARRAY_LEN(arr); ++i)
    {
        string s = getString(RARRAY_AREF(arr, i));
        seq.push_back(getString(RARRAY_AREF(arr, i)));
    }
    return true;
}

VALUE
IceRuby::stringSeqToArray(const vector<string>& seq)
{
    volatile VALUE result = createArray(seq.size());
    long i = 0;
    if (seq.size() > 0)
    {
        for (vector<string>::const_iterator p = seq.begin(); p != seq.end(); ++p, ++i)
        {
            RARRAY_ASET(result, i, createString(*p));
        }
    }
    return result;
}

VALUE
IceRuby::createNumSeq(const vector<byte>& v)
{
    volatile VALUE result = createArray(v.size());
    long i = 0;
    if (v.size() > 0)
    {
        for (vector<byte>::const_iterator p = v.begin(); p != v.end(); ++p, ++i)
        {
            RARRAY_ASET(result, i, INT2FIX(std::to_integer<int>(*p)));
        }
    }
    return result;
}

namespace
{
    struct HashToContextIterator : public IceRuby::HashIterator
    {
        HashToContextIterator(Ice::Context& c) : ctx(c) {}

        virtual void element(VALUE key, VALUE value)
        {
            string kstr = IceRuby::getString(key);
            string vstr = IceRuby::getString(value);
            ctx[kstr] = vstr;
        }

        Ice::Context& ctx;
    };
}

bool
IceRuby::hashToContext(VALUE val, Ice::Context& ctx)
{
    if (TYPE(val) != T_HASH)
    {
        val = callRuby(rb_convert_type, val, T_HASH, "Hash", "to_hash");
        if (NIL_P(val))
        {
            return false;
        }
    }
    HashToContextIterator iter(ctx);
    hashIterate(val, iter);
    return true;
}

VALUE
IceRuby::contextToHash(const Ice::Context& ctx)
{
    volatile VALUE result = callRuby(rb_hash_new);
    for (Ice::Context::const_iterator p = ctx.begin(); p != ctx.end(); ++p)
    {
        volatile VALUE key = createString(p->first);
        volatile VALUE value = createString(p->second);
        callRuby(rb_hash_aset, result, key, value);
    }
    return result;
}

extern "C" VALUE
#ifdef RUBY_BLOCK_CALL_FUNC_TAKES_BLOCKARG // Defined Ruby >= 2.1
IceRuby_Util_hash_foreach_callback(VALUE val, VALUE arg, int, const VALUE*, VALUE)
#else
IceRuby_Util_hash_foreach_callback(VALUE val, VALUE arg, int, VALUE*)
#endif
{
    VALUE key = rb_ary_entry(val, 0);
    VALUE value = rb_ary_entry(val, 1);

    //
    // We can't allow any C++ exceptions to propagate out of this function.
    //
    ICE_RUBY_TRY
    {
        IceRuby::HashIterator* iter = reinterpret_cast<IceRuby::HashIterator*>(arg);
        iter->element(key, value);
    }
    ICE_RUBY_CATCH
    return val;
}

extern "C"
{
// Defined Ruby >= 2.1. Ruby 2.7 enables RB_BLOCK_CALL_FUNC_STRICT by default
#ifdef RB_BLOCK_CALL_FUNC_STRICT
    typedef rb_block_call_func_t ICE_RUBY_HASH_FOREACH_CALLBACK;
#else
    typedef VALUE (*ICE_RUBY_HASH_FOREACH_CALLBACK)(...);
#endif
}

void
IceRuby::hashIterate(VALUE h, HashIterator& iter)
{
    assert(TYPE(h) == T_HASH);

    callRuby(
        ::rb_block_call,
        h,
        rb_intern("each"),
        0,
        static_cast<VALUE*>(0),
        reinterpret_cast<ICE_RUBY_HASH_FOREACH_CALLBACK>(IceRuby_Util_hash_foreach_callback),
        reinterpret_cast<VALUE>(&iter));
}

Ice::Identity
IceRuby::getIdentity(VALUE v)
{
    volatile VALUE cls = callRuby(rb_path2class, "Ice::Identity");
    assert(!NIL_P(cls));

    if (callRuby(rb_obj_is_kind_of, v, cls) == Qfalse)
    {
        throw RubyException(rb_eTypeError, "value is not an Ice::Identity");
    }

    volatile VALUE name = callRuby(rb_iv_get, v, "@name");
    volatile VALUE category = callRuby(rb_iv_get, v, "@category");

    if (!NIL_P(category) && !isString(category))
    {
        throw RubyException(rb_eTypeError, "identity category must be a string");
    }

    if (NIL_P(name) || !isString(name))
    {
        throw RubyException(rb_eTypeError, "identity name must be a string");
    }

    Ice::Identity result;
    result.name = getString(name);
    if (!NIL_P(category))
    {
        result.category = getString(category);
    }
    return result;
}

VALUE
IceRuby::createIdentity(const Ice::Identity& id)
{
    volatile VALUE cls = callRuby(rb_path2class, "Ice::Identity");
    assert(!NIL_P(cls));

    volatile VALUE result = callRuby(rb_class_new_instance, 0, reinterpret_cast<VALUE*>(0), cls);
    volatile VALUE name = createString(id.name);
    volatile VALUE category = createString(id.category);
    callRuby(rb_iv_set, result, "@name", name);
    callRuby(rb_iv_set, result, "@category", category);
    return result;
}

VALUE
IceRuby::createProtocolVersion(const Ice::ProtocolVersion& v)
{
    return createVersion<Ice::ProtocolVersion>(v, Ice_ProtocolVersion);
}

VALUE
IceRuby::createEncodingVersion(const Ice::EncodingVersion& v)
{
    return createVersion<Ice::EncodingVersion>(v, Ice_EncodingVersion);
}

bool
IceRuby::getEncodingVersion(VALUE p, Ice::EncodingVersion& v)
{
    volatile VALUE cls = callRuby(rb_path2class, Ice_EncodingVersion);
    assert(!NIL_P(cls));

    if (callRuby(rb_obj_is_kind_of, p, cls) == Qfalse)
    {
        throw RubyException(rb_eTypeError, "value is not an Ice::EncodingVersion");
    }

    if (!getVersion<Ice::EncodingVersion>(p, v))
    {
        return false;
    }

    return true;
}

VALUE
IceRuby::callProtected(RubyFunction func, VALUE arg)
{
    int error = 0;
    volatile VALUE result = rb_protect(func, arg, &error);
    if (error)
    {
        throw RubyException();
    }
    return result;
}

VALUE
IceRuby::createArray(long sz)
{
    volatile VALUE arr = callRuby(rb_ary_new2, sz);
    if (sz > 0)
    {
        callRubyVoid(rb_ary_store, arr, sz - 1, Qnil);
    }
    return arr;
}

namespace
{
    template<size_t N>
    VALUE createRubyException(const char* typeId, std::array<VALUE, N> args, bool fallbackToLocalException = false)
    {
        // Convert the exception's typeId to its mapped Ruby type by removing the leading "::".
        // This function should only ever be called on Ice local exceptions which don't use 'ruby:identifier'.
        string className = string{typeId}.substr(2);
        assert(className.starts_with("Ice::"));

        VALUE rubyClass;
        try
        {
            // callRuby throws a RubyException if rb_path2class fails.
            rubyClass = callRuby(rb_path2class, className.c_str());
        }
        catch (const RubyException&)
        {
            if (fallbackToLocalException)
            {
                rubyClass = callRuby(rb_path2class, "Ice::LocalException");
            }
            else
            {
                throw;
            }
        }
        return callRuby(rb_class_new_instance, N, args.data(), rubyClass);
    }
}

VALUE
IceRuby::convertException(std::exception_ptr eptr)
{
    const char* const localExceptionTypeId = "::Ice::LocalException";

    // We cannot throw a C++ exception or raise a Ruby exception. If an error occurs while we are converting the
    // exception, we do our best to _return_ an appropriate Ruby exception.
    try
    {
        try
        {
            rethrow_exception(eptr);
        }
        // First handle exceptions with extra fields we want to provide to Ruby users.
        catch (const Ice::AlreadyRegisteredException& ex)
        {
            std::array args{
                IceRuby::createString(ex.kindOfObject()),
                IceRuby::createString(ex.id()),
                IceRuby::createString(ex.what())};

            return createRubyException(ex.ice_id(), std::move(args));
        }
        catch (const Ice::NotRegisteredException& ex)
        {
            std::array args{
                IceRuby::createString(ex.kindOfObject()),
                IceRuby::createString(ex.id()),
                IceRuby::createString(ex.what())};

            return createRubyException(ex.ice_id(), std::move(args));
        }
        catch (const Ice::RequestFailedException& ex)
        {
            volatile VALUE replyStatus = callRuby(rb_int2inum, static_cast<int>(ex.replyStatus()));
            std::array args{
                replyStatus,
                IceRuby::createIdentity(ex.id()),
                IceRuby::createString(ex.facet()),
                IceRuby::createString(ex.operation()),
                IceRuby::createString(ex.what())};

            return createRubyException(ex.ice_id(), std::move(args));
        }
        catch (const Ice::DispatchException& ex)
        {
            volatile VALUE replyStatus = callRuby(rb_int2inum, static_cast<int>(ex.replyStatus()));
            std::array args{replyStatus, IceRuby::createString(ex.what())};
            return createRubyException(ex.ice_id(), std::move(args));
        }
        // Then all other exceptions.
        catch (const Ice::LocalException& ex)
        {
            std::array args{IceRuby::createString(ex.what())};
            return createRubyException(ex.ice_id(), std::move(args), true);
        }
        catch (const std::exception& ex)
        {
            std::array args{IceRuby::createString(ex.what())};
            return createRubyException(localExceptionTypeId, std::move(args));
        }
        catch (...)
        {
            std::array args{IceRuby::createString("unknown C++ exception")};
            return createRubyException(localExceptionTypeId, std::move(args));
        }
    }
    catch (const RubyException& e)
    {
        return e.ex;
    }
    catch (const std::exception& e)
    {
        string msg = "failure occurred while converting C++ exception to Ruby: " + string{e.what()};
        return rb_exc_new2(rb_eRuntimeError, msg.c_str());
    }
    catch (...)
    {
        return rb_exc_new2(rb_eRuntimeError, "failure occurred while converting C++ exception to Ruby");
    }
}
