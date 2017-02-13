// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <Proxy.h>
#include <Communicator.h>
#include <Connection.h>
#include <Endpoint.h>
#include <Util.h>
#include <Ice/LocalException.h>
#include <Ice/Locator.h>
#include <Ice/Proxy.h>
#include <Ice/Router.h>

using namespace std;
using namespace IceRuby;

static VALUE _proxyClass;

// **********************************************************************
// ObjectPrx
// **********************************************************************

extern "C"
void
IceRuby_ObjectPrx_mark(Ice::ObjectPrx* p)
{
    //
    // We need to mark the communicator associated with this proxy.
    //
    assert(p);
    volatile VALUE communicator = lookupCommunicator((*p)->ice_getCommunicator());
    assert(!NIL_P(communicator));
    rb_gc_mark(communicator);
}

extern "C"
void
IceRuby_ObjectPrx_free(Ice::ObjectPrx* p)
{
    assert(p);
    delete p;
}

//
// If a context was provided set it to ::Ice::noExplicitContext.
//
static void
checkArgs(const char* name, int numArgs, int argc, VALUE* argv, Ice::Context& ctx)
{
    if(argc < numArgs || argc > numArgs + 1)
    {
        throw RubyException(rb_eArgError, "%s expects %d argument%s including an optional context hash", name,
                            numArgs + 1, numArgs + 1 == 1 ? "" : "s");
    }
    if(argc == numArgs + 1)
    {
        if(!hashToContext(argv[numArgs], ctx))
        {
            throw RubyException(rb_eArgError, "%s: invalid context hash", name);
        }
    }
    else
    {
        ctx = ::Ice::noExplicitContext;
    }
}

