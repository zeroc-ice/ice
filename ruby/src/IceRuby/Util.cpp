//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Util.h>
#include <Ice/LocalException.h>
#include <Ice/Protocol.h>
#include <stdarg.h>

#ifdef HAVE_RUBY_ENCODING_H
#  include <ruby/encoding.h>
#endif

using namespace std;
using namespace IceRuby;

namespace
{

template<typename T>
bool
setVersion(VALUE p, const T& version)
{
    volatile VALUE major = callRuby(rb_int2inum, version.major);
    volatile VALUE minor = callRuby(rb_int2inum, version.minor);
    rb_ivar_set(p, rb_intern("@major"), major);
    rb_ivar_set(p, rb_intern("@minor"), minor);

    return true;
}

template<typename T>
bool
getVersion(VALUE p, T& v)
{
    volatile VALUE major = callRuby(rb_ivar_get, p, rb_intern("@major"));
    volatile VALUE minor = callRuby(rb_ivar_get, p, rb_intern("@minor"));

    long m;

    m = getInteger(major);
    if(m < 0 || m > 255)
    {
        throw RubyException(rb_eTypeError, "version major must be a value between 0 and 255");
        return false;
    }
    v.major = m;

    m = getInteger(minor);
    if(m < 0 || m > 255)
    {
        throw RubyException(rb_eTypeError, "version minor must be a value between 0 and 255");
        return false;
    }
    v.minor = m;

    return true;
}

template<typename T>
VALUE
createVersion(const T& version, const char* type)
{
    volatile VALUE rbType = callRuby(rb_path2class, type);
    assert(!NIL_P(rbType));

    volatile VALUE obj = callRuby(rb_class_new_instance, 0, static_cast<VALUE*>(0), rbType);

    if(!setVersion<T>(obj, version))
    {
        return Qnil;
    }

    return obj;
}

template<typename T>
VALUE
versionToString(VALUE p, const char* type)
{
    volatile VALUE rbType = callRuby(rb_path2class, type);
    assert(!NIL_P(rbType));
    if(callRuby(rb_obj_is_instance_of, p, rbType) != Qtrue)
    {
        throw RubyException(rb_eTypeError, "argument is not an instance of %s", type);
    }

    T v;
    if(!getVersion<T>(p, v))
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

template<typename T>
VALUE
stringToVersion(VALUE p, const char* type)
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

extern "C"
VALUE
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

extern "C"
VALUE
IceRuby_intVersion(int /*argc*/, VALUE* /*argv*/, VALUE /*self*/)
{
    ICE_RUBY_TRY
    {
        return INT2FIX(ICE_INT_VERSION);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_currentProtocol(int /*argc*/, VALUE* /*argv*/, VALUE /*self*/)
{
    ICE_RUBY_TRY
    {
        return createProtocolVersion(Ice::currentProtocol);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_currentProtocolEncoding(int /*argc*/, VALUE* /*argv*/, VALUE /*self*/)
{
    ICE_RUBY_TRY
    {
        return createEncodingVersion(Ice::currentProtocolEncoding);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_currentEncoding(int /*argc*/, VALUE* /*argv*/, VALUE /*self*/)
{
    ICE_RUBY_TRY
    {
        return createEncodingVersion(Ice::currentEncoding);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_protocolVersionToString(VALUE /*self*/, VALUE v)
{
    return versionToString<Ice::ProtocolVersion>(v, Ice_ProtocolVersion);
}

extern "C"
VALUE
IceRuby_stringToProtocolVersion(VALUE /*self*/, VALUE v)
{
    return stringToVersion<Ice::ProtocolVersion>(v, Ice_ProtocolVersion);
}

extern "C"
VALUE
IceRuby_encodingVersionToString(VALUE /*self*/, VALUE v)
{
    return versionToString<Ice::EncodingVersion>(v, Ice_EncodingVersion);
}

extern "C"
VALUE
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

IceRuby::RubyException::RubyException()
{
    ex = rb_gv_get("$!");
}

IceRuby::RubyException::RubyException(VALUE exv) :
    ex(exv)
{
}

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
    ostr << RSTRING_PTR(cls) << ": " << RSTRING_PTR(msg);
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
IceRuby::createString(const string& str)
{
#ifdef HAVE_RUBY_ENCODING_H
    return callRuby(rb_enc_str_new, str.c_str(), static_cast<long>(str.size()), rb_utf8_encoding());
#else
    return callRuby(rb_str_new, str.c_str(), static_cast<long>(str.size()));
#endif
}

namespace
{

template <typename T>
struct RubyCallArgs
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
    RubyCallArgs<Ice::Long>* data = (RubyCallArgs<Ice::Long>*)val;
    data->ret = rb_num2ll(data->val);
    return val;
}

}

long
IceRuby::getInteger(VALUE val)
{
    RubyCallArgs<long> arg= {val, -1};
    int error = 0;
    rb_protect(rb_num2long_wrapper, (VALUE)&arg, &error);
    if(error)
    {
        throw RubyException(rb_eTypeError, "unable to convert value to an int");
    }
    return arg.ret;
}

Ice::Long
IceRuby::getLong(VALUE val)
{
    RubyCallArgs<Ice::Long> arg= {val, -1};
    int error = 0;
    rb_protect(rb_num2ll_wrapper, (VALUE)&arg, &error);
    if(error)
    {
        throw RubyException(rb_eTypeError, "unable to convert value to a long");
    }
    return arg.ret;
}

bool
IceRuby::arrayToStringSeq(VALUE val, vector<string>& seq)
{
    volatile VALUE arr = callRuby(rb_check_array_type, val);
    if(NIL_P(arr))
    {
        return false;
    }
    for(long i = 0; i < RARRAY_LEN(arr); ++i)
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
    if(seq.size() > 0)
    {
        for(vector<string>::const_iterator p = seq.begin(); p != seq.end(); ++p, ++i)
        {
            RARRAY_ASET(result, i, createString(*p));
        }
    }
    return result;
}

VALUE
IceRuby::createNumSeq(const vector<Ice::Byte>& v)
{
    volatile VALUE result = createArray(v.size());
    long i = 0;
    if(v.size() > 0)
    {
        for(vector<Ice::Byte>::const_iterator p = v.begin(); p != v.end(); ++p, ++i)
        {
            RARRAY_ASET(result, i, INT2FIX(*p));
        }
    }
    return result;
}

namespace
{

struct HashToContextIterator : public IceRuby::HashIterator
{
    HashToContextIterator(Ice::Context& c) : ctx(c)
    {
    }

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
    if(TYPE(val) != T_HASH)
    {
        val = callRuby(rb_convert_type, val, T_HASH, "Hash", "to_hash");
        if(NIL_P(val))
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
    for(Ice::Context::const_iterator p = ctx.begin(); p != ctx.end(); ++p)
    {
        volatile VALUE key = createString(p->first);
        volatile VALUE value = createString(p->second);
        callRuby(rb_hash_aset, result, key, value);
    }
    return result;
}

extern "C"
VALUE
IceRuby_Util_hash_foreach_callback(VALUE val, VALUE arg)
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
typedef VALUE (*ICE_RUBY_HASH_FOREACH_CALLBACK)(...);
}

void
IceRuby::hashIterate(VALUE h, HashIterator& iter)
{
    assert(TYPE(h) == T_HASH);
    callRuby(rb_iterate, rb_each, h,
             reinterpret_cast<ICE_RUBY_HASH_FOREACH_CALLBACK>(IceRuby_Util_hash_foreach_callback),
             reinterpret_cast<VALUE>(&iter));
}

Ice::Identity
IceRuby::getIdentity(VALUE v)
{
    volatile VALUE cls = callRuby(rb_path2class, "Ice::Identity");
    assert(!NIL_P(cls));

    if(callRuby(rb_obj_is_kind_of, v, cls) == Qfalse)
    {
        throw RubyException(rb_eTypeError, "value is not an Ice::Identity");
    }

    volatile VALUE name = callRuby(rb_iv_get, v, "@name");
    volatile VALUE category = callRuby(rb_iv_get, v, "@category");

    if(!NIL_P(category) && !isString(category))
    {
        throw RubyException(rb_eTypeError, "identity category must be a string");
    }

    if(NIL_P(name) || !isString(name))
    {
        throw RubyException(rb_eTypeError, "identity name must be a string");
    }

    Ice::Identity result;
    result.name = getString(name);
    if(!NIL_P(category))
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

    if(callRuby(rb_obj_is_kind_of, p, cls) == Qfalse)
    {
        throw RubyException(rb_eTypeError, "value is not an Ice::EncodingVersion");
    }

    if(!getVersion<Ice::EncodingVersion>(p, v))
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
    if(error)
    {
        throw RubyException();
    }
    return result;
}

static void
setExceptionMembers(const Ice::LocalException& ex, VALUE p)
{
    //
    // Transfer data members from Ice exception to Ruby exception.
    //
    try
    {
        ex.ice_throw();
    }
    catch(const Ice::InitializationException& e)
    {
        volatile VALUE v = createString(e.reason);
        callRuby(rb_iv_set, p, "@reason", v);
    }
    catch(const Ice::PluginInitializationException& e)
    {
        volatile VALUE v = createString(e.reason);
        callRuby(rb_iv_set, p, "@reason", v);
    }
    catch(const Ice::AlreadyRegisteredException& e)
    {
        volatile VALUE v;
        v = createString(e.kindOfObject);
        callRuby(rb_iv_set, p, "@kindOfObject", v);
        v = createString(e.id);
        callRuby(rb_iv_set, p, "@id", v);
    }
    catch(const Ice::NotRegisteredException& e)
    {
        volatile VALUE v;
        v = createString(e.kindOfObject);
        callRuby(rb_iv_set, p, "@kindOfObject", v);
        v = createString(e.id);
        callRuby(rb_iv_set, p, "@id", v);
    }
    catch(const Ice::TwowayOnlyException& e)
    {
        volatile VALUE v = createString(e.operation);
        callRuby(rb_iv_set, p, "@operation", v);
    }
    catch(const Ice::UnknownException& e)
    {
        volatile VALUE v = createString(e.unknown);
        callRuby(rb_iv_set, p, "@unknown", v);
    }
    catch(const Ice::ObjectAdapterDeactivatedException& e)
    {
        volatile VALUE v = createString(e.name);
        callRuby(rb_iv_set, p, "@name", v);
    }
    catch(const Ice::ObjectAdapterIdInUseException& e)
    {
        volatile VALUE v = createString(e.id);
        callRuby(rb_iv_set, p, "@id", v);
    }
    catch(const Ice::NoEndpointException& e)
    {
        volatile VALUE v = createString(e.proxy);
        callRuby(rb_iv_set, p, "@proxy", v);
    }
    catch(const Ice::EndpointParseException& e)
    {
        volatile VALUE v = createString(e.str);
        callRuby(rb_iv_set, p, "@str", v);
    }
    catch(const Ice::EndpointSelectionTypeParseException& e)
    {
        volatile VALUE v = createString(e.str);
        callRuby(rb_iv_set, p, "@str", v);
    }
    catch(const Ice::VersionParseException& e)
    {
        volatile VALUE v = createString(e.str);
        callRuby(rb_iv_set, p, "@str", v);
    }
    catch(const Ice::IdentityParseException& e)
    {
        volatile VALUE v = createString(e.str);
        callRuby(rb_iv_set, p, "@str", v);
    }
    catch(const Ice::ProxyParseException& e)
    {
        volatile VALUE v = createString(e.str);
        callRuby(rb_iv_set, p, "@str", v);
    }
    catch(const Ice::IllegalIdentityException& e)
    {
        volatile VALUE v = IceRuby::createIdentity(e.id);
        callRuby(rb_iv_set, p, "@id", v);
    }
    catch(const Ice::IllegalServantException& e)
    {
        volatile VALUE v = createString(e.reason);
        callRuby(rb_iv_set, p, "@reason", v);
    }
    catch(const Ice::RequestFailedException& e)
    {
        volatile VALUE v;
        v = IceRuby::createIdentity(e.id);
        callRuby(rb_iv_set, p, "@id", v);
        v = createString(e.facet);
        callRuby(rb_iv_set, p, "@facet", v);
        v = createString(e.operation);
        callRuby(rb_iv_set, p, "@operation", v);
    }
    catch(const Ice::FileException& e)
    {
        volatile VALUE v = INT2FIX(e.error);
        callRuby(rb_iv_set, p, "@error", v);
        v = createString(e.path);
        callRuby(rb_iv_set, p, "@path", v);
    }
    catch(const Ice::SyscallException& e) // This must appear after all subclasses of SyscallException.
    {
        volatile VALUE v = INT2FIX(e.error);
        callRuby(rb_iv_set, p, "@error", v);
    }
    catch(const Ice::DNSException& e)
    {
        volatile VALUE v;
        v = INT2FIX(e.error);
        callRuby(rb_iv_set, p, "@error", v);
        v = createString(e.host);
        callRuby(rb_iv_set, p, "@host", v);
    }
    catch(const Ice::BadMagicException& e)
    {
        volatile VALUE v = createNumSeq(e.badMagic);
        callRuby(rb_iv_set, p, "@badMagic", v);
    }
    catch(const Ice::UnsupportedProtocolException& e)
    {
        VALUE m;
        m = createProtocolVersion(e.bad);
        callRuby(rb_iv_set, p, "@bad", m);
        m = createProtocolVersion(e.supported);
        callRuby(rb_iv_set, p, "@supported", m);
    }
    catch(const Ice::UnsupportedEncodingException& e)
    {
        VALUE m;
        m = createEncodingVersion(e.bad);
        callRuby(rb_iv_set, p, "@bad", m);
        m = createEncodingVersion(e.supported);
        callRuby(rb_iv_set, p, "@supported", m);
    }
    catch(const Ice::NoValueFactoryException& e)
    {
        volatile VALUE v;
        v = createString(e.reason);
        callRuby(rb_iv_set, p, "@reason", v);
        v = createString(e.type);
        callRuby(rb_iv_set, p, "@type", v);
    }
    catch(const Ice::UnexpectedObjectException& e)
    {
        volatile VALUE v;
        v = createString(e.reason);
        callRuby(rb_iv_set, p, "@reason", v);
        v = createString(e.type);
        callRuby(rb_iv_set, p, "@type", v);
        v = createString(e.expectedType);
        callRuby(rb_iv_set, p, "@expectedType", v);
    }
    catch(const Ice::ProtocolException& e) // This must appear after all subclasses of ProtocolException.
    {
        volatile VALUE v = createString(e.reason);
        callRuby(rb_iv_set, p, "@reason", v);
    }
    catch(const Ice::ConnectionManuallyClosedException& e)
    {
        callRuby(rb_iv_set, p, "@graceful", e.graceful ? Qtrue : Qfalse);
    }
    catch(const Ice::FeatureNotSupportedException& e)
    {
        volatile VALUE v = createString(e.unsupportedFeature);
        callRuby(rb_iv_set, p, "@unsupportedFeature", v);
    }
    catch(const Ice::SecurityException& e)
    {
        volatile VALUE v = createString(e.reason);
        callRuby(rb_iv_set, p, "@reason", v);
    }
    catch(const Ice::LocalException&)
    {
        //
        // Nothing to do.
        //
    }
}

VALUE
IceRuby::createArrayHelper(long sz)
{
    volatile VALUE arr = callRuby(rb_ary_new2, sz);
    if(sz > 0)
    {
        callRubyVoid(rb_ary_store, arr, sz - 1, Qnil);
    }
    return arr;
}

VALUE
IceRuby::convertLocalException(const Ice::LocalException& ex)
{
    //
    // We cannot throw a C++ exception or raise a Ruby exception. If an error
    // occurs while we are converting the exception, we do our best to return
    // an appropriate Ruby exception.
    //
    try
    {
        string name = ex.ice_id().substr(2);
        volatile VALUE cls = callRuby(rb_path2class, name.c_str());
        if(NIL_P(cls))
        {
            throw RubyException(rb_eRuntimeError, "exception class `%s' not found", name.c_str());
        }
        volatile VALUE result = callRuby(rb_class_new_instance, 0, reinterpret_cast<VALUE*>(0), cls);
        setExceptionMembers(ex, result);
        return result;
    }
    catch(const RubyException& e)
    {
        return e.ex;
    }
    catch(...)
    {
        string msg = "failure occurred while converting exception " + ex.ice_id();
        return rb_exc_new2(rb_eRuntimeError, msg.c_str());
    }
}
