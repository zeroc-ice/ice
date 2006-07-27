// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
    VALUE cls = rb_class_path(CLASS_OF(ex));
    VALUE msg = rb_obj_as_string(ex);
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
    VALUE result = callRuby(rb_string_value, &val);
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
    VALUE v = callRuby(rb_Integer, val);
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
    VALUE arr = callRuby(rb_check_array_type, val);
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
    VALUE result = createArray(seq.size());
    long i = 0;
    for(vector<string>::const_iterator p = seq.begin(); p != seq.end(); ++p, ++i)
    {
	RARRAY(result)->ptr[i] = createString(*p);
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
    VALUE result = callRuby(rb_hash_new);
    for(Ice::Context::const_iterator p = ctx.begin(); p != ctx.end(); ++p)
    {
	VALUE key = callRuby(rb_str_new, p->first.c_str(), static_cast<long>(p->first.size()));
	VALUE value = callRuby(rb_str_new, p->second.c_str(), static_cast<long>(p->second.size()));
	callRuby(rb_hash_aset, result, key, value);
    }
    return result;
}

extern "C"
int
IceRuby_Util_hash_foreach_callback(VALUE key, VALUE value, VALUE arg)
{
    //
    // We can't allow any C++ exceptions to propagate out of this function.
    //
    ICE_RUBY_TRY
    {
	IceRuby::HashIterator* iter = reinterpret_cast<IceRuby::HashIterator*>(arg);
	iter->element(key, value);
	return ST_CONTINUE;
    }
    ICE_RUBY_CATCH
    return ST_STOP;
}

//
// Wrapper for rb_hash_foreach because it doesn't return VALUE.
//
extern "C"
VALUE
IceRuby_Util_rb_hash_foreach(VALUE hash, int (*func)(ANYARGS), VALUE arg)
{
    rb_hash_foreach(hash, func, arg);
    return Qnil;
}   

void
IceRuby::hashIterate(VALUE hash, HashIterator& iter)
{
    assert(TYPE(hash) == T_HASH);
    callRuby(IceRuby_Util_rb_hash_foreach, hash,
	     reinterpret_cast<int (*)(ANYARGS)>(IceRuby_Util_hash_foreach_callback),
	     reinterpret_cast<VALUE>(&iter));
}

Ice::Identity
IceRuby::getIdentity(VALUE v)
{
    VALUE cls = callRuby(rb_path2class, "Ice::Identity");
    assert(!NIL_P(cls));

    if(callRuby(rb_obj_is_kind_of, v, cls) == Qfalse)
    {
	throw RubyException(rb_eTypeError, "value is not an Ice::Identity");
    }

    VALUE name = callRuby(rb_iv_get, v, "@name");
    VALUE category = callRuby(rb_iv_get, v, "@category");

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
    VALUE cls = callRuby(rb_path2class, "Ice::Identity");
    assert(!NIL_P(cls));

    VALUE result = callRuby(rb_class_new_instance, 0, reinterpret_cast<VALUE*>(0), cls);
    VALUE name = callRuby(rb_str_new, id.name.c_str(), static_cast<long>(id.name.size()));
    VALUE category = callRuby(rb_str_new, id.category.c_str(), static_cast<long>(id.category.size()));
    callRuby(rb_iv_set, result, "@name", name);
    callRuby(rb_iv_set, result, "@category", category);
    return result;
}

