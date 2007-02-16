// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Util.h>
#include <Ice/LocalException.h>
#include <stdarg.h>
#include <st.h>

using namespace std;
using namespace IceRuby;

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
    ostr << RSTRING(cls)->ptr << ": " << RSTRING(msg)->ptr;
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
    return RSTRING(result)->ptr;
}

VALUE
IceRuby::createString(const string& str)
{
    return callRuby(rb_str_new, str.c_str(), static_cast<long>(str.size()));
}

long
IceRuby::getInteger(VALUE val)
{
    if(!FIXNUM_P(val) && TYPE(val) != T_BIGNUM)
    {
        val = callRuby(rb_Integer, val);
    }
    if(FIXNUM_P(val))
    {
        return FIX2LONG(val);
    }
    else if(TYPE(val) == T_BIGNUM)
    {
        Ice::Long l = getLong(val);
        if(l >= static_cast<Ice::Long>(INT_MIN) && l <= static_cast<Ice::Long>(INT_MAX))
        {
            return static_cast<long>(l);
        }
    }
    throw RubyException(rb_eTypeError, "unable to convert value to an integer");
}

#define BDIGITS(x) ((BDIGIT*)RBIGNUM(x)->digits)
#define BITSPERDIG (SIZEOF_BDIGITS*CHAR_BIT)
#define BIGUP(x) ((BDIGIT_DBL)(x) << BITSPERDIG)

Ice::Long
IceRuby::getLong(VALUE val)
{
    //
    // The rb_num2ll function raises exceptions, but we can't call it using callProtected
    // because its return type is long long and not VALUE.
    //
    volatile VALUE v = callRuby(rb_Integer, val);
    if(NIL_P(v))
    {
        throw RubyException(rb_eTypeError, "unable to convert value to a long");
    }
    if(FIXNUM_P(v))
    {
        return FIX2LONG(v);
    }
    else
    {
        assert(TYPE(v) == T_BIGNUM);
        long len = RBIGNUM(v)->len;
        if(len > SIZEOF_LONG_LONG/SIZEOF_BDIGITS)
        {
            throw RubyException(rb_eRangeError, "bignum too big to convert into long");
        }
        BDIGIT *ds = BDIGITS(v);
        BDIGIT_DBL num = 0;
        while(len--)
        {
            num = BIGUP(num);
            num += ds[len];
        }
        Ice::Long l = static_cast<Ice::Long>(num);
        if(l < 0 && (RBIGNUM(v)->sign || l != LLONG_MIN))
        {
            throw RubyException(rb_eRangeError, "bignum too big to convert into long");
        }
        if (!RBIGNUM(v)->sign)
        {
            return -l;
        }
        return l;
    }
}

bool
IceRuby::arrayToStringSeq(VALUE val, vector<string>& seq)
{
    volatile VALUE arr = callRuby(rb_check_array_type, val);
    if(NIL_P(arr))
    {
        return false;
    }
    for(long i = 0; i < RARRAY(arr)->len; ++i)
    {
        string s = getString(RARRAY(arr)->ptr[i]);
        seq.push_back(getString(RARRAY(arr)->ptr[i]));
    }
    return true;
}

VALUE
IceRuby::stringSeqToArray(const vector<string>& seq)
{
    volatile VALUE result = createArray(seq.size());
    long i = 0;
    for(vector<string>::const_iterator p = seq.begin(); p != seq.end(); ++p, ++i)
    {
        RARRAY(result)->ptr[i] = createString(*p);
        RARRAY(result)->len++; // Increment len for each new element to prevent premature GC.
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
        volatile VALUE key = callRuby(rb_str_new, p->first.c_str(), static_cast<long>(p->first.size()));
        volatile VALUE value = callRuby(rb_str_new, p->second.c_str(), static_cast<long>(p->second.size()));
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
    volatile VALUE name = callRuby(rb_str_new, id.name.c_str(), static_cast<long>(id.name.size()));
    volatile VALUE category = callRuby(rb_str_new, id.category.c_str(), static_cast<long>(id.category.size()));
    callRuby(rb_iv_set, result, "@name", name);
    callRuby(rb_iv_set, result, "@category", category);
    return result;
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
    catch(const Ice::UnsupportedProtocolException& e)
    {
        callRuby(rb_iv_set, p, "@badMajor", INT2FIX(e.badMajor));
        callRuby(rb_iv_set, p, "@badMinor", INT2FIX(e.badMinor));
        callRuby(rb_iv_set, p, "@major", INT2FIX(e.major));
        callRuby(rb_iv_set, p, "@minor", INT2FIX(e.minor));
    }
    catch(const Ice::UnsupportedEncodingException& e)
    {
        callRuby(rb_iv_set, p, "@badMajor", INT2FIX(e.badMajor));
        callRuby(rb_iv_set, p, "@badMinor", INT2FIX(e.badMinor));
        callRuby(rb_iv_set, p, "@major", INT2FIX(e.major));
        callRuby(rb_iv_set, p, "@minor", INT2FIX(e.minor));
    }
    catch(const Ice::NoObjectFactoryException& e)
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
IceRuby::convertLocalException(const Ice::LocalException& ex)
{
    //
    // We cannot throw a C++ exception or raise a Ruby exception. If an error
    // occurs while we are converting the exception, we do our best to return
    // an appropriate Ruby exception.
    //
    try
    {
        string name = ex.ice_name();
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
        string msg = "failure occurred while converting exception " + ex.ice_name();
        return rb_exc_new2(rb_eRuntimeError, msg.c_str());
    }
}