extern "C"
VALUE
IceRuby_ObjectPrx_hash(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        return INT2FIX(p->_hash());
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_getCommunicator(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        Ice::CommunicatorPtr communicator = p->ice_getCommunicator();
        return lookupCommunicator(communicator);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_toString(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        string s = p->ice_toString();
        return createString(s);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_isA(int argc, VALUE* argv, VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);

        Ice::Context ctx;
        checkArgs("ice_isA", 1, argc, argv, ctx);
        string id = getString(argv[0]);

        return p->ice_isA(id, ctx) ? Qtrue : Qfalse;
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_ping(int argc, VALUE* argv, VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);

        Ice::Context ctx;
        checkArgs("ice_ping", 0, argc, argv, ctx);
        p->ice_ping(ctx);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_ids(int argc, VALUE* argv, VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);

        Ice::Context ctx;
        checkArgs("ice_ids", 0, argc, argv, ctx);

        vector<string> ids = p->ice_ids(ctx);
        volatile VALUE result = createArray(ids.size());
        long i = 0;
        for(vector<string>::iterator q = ids.begin(); q != ids.end(); ++q, ++i)
        {
            RARRAY_ASET(result, i, createString(*q));
        }

        return result;
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_id(int argc, VALUE* argv, VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);

        Ice::Context ctx;
        checkArgs("ice_id", 0, argc, argv, ctx);
        string id = p->ice_id(ctx);
        return createString(id);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_getIdentity(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        return createIdentity(p->ice_getIdentity());
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_identity(VALUE self, VALUE id)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        Ice::Identity ident = getIdentity(id);
        return createProxy(p->ice_identity(ident));
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_getContext(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        return contextToHash(p->ice_getContext());
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_context(VALUE self, VALUE ctx)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);

        Ice::Context context;
        if(!NIL_P(ctx) && !hashToContext(ctx, context))
        {
            throw RubyException(rb_eTypeError, "argument is not a context hash");
        }
        return createProxy(p->ice_context(context), rb_class_of(self));
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_getFacet(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        string facet = p->ice_getFacet();
        return createString(facet);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_facet(VALUE self, VALUE facet)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        string f = getString(facet);
        return createProxy(p->ice_facet(f));
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_getAdapterId(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        string id = p->ice_getAdapterId();
        return createString(id);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_adapterId(VALUE self, VALUE id)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        string idstr = getString(id);
        return createProxy(p->ice_adapterId(idstr), rb_class_of(self));
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_getEndpoints(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);

        Ice::EndpointSeq seq = p->ice_getEndpoints();
        volatile VALUE result = createArray(seq.size());
        long i = 0;
        for(Ice::EndpointSeq::iterator q = seq.begin(); q != seq.end(); ++q, ++i)
        {
            RARRAY_ASET(result, i, createEndpoint(*q));
        }
        return result;
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_endpoints(VALUE self, VALUE seq)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);

        if(!NIL_P(seq) && !isArray(seq))
        {
            throw RubyException(rb_eTypeError, "ice_endpoints requires an array of endpoints");
        }

        Ice::EndpointSeq endpoints;
        if(!NIL_P(seq))
        {
            volatile VALUE arr = callRuby(rb_check_array_type, seq);
            if(NIL_P(seq))
            {
                throw RubyException(rb_eTypeError, "unable to convert value to an array of endpoints");
            }
            for(long i = 0; i < RARRAY_LEN(arr); ++i)
            {
                if(!checkEndpoint(RARRAY_AREF(arr, i)))
                {
                    throw RubyException(rb_eTypeError, "array element is not an Ice::Endpoint");
                }
                Ice::EndpointPtr* e = reinterpret_cast<Ice::EndpointPtr*>(DATA_PTR(RARRAY_AREF(arr, i)));
                assert(e);
                endpoints.push_back(*e);
            }
        }
        return createProxy(p->ice_endpoints(endpoints), rb_class_of(self));
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_getLocatorCacheTimeout(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        Ice::Int t = p->ice_getLocatorCacheTimeout();
        return INT2FIX(t);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_getInvocationTimeout(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        Ice::Int t = p->ice_getInvocationTimeout();
        return INT2FIX(t);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_getConnectionId(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        string connectionId = p->ice_getConnectionId();
        return createString(connectionId);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_locatorCacheTimeout(VALUE self, VALUE timeout)
{
    ICE_RUBY_TRY
    {
        try
        {
            Ice::ObjectPrx p = getProxy(self);
            long t = getInteger(timeout);
            return createProxy(p->ice_locatorCacheTimeout(static_cast<Ice::Int>(t)), rb_class_of(self));
        }
        catch(const IceUtil::IllegalArgumentException& ex)
        {
            throw RubyException(rb_eArgError, ex.reason().c_str());
        }
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_invocationTimeout(VALUE self, VALUE timeout)
{
    ICE_RUBY_TRY
    {
        try
        {
            Ice::ObjectPrx p = getProxy(self);
            long t = getInteger(timeout);
            return createProxy(p->ice_invocationTimeout(static_cast<Ice::Int>(t)), rb_class_of(self));
        }
        catch(const IceUtil::IllegalArgumentException& ex)
        {
            throw RubyException(rb_eArgError, ex.reason().c_str());
        }
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_isConnectionCached(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        return p->ice_isConnectionCached() ? Qtrue : Qfalse;
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_connectionCached(VALUE self, VALUE b)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        return createProxy(p->ice_connectionCached(RTEST(b)), rb_class_of(self));
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_getEndpointSelection(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);

        Ice::EndpointSelectionType type = p->ice_getEndpointSelection();
        volatile VALUE cls = callRuby(rb_path2class, "Ice::EndpointSelectionType");
        assert(!NIL_P(cls));
        return callRuby(rb_funcall, cls, rb_intern("from_int"), 1, INT2NUM(static_cast<int>(type)));
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_endpointSelection(VALUE self, VALUE type)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);

        volatile VALUE cls = callRuby(rb_path2class, "Ice::EndpointSelectionType");
        assert(!NIL_P(cls));
        if(callRuby(rb_obj_is_instance_of, type, cls) == Qfalse)
        {
            throw RubyException(rb_eTypeError, "argument must be an Ice::EndpointSelectionType enumerator");
        }

        volatile VALUE val = callRuby(rb_funcall, type, rb_intern("to_i"), 0);
        Ice::EndpointSelectionType t = static_cast<Ice::EndpointSelectionType>(getInteger(val));
        return createProxy(p->ice_endpointSelection(t), rb_class_of(self));
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_isSecure(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        return p->ice_isSecure() ? Qtrue : Qfalse;
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_secure(VALUE self, VALUE b)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        return createProxy(p->ice_secure(RTEST(b)), rb_class_of(self));
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_getEncodingVersion(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        return createEncodingVersion(p->ice_getEncodingVersion());
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_encodingVersion(VALUE self, VALUE v)
{
    Ice::EncodingVersion val;
    if(getEncodingVersion(v, val))
    {
        ICE_RUBY_TRY
        {
            Ice::ObjectPrx p = getProxy(self);
            return createProxy(p->ice_encodingVersion(val), rb_class_of(self));
        }
        ICE_RUBY_CATCH
    }

    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_isPreferSecure(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        return p->ice_isPreferSecure() ? Qtrue : Qfalse;
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_preferSecure(VALUE self, VALUE b)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        return createProxy(p->ice_preferSecure(RTEST(b)), rb_class_of(self));
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_getRouter(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);

        Ice::RouterPrx router = p->ice_getRouter();
        if(router)
        {
            volatile VALUE cls = callRuby(rb_path2class, "Ice::RouterPrx");
            assert(!NIL_P(cls));
            return createProxy(router, cls);
        }
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_router(VALUE self, VALUE router)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);

        Ice::RouterPrx proxy;
        if(!NIL_P(router))
        {
            if(!checkProxy(router))
            {
                throw RubyException(rb_eTypeError, "argument must be a proxy");
            }
            proxy = Ice::RouterPrx::uncheckedCast(getProxy(router));
        }
        return createProxy(p->ice_router(proxy), rb_class_of(self));
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_getLocator(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);

        Ice::LocatorPrx locator = p->ice_getLocator();
        if(locator)
        {
            volatile VALUE cls = callRuby(rb_path2class, "Ice::LocatorPrx");
            assert(!NIL_P(cls));
            return createProxy(locator, cls);
        }
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_locator(VALUE self, VALUE locator)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);

        Ice::LocatorPrx proxy;
        if(!NIL_P(locator))
        {
            if(!checkProxy(locator))
            {
                throw RubyException(rb_eTypeError, "argument must be a proxy");
            }
            proxy = Ice::LocatorPrx::uncheckedCast(getProxy(locator));
        }
        return createProxy(p->ice_locator(proxy), rb_class_of(self));
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_twoway(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        return createProxy(p->ice_twoway(), rb_class_of(self));
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_isTwoway(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        return p->ice_isTwoway() ? Qtrue : Qfalse;
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_oneway(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        return createProxy(p->ice_oneway(), rb_class_of(self));
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_isOneway(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        return p->ice_isOneway() ? Qtrue : Qfalse;
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_batchOneway(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        return createProxy(p->ice_batchOneway(), rb_class_of(self));
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_isBatchOneway(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        return p->ice_isBatchOneway() ? Qtrue : Qfalse;
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_datagram(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        return createProxy(p->ice_datagram(), rb_class_of(self));
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_isDatagram(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        return p->ice_isDatagram() ? Qtrue : Qfalse;
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_batchDatagram(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        return createProxy(p->ice_batchDatagram(), rb_class_of(self));
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_isBatchDatagram(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        return p->ice_isBatchDatagram() ? Qtrue : Qfalse;
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_compress(VALUE self, VALUE b)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        return createProxy(p->ice_compress(RTEST(b)), rb_class_of(self));
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_timeout(VALUE self, VALUE t)
{
    ICE_RUBY_TRY
    {
        try
        {
            Ice::ObjectPrx p = getProxy(self);
            Ice::Int timeout = static_cast<Ice::Int>(getInteger(t));
            return createProxy(p->ice_timeout(timeout), rb_class_of(self));
        }
        catch(const IceUtil::IllegalArgumentException& ex)
        {
            throw RubyException(rb_eArgError, ex.reason().c_str());
        }
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_connectionId(VALUE self, VALUE id)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        string idstr = getString(id);
        return createProxy(p->ice_connectionId(idstr), rb_class_of(self));
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_getConnection(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        Ice::ConnectionPtr conn = p->ice_getConnection();
        return createConnection(conn);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_getCachedConnection(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        Ice::ConnectionPtr conn = p->ice_getCachedConnection();
        if(conn)
        {
            return createConnection(conn);
        }
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_flushBatchRequests(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ObjectPrx p = getProxy(self);
        p->ice_flushBatchRequests();
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_cmp(VALUE self, VALUE other)
{
    ICE_RUBY_TRY
    {
        if(NIL_P(other))
        {
            return INT2NUM(1);
        }
        if(!checkProxy(other))
        {
            throw RubyException(rb_eTypeError, "argument must be a proxy");
        }
        Ice::ObjectPrx p1 = getProxy(self);
        Ice::ObjectPrx p2 = getProxy(other);
        if(p1 < p2)
        {
            return INT2NUM(-1);
        }
        else if(p1 == p2)
        {
            return INT2NUM(0);
        }
        else
        {
            return INT2NUM(1);
        }
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_equals(VALUE self, VALUE other)
{
    return IceRuby_ObjectPrx_cmp(self, other) == INT2NUM(0) ? Qtrue : Qfalse;
}

static VALUE
checkedCastImpl(const Ice::ObjectPrx& p, const string& id, VALUE facet, VALUE ctx, VALUE type)
{
    Ice::ObjectPrx target;
    if(NIL_P(facet))
    {
        target = p;
    }
    else
    {
        target = p->ice_facet(getString(facet));
    }

    try
    {
        if(NIL_P(ctx))
        {
            if(target->ice_isA(id))
            {
                return createProxy(target, type);
            }
        }
        else
        {
            Ice::Context c;
#ifndef NDEBUG
            bool b =
#endif
            hashToContext(ctx, c);
            assert(b);

            if(target->ice_isA(id, c))
            {
                return createProxy(target, type);
            }
        }
    }
    catch(const Ice::FacetNotExistException&)
    {
        // Ignore.
    }

    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_checkedCast(int argc, VALUE* args, VALUE self)
{
    //
    // ice_checkedCast is called from generated code, therefore we always expect
    // to receive all four arguments.
    //
    ICE_RUBY_TRY
    {
        if(argc < 1 || argc > 3)
        {
            throw RubyException(rb_eArgError, "checkedCast requires a proxy argument and optional facet and context");
        }

        if(NIL_P(args[0]))
        {
            return Qnil;
        }

        if(!checkProxy(args[0]))
        {
            throw RubyException(rb_eArgError, "checkedCast requires a proxy argument");
        }

        Ice::ObjectPrx p = getProxy(args[0]);

        volatile VALUE facet = Qnil;
        volatile VALUE ctx = Qnil;

        if(argc == 3)
        {
            if(!NIL_P(args[1]) && !isString(args[1]))
            {
                throw RubyException(rb_eArgError, "facet argument to checkedCast must be a string");
            }
            facet = args[1];

            if(!NIL_P(args[2]) && !isHash(args[2]))
            {
                throw RubyException(rb_eArgError, "context argument to checkedCast must be a hash");
            }
            ctx = args[2];
        }
        else if(argc == 2)
        {
            if(isString(args[1]))
            {
                facet = args[1];
            }
            else if(isHash(args[1]))
            {
                ctx = args[1];
            }
            else
            {
                throw RubyException(rb_eArgError, "second argument to checkedCast must be a facet or context");
            }
        }

        return checkedCastImpl(p, "::Ice::Object", facet, ctx, Qnil);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_uncheckedCast(int argc, VALUE* args, VALUE self)
{
    ICE_RUBY_TRY
    {
        if(argc < 1 || argc > 2)
        {
            throw RubyException(rb_eArgError, "uncheckedCast requires a proxy argument and an optional facet");
        }

        if(NIL_P(args[0]))
        {
            return Qnil;
        }

        if(!checkProxy(args[0]))
        {
            throw RubyException(rb_eArgError, "uncheckedCast requires a proxy argument");
        }

        volatile VALUE facet = Qnil;
        if(argc == 2)
        {
            facet = args[1];
        }

        Ice::ObjectPrx p = getProxy(args[0]);

        if(!NIL_P(facet))
        {
            return createProxy(p->ice_facet(getString(facet)));
        }
        else
        {
            return createProxy(p);
        }
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_checkedCast(VALUE self, VALUE obj, VALUE id, VALUE facetOrContext, VALUE ctx)
{
    //
    // ice_checkedCast is called from generated code, therefore we always expect
    // to receive all four arguments.
    //
    ICE_RUBY_TRY
    {
        if(NIL_P(obj))
        {
            return Qnil;
        }

        if(!checkProxy(obj))
        {
            throw RubyException(rb_eArgError, "checkedCast requires a proxy argument");
        }

        Ice::ObjectPrx p = getProxy(obj);

        string idstr = getString(id);

        volatile VALUE facet = Qnil;
        if(isString(facetOrContext))
        {
            facet = facetOrContext;
        }
        else if(isHash(facetOrContext))
        {
            if(!NIL_P(ctx))
            {
                throw RubyException(rb_eArgError, "facet argument to checkedCast must be a string");
            }
            ctx = facetOrContext;
        }
        else if(!NIL_P(facetOrContext))
        {
            throw RubyException(rb_eArgError, "second argument to checkedCast must be a facet or context");
        }

        if(!NIL_P(ctx) && !isHash(ctx))
        {
            throw RubyException(rb_eArgError, "context argument to checkedCast must be a hash");
        }

        return checkedCastImpl(p, idstr, facet, ctx, self);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_uncheckedCast(VALUE self, VALUE obj, VALUE facet)
{
    ICE_RUBY_TRY
    {
        if(NIL_P(obj))
        {
            return Qnil;
        }

        if(!checkProxy(obj))
        {
            throw RubyException(rb_eArgError, "uncheckedCast requires a proxy argument");
        }

        Ice::ObjectPrx p = getProxy(obj);

        if(!NIL_P(facet))
        {
            return createProxy(p->ice_facet(getString(facet)), self);
        }
        else
        {
            return createProxy(p, self);
        }
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_ice_staticId(VALUE self)
{
    ICE_RUBY_TRY
    {
        return createString(Ice::Object::ice_staticId());
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_ObjectPrx_new(int /*argc*/, VALUE* /*args*/, VALUE self)
{
    ICE_RUBY_TRY
    {
        throw RubyException(rb_eRuntimeError, "a proxy cannot be created via new");
    }
    ICE_RUBY_CATCH
    return Qnil;
}

void
IceRuby::initProxy(VALUE iceModule)
{
    //
    // ObjectPrx.
    //
    _proxyClass = rb_define_class_under(iceModule, "ObjectPrx", rb_cObject);

    //
    // Instance methods.
    //
    rb_define_method(_proxyClass, "ice_getCommunicator", CAST_METHOD(IceRuby_ObjectPrx_ice_getCommunicator), 0);
    rb_define_method(_proxyClass, "ice_toString", CAST_METHOD(IceRuby_ObjectPrx_ice_toString), 0);
    rb_define_method(_proxyClass, "ice_isA", CAST_METHOD(IceRuby_ObjectPrx_ice_isA), -1);
    rb_define_method(_proxyClass, "ice_ping", CAST_METHOD(IceRuby_ObjectPrx_ice_ping), -1);
    rb_define_method(_proxyClass, "ice_ids", CAST_METHOD(IceRuby_ObjectPrx_ice_ids), -1);
    rb_define_method(_proxyClass, "ice_id", CAST_METHOD(IceRuby_ObjectPrx_ice_id), -1);
    rb_define_method(_proxyClass, "ice_getIdentity", CAST_METHOD(IceRuby_ObjectPrx_ice_getIdentity), 0);
    rb_define_method(_proxyClass, "ice_identity", CAST_METHOD(IceRuby_ObjectPrx_ice_identity), 1);
    rb_define_method(_proxyClass, "ice_getContext", CAST_METHOD(IceRuby_ObjectPrx_ice_getContext), 0);
    rb_define_method(_proxyClass, "ice_context", CAST_METHOD(IceRuby_ObjectPrx_ice_context), 1);
    rb_define_method(_proxyClass, "ice_getFacet", CAST_METHOD(IceRuby_ObjectPrx_ice_getFacet), 0);
    rb_define_method(_proxyClass, "ice_facet", CAST_METHOD(IceRuby_ObjectPrx_ice_facet), 1);
    rb_define_method(_proxyClass, "ice_getAdapterId", CAST_METHOD(IceRuby_ObjectPrx_ice_getAdapterId), 0);
    rb_define_method(_proxyClass, "ice_adapterId", CAST_METHOD(IceRuby_ObjectPrx_ice_adapterId), 1);
    rb_define_method(_proxyClass, "ice_getEndpoints", CAST_METHOD(IceRuby_ObjectPrx_ice_getEndpoints), 0);
    rb_define_method(_proxyClass, "ice_endpoints", CAST_METHOD(IceRuby_ObjectPrx_ice_endpoints), 1);
    rb_define_method(_proxyClass, "ice_getLocatorCacheTimeout",
                     CAST_METHOD(IceRuby_ObjectPrx_ice_getLocatorCacheTimeout), 0);
    rb_define_method(_proxyClass, "ice_getInvocationTimeout",
                     CAST_METHOD(IceRuby_ObjectPrx_ice_getInvocationTimeout), 0);
    rb_define_method(_proxyClass, "ice_getConnectionId", CAST_METHOD(IceRuby_ObjectPrx_ice_getConnectionId), 0);
    rb_define_method(_proxyClass, "ice_locatorCacheTimeout", CAST_METHOD(IceRuby_ObjectPrx_ice_locatorCacheTimeout), 1);
    rb_define_method(_proxyClass, "ice_invocationTimeout", CAST_METHOD(IceRuby_ObjectPrx_ice_invocationTimeout), 1);
    rb_define_method(_proxyClass, "ice_isConnectionCached", CAST_METHOD(IceRuby_ObjectPrx_ice_isConnectionCached), 0);
    rb_define_method(_proxyClass, "ice_connectionCached", CAST_METHOD(IceRuby_ObjectPrx_ice_connectionCached), 1);
    rb_define_method(_proxyClass, "ice_getEndpointSelection",
                     CAST_METHOD(IceRuby_ObjectPrx_ice_getEndpointSelection), 0);
    rb_define_method(_proxyClass, "ice_endpointSelection", CAST_METHOD(IceRuby_ObjectPrx_ice_endpointSelection), 1);
    rb_define_method(_proxyClass, "ice_isSecure", CAST_METHOD(IceRuby_ObjectPrx_ice_isSecure), 0);
    rb_define_method(_proxyClass, "ice_secure", CAST_METHOD(IceRuby_ObjectPrx_ice_secure), 1);
    rb_define_method(_proxyClass, "ice_getEncodingVersion", CAST_METHOD(IceRuby_ObjectPrx_ice_getEncodingVersion), 0);
    rb_define_method(_proxyClass, "ice_encodingVersion", CAST_METHOD(IceRuby_ObjectPrx_ice_encodingVersion), 1);
    rb_define_method(_proxyClass, "ice_isPreferSecure", CAST_METHOD(IceRuby_ObjectPrx_ice_isPreferSecure), 0);
    rb_define_method(_proxyClass, "ice_preferSecure", CAST_METHOD(IceRuby_ObjectPrx_ice_preferSecure), 1);
    rb_define_method(_proxyClass, "ice_getRouter", CAST_METHOD(IceRuby_ObjectPrx_ice_getRouter), 0);
    rb_define_method(_proxyClass, "ice_router", CAST_METHOD(IceRuby_ObjectPrx_ice_router), 1);
    rb_define_method(_proxyClass, "ice_getLocator", CAST_METHOD(IceRuby_ObjectPrx_ice_getLocator), 0);
    rb_define_method(_proxyClass, "ice_locator", CAST_METHOD(IceRuby_ObjectPrx_ice_locator), 1);
    rb_define_method(_proxyClass, "ice_twoway", CAST_METHOD(IceRuby_ObjectPrx_ice_twoway), 0);
    rb_define_method(_proxyClass, "ice_isTwoway", CAST_METHOD(IceRuby_ObjectPrx_ice_isTwoway), 0);
    rb_define_method(_proxyClass, "ice_oneway", CAST_METHOD(IceRuby_ObjectPrx_ice_oneway), 0);
    rb_define_method(_proxyClass, "ice_isOneway", CAST_METHOD(IceRuby_ObjectPrx_ice_isOneway), 0);
    rb_define_method(_proxyClass, "ice_batchOneway", CAST_METHOD(IceRuby_ObjectPrx_ice_batchOneway), 0);
    rb_define_method(_proxyClass, "ice_isBatchOneway", CAST_METHOD(IceRuby_ObjectPrx_ice_isBatchOneway), 0);
    rb_define_method(_proxyClass, "ice_datagram", CAST_METHOD(IceRuby_ObjectPrx_ice_datagram), 0);
    rb_define_method(_proxyClass, "ice_isDatagram", CAST_METHOD(IceRuby_ObjectPrx_ice_isDatagram), 0);
    rb_define_method(_proxyClass, "ice_batchDatagram", CAST_METHOD(IceRuby_ObjectPrx_ice_batchDatagram), 0);
    rb_define_method(_proxyClass, "ice_isBatchDatagram", CAST_METHOD(IceRuby_ObjectPrx_ice_isBatchDatagram), 0);
    rb_define_method(_proxyClass, "ice_compress", CAST_METHOD(IceRuby_ObjectPrx_ice_compress), 1);
    rb_define_method(_proxyClass, "ice_timeout", CAST_METHOD(IceRuby_ObjectPrx_ice_timeout), 1);
    rb_define_method(_proxyClass, "ice_connectionId", CAST_METHOD(IceRuby_ObjectPrx_ice_connectionId), 1);
    rb_define_method(_proxyClass, "ice_getConnection", CAST_METHOD(IceRuby_ObjectPrx_ice_getConnection), 0);
    rb_define_method(_proxyClass, "ice_getCachedConnection", CAST_METHOD(IceRuby_ObjectPrx_ice_getCachedConnection), 0);
    rb_define_method(_proxyClass, "ice_flushBatchRequests", CAST_METHOD(IceRuby_ObjectPrx_ice_flushBatchRequests), 0);

    rb_define_method(_proxyClass, "hash", CAST_METHOD(IceRuby_ObjectPrx_hash), 0);
    rb_define_method(_proxyClass, "to_s", CAST_METHOD(IceRuby_ObjectPrx_ice_toString), 0);
    rb_define_method(_proxyClass, "inspect", CAST_METHOD(IceRuby_ObjectPrx_ice_toString), 0);
    rb_define_method(_proxyClass, "<=>", CAST_METHOD(IceRuby_ObjectPrx_cmp), 1);
    rb_define_method(_proxyClass, "==", CAST_METHOD(IceRuby_ObjectPrx_equals), 1);
    rb_define_method(_proxyClass, "eql?", CAST_METHOD(IceRuby_ObjectPrx_equals), 1);

    //
    // Static methods.
    //
    rb_define_singleton_method(_proxyClass, "checkedCast", CAST_METHOD(IceRuby_ObjectPrx_checkedCast), -1);
    rb_define_singleton_method(_proxyClass, "uncheckedCast", CAST_METHOD(IceRuby_ObjectPrx_uncheckedCast), -1);
    rb_define_singleton_method(_proxyClass, "ice_checkedCast", CAST_METHOD(IceRuby_ObjectPrx_ice_checkedCast), 4);
    rb_define_singleton_method(_proxyClass, "ice_uncheckedCast", CAST_METHOD(IceRuby_ObjectPrx_ice_uncheckedCast), 2);
    rb_define_singleton_method(_proxyClass, "ice_staticId", CAST_METHOD(IceRuby_ObjectPrx_ice_staticId), 0);
    rb_define_singleton_method(_proxyClass, "new", CAST_METHOD(IceRuby_ObjectPrx_new), -1);
}

VALUE
IceRuby::createProxy(const Ice::ObjectPrx& p, VALUE cls)
{
    //
    // If cls is nil then the proxy has the base type Ice::ObjectPrx.
    //
    if(NIL_P(cls))
    {
        return Data_Wrap_Struct(_proxyClass, IceRuby_ObjectPrx_mark, IceRuby_ObjectPrx_free, new Ice::ObjectPrx(p));
    }
    else
    {
        return Data_Wrap_Struct(cls, IceRuby_ObjectPrx_mark, IceRuby_ObjectPrx_free, new Ice::ObjectPrx(p));
    }
}

Ice::ObjectPrx
IceRuby::getProxy(VALUE v)
{
    Ice::ObjectPrx* p = reinterpret_cast<Ice::ObjectPrx*>(DATA_PTR(v));
    return *p;
}

bool
IceRuby::checkProxy(VALUE v)
{
    return callRuby(rb_obj_is_kind_of, v, _proxyClass) == Qtrue;
}