VALUE
IceRuby::callProtected(RubyFunction func, VALUE arg)
{
    int error = 0;
    VALUE result = rb_protect(func, arg, &error);
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
    catch(const Ice::UnknownException& e)
    {
	VALUE v = createString(e.unknown);
	callRuby(rb_iv_set, p, "@unknown", v);
    }
    catch(const Ice::ObjectAdapterDeactivatedException& e)
    {
	VALUE v = createString(e.name);
	callRuby(rb_iv_set, p, "@name", v);
    }
    catch(const Ice::ObjectAdapterIdInUseException& e)
    {
	VALUE v = createString(e.id);
	callRuby(rb_iv_set, p, "@id", v);
    }
    catch(const Ice::NoEndpointException& e)
    {
	VALUE v = createString(e.proxy);
	callRuby(rb_iv_set, p, "@proxy", v);
    }
    catch(const Ice::EndpointParseException& e)
    {
	VALUE v = createString(e.str);
	callRuby(rb_iv_set, p, "@str", v);
    }
    catch(const Ice::IdentityParseException& e)
    {
	VALUE v = createString(e.str);
	callRuby(rb_iv_set, p, "@str", v);
    }
    catch(const Ice::ProxyParseException& e)
    {
	VALUE v = createString(e.str);
	callRuby(rb_iv_set, p, "@str", v);
    }
    catch(const Ice::IllegalIdentityException& e)
    {
	VALUE v = IceRuby::createIdentity(e.id);
	callRuby(rb_iv_set, p, "@id", v);
    }
    catch(const Ice::RequestFailedException& e)
    {
	VALUE v;
	v = IceRuby::createIdentity(e.id);
	callRuby(rb_iv_set, p, "@id", v);
	v = createString(e.facet);
	callRuby(rb_iv_set, p, "@facet", v);
	v = createString(e.operation);
	callRuby(rb_iv_set, p, "@operation", v);
    }
    catch(const Ice::SyscallException& e)
    {
	VALUE v = INT2FIX(e.error);
	callRuby(rb_iv_set, p, "@error", v);
    }
    catch(const Ice::DNSException& e)
    {
	VALUE v;
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
	VALUE v;
	v = createString(e.reason);
	callRuby(rb_iv_set, p, "@reason", v);
	v = createString(e.type);
	callRuby(rb_iv_set, p, "@type", v);
    }
    catch(const Ice::MarshalException& e)
    {
	VALUE v = createString(e.reason);
	callRuby(rb_iv_set, p, "@reason", v);
    }
    catch(const Ice::PluginInitializationException& e)
    {
	VALUE v = createString(e.reason);
	callRuby(rb_iv_set, p, "@reason", v);
    }
    catch(const Ice::AlreadyRegisteredException& e)
    {
	VALUE v;
	v = createString(e.kindOfObject);
	callRuby(rb_iv_set, p, "@kindOfObject", v);
	v = createString(e.id);
	callRuby(rb_iv_set, p, "@id", v);
    }
    catch(const Ice::NotRegisteredException& e)
    {
	VALUE v;
	v = createString(e.kindOfObject);
	callRuby(rb_iv_set, p, "@kindOfObject", v);
	v = createString(e.id);
	callRuby(rb_iv_set, p, "@id", v);
    }
    catch(const Ice::TwowayOnlyException& e)
    {
	VALUE v = createString(e.operation);
	callRuby(rb_iv_set, p, "@operation", v);
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
	VALUE cls = callRuby(rb_path2class, name.c_str());
	if(NIL_P(cls))
	{
	    throw RubyException(rb_eRuntimeError, "exception class `%s' not found", name.c_str());
	}
	VALUE result = callRuby(rb_class_new_instance, 0, reinterpret_cast<VALUE*>(0), cls);
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

extern "C"
VALUE
IceRuby_RubyThread_threadMain(RubyThreadPtr* p)
{
    ICE_RUBY_TRY
    {
	assert(p);
	try
	{
	    (*p)->run();
	}
	catch(...)
	{
	    delete p;
	    throw;
	}
	delete p;
    }
    ICE_RUBY_CATCH
    return Qnil;
}

void
IceRuby::RubyThread::start(bool join)
{
    VALUE t = callRuby(rb_thread_create, reinterpret_cast<VALUE (*)(ANYARGS)>(IceRuby_RubyThread_threadMain),
		       reinterpret_cast<void*>(new RubyThreadPtr(this)));
    assert(!NIL_P(t));
    if(join)
    {
	callRuby(rb_funcall, t, rb_intern("join"), 0);
    }
}
