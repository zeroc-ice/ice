// Copyright (c) ZeroC, Inc.

#include "Endpoint.h"
#include "Ice/Ice.h"
#include "Ice/TargetCompare.h"
#include "Util.h"

using namespace std;
using namespace IceRuby;

static VALUE _endpointClass;

static VALUE _endpointInfoClass;
static VALUE _ipEndpointInfoClass;
static VALUE _tcpEndpointInfoClass;
static VALUE _udpEndpointInfoClass;
static VALUE _wsEndpointInfoClass;
static VALUE _opaqueEndpointInfoClass;
static VALUE _sslEndpointInfoClass;

// Endpoint

extern "C" void
IceRuby_Endpoint_free(void* p)
{
    delete static_cast<Ice::EndpointPtr*>(p);
}

static const rb_data_type_t IceRuby_EndpointType = {
    .wrap_struct_name = "Ice::Endpoint",
    .function =
        {
            .dfree = IceRuby_Endpoint_free,
        },
    .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

VALUE
IceRuby::createEndpoint(const Ice::EndpointPtr& p)
{
    return TypedData_Wrap_Struct(_endpointClass, &IceRuby_EndpointType, new Ice::EndpointPtr(p));
}

extern "C" VALUE
IceRuby_Endpoint_toString(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::EndpointPtr* p = reinterpret_cast<Ice::EndpointPtr*>(DATA_PTR(self));
        assert(p);

        string s = (*p)->toString();
        return createString(s);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_Endpoint_getInfo(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::EndpointPtr* p = reinterpret_cast<Ice::EndpointPtr*>(DATA_PTR(self));
        assert(p);

        Ice::EndpointInfoPtr info = (*p)->getInfo();
        return createEndpointInfo(info);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_Endpoint_cmp(VALUE self, VALUE other)
{
    ICE_RUBY_TRY
    {
        if (NIL_P(other))
        {
            return INT2NUM(1);
        }
        if (!checkEndpoint(other))
        {
            throw RubyException(rb_eTypeError, "argument must be a endpoint");
        }
        Ice::EndpointPtr p1 = Ice::EndpointPtr(*reinterpret_cast<Ice::EndpointPtr*>(DATA_PTR(self)));
        Ice::EndpointPtr p2 = Ice::EndpointPtr(*reinterpret_cast<Ice::EndpointPtr*>(DATA_PTR(other)));
        if (Ice::targetLess(p1, p2))
        {
            return INT2NUM(-1);
        }
        else if (Ice::targetEqualTo(p1, p1))
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

extern "C" VALUE
IceRuby_Endpoint_equals(VALUE self, VALUE other)
{
    return IceRuby_Endpoint_cmp(self, other) == INT2NUM(0) ? Qtrue : Qfalse;
}

// EndpointInfo

extern "C" void
IceRuby_EndpointInfo_free(void* p)
{
    delete static_cast<Ice::EndpointInfoPtr*>(p);
}

static const rb_data_type_t IceRuby_EndpointInfoType = {
    .wrap_struct_name = "Ice::EndpointInfo",
    .function =
        {
            .dfree = IceRuby_EndpointInfo_free,
        },
    .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

VALUE
IceRuby::createEndpointInfo(const Ice::EndpointInfoPtr& p)
{
    if (!p)
    {
        return Qnil;
    }

    VALUE info;
    if (dynamic_pointer_cast<Ice::WSEndpointInfo>(p))
    {
        info = TypedData_Wrap_Struct(_wsEndpointInfoClass, &IceRuby_EndpointInfoType, new Ice::EndpointInfoPtr(p));

        Ice::WSEndpointInfoPtr ws = dynamic_pointer_cast<Ice::WSEndpointInfo>(p);
        rb_ivar_set(info, rb_intern("@resource"), createString(ws->resource));
    }
    else if (dynamic_pointer_cast<Ice::TCPEndpointInfo>(p))
    {
        info = TypedData_Wrap_Struct(_tcpEndpointInfoClass, &IceRuby_EndpointInfoType, new Ice::EndpointInfoPtr(p));
    }
    else if (dynamic_pointer_cast<Ice::UDPEndpointInfo>(p))
    {
        info = TypedData_Wrap_Struct(_udpEndpointInfoClass, &IceRuby_EndpointInfoType, new Ice::EndpointInfoPtr(p));

        Ice::UDPEndpointInfoPtr udp = dynamic_pointer_cast<Ice::UDPEndpointInfo>(p);
        rb_ivar_set(info, rb_intern("@mcastInterface"), createString(udp->mcastInterface));
        rb_ivar_set(info, rb_intern("@mcastTtl"), INT2FIX(udp->mcastTtl));
    }
    else if (dynamic_pointer_cast<Ice::OpaqueEndpointInfo>(p))
    {
        info = TypedData_Wrap_Struct(_opaqueEndpointInfoClass, &IceRuby_EndpointInfoType, new Ice::EndpointInfoPtr(p));

        Ice::OpaqueEndpointInfoPtr opaque = dynamic_pointer_cast<Ice::OpaqueEndpointInfo>(p);
        auto b = opaque->rawBytes;
        volatile VALUE v = callRuby(rb_str_new, reinterpret_cast<const char*>(&b[0]), static_cast<long>(b.size()));
        rb_ivar_set(info, rb_intern("@rawBytes"), v);
        rb_ivar_set(info, rb_intern("@rawEncoding"), createEncodingVersion(opaque->rawEncoding));
    }
    else if (dynamic_pointer_cast<Ice::SSL::EndpointInfo>(p))
    {
        info = TypedData_Wrap_Struct(_sslEndpointInfoClass, &IceRuby_EndpointInfoType, new Ice::EndpointInfoPtr(p));
    }
    else if (dynamic_pointer_cast<Ice::IPEndpointInfo>(p))
    {
        info = TypedData_Wrap_Struct(_ipEndpointInfoClass, &IceRuby_EndpointInfoType, new Ice::EndpointInfoPtr(p));
    }
    else
    {
        info = TypedData_Wrap_Struct(_endpointInfoClass, &IceRuby_EndpointInfoType, new Ice::EndpointInfoPtr(p));
    }

    if (dynamic_pointer_cast<Ice::IPEndpointInfo>(p))
    {
        Ice::IPEndpointInfoPtr ip = dynamic_pointer_cast<Ice::IPEndpointInfo>(p);
        rb_ivar_set(info, rb_intern("@host"), createString(ip->host));
        rb_ivar_set(info, rb_intern("@port"), INT2FIX(ip->port));
        rb_ivar_set(info, rb_intern("@sourceAddress"), createString(ip->sourceAddress));
    }

    rb_ivar_set(info, rb_intern("@underlying"), createEndpointInfo(p->underlying));
    rb_ivar_set(info, rb_intern("@timeout"), INT2FIX(p->timeout));
    rb_ivar_set(info, rb_intern("@compress"), p->compress ? Qtrue : Qfalse);
    return info;
}

extern "C" VALUE
IceRuby_EndpointInfo_type(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::EndpointInfoPtr* p = reinterpret_cast<Ice::EndpointInfoPtr*>(DATA_PTR(self));
        assert(p);

        int16_t type = (*p)->type();
        return INT2FIX(type);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_EndpointInfo_datagram(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::EndpointInfoPtr* p = reinterpret_cast<Ice::EndpointInfoPtr*>(DATA_PTR(self));
        assert(p);

        bool result = (*p)->datagram();
        return result ? Qtrue : Qfalse;
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_EndpointInfo_secure(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::EndpointInfoPtr* p = reinterpret_cast<Ice::EndpointInfoPtr*>(DATA_PTR(self));
        assert(p);

        bool result = (*p)->secure();
        return result ? Qtrue : Qfalse;
    }
    ICE_RUBY_CATCH
    return Qnil;
}

void
IceRuby::initEndpoint(VALUE iceModule)
{
    //
    // Endpoint.
    //
    _endpointClass = rb_define_class_under(iceModule, "Endpoint", rb_cObject);
    rb_undef_alloc_func(_endpointClass);

    //
    // Instance methods.
    //
    rb_define_method(_endpointClass, "toString", CAST_METHOD(IceRuby_Endpoint_toString), 0);
    rb_define_method(_endpointClass, "getInfo", CAST_METHOD(IceRuby_Endpoint_getInfo), 0);
    rb_define_method(_endpointClass, "to_s", CAST_METHOD(IceRuby_Endpoint_toString), 0);
    rb_define_method(_endpointClass, "inspect", CAST_METHOD(IceRuby_Endpoint_toString), 0);
    rb_define_method(_endpointClass, "<=>", CAST_METHOD(IceRuby_Endpoint_cmp), 1);
    rb_define_method(_endpointClass, "==", CAST_METHOD(IceRuby_Endpoint_equals), 1);
    rb_define_method(_endpointClass, "eql?", CAST_METHOD(IceRuby_Endpoint_equals), 1);

    //
    // EndpointInfo.
    //
    _endpointInfoClass = rb_define_class_under(iceModule, "EndpointInfo", rb_cObject);
    rb_undef_alloc_func(_endpointInfoClass);

    //
    // Instance methods.
    //
    rb_define_method(_endpointInfoClass, "type", CAST_METHOD(IceRuby_EndpointInfo_type), 0);
    rb_define_method(_endpointInfoClass, "datagram", CAST_METHOD(IceRuby_EndpointInfo_datagram), 0);
    rb_define_method(_endpointInfoClass, "secure", CAST_METHOD(IceRuby_EndpointInfo_secure), 0);

    //
    // Instance members.
    //
    rb_define_attr(_endpointInfoClass, "underlying", 1, 0);
    rb_define_attr(_endpointInfoClass, "timeout", 1, 0);
    rb_define_attr(_endpointInfoClass, "compress", 1, 0);

    //
    // IPEndpointInfo
    //
    _ipEndpointInfoClass = rb_define_class_under(iceModule, "IPEndpointInfo", _endpointInfoClass);
    rb_undef_alloc_func(_ipEndpointInfoClass);

    //
    // Instance members.
    //
    rb_define_attr(_ipEndpointInfoClass, "host", 1, 0);
    rb_define_attr(_ipEndpointInfoClass, "port", 1, 0);
    rb_define_attr(_ipEndpointInfoClass, "sourceAddress", 1, 0);

    //
    // TCPEndpointInfo
    //
    _tcpEndpointInfoClass = rb_define_class_under(iceModule, "TCPEndpointInfo", _ipEndpointInfoClass);
    rb_undef_alloc_func(_tcpEndpointInfoClass);

    //
    // UDPEndpointInfo
    //
    _udpEndpointInfoClass = rb_define_class_under(iceModule, "UDPEndpointInfo", _ipEndpointInfoClass);
    rb_undef_alloc_func(_udpEndpointInfoClass);

    //
    // Instance members.
    //
    rb_define_attr(_udpEndpointInfoClass, "mcastInterface", 1, 0);
    rb_define_attr(_udpEndpointInfoClass, "mcastTtl", 1, 0);

    //
    // WSEndpointInfo
    //
    _wsEndpointInfoClass = rb_define_class_under(iceModule, "WSEndpointInfo", _endpointInfoClass);
    rb_undef_alloc_func(_wsEndpointInfoClass);

    //
    // Instance members.
    //
    rb_define_attr(_wsEndpointInfoClass, "resource", 1, 0);

    //
    // OpaqueEndpointInfo
    //
    _opaqueEndpointInfoClass = rb_define_class_under(iceModule, "OpaqueEndpointInfo", _endpointInfoClass);
    rb_undef_alloc_func(_opaqueEndpointInfoClass);

    //
    // Instance members.
    //
    rb_define_attr(_opaqueEndpointInfoClass, "rawBytes", 1, 0);
    rb_define_attr(_opaqueEndpointInfoClass, "rawEncoding", 1, 0);

    //
    // SSLEndpointInfo
    //
    _sslEndpointInfoClass = rb_define_class_under(iceModule, "SSLEndpointInfo", _endpointInfoClass);
    rb_undef_alloc_func(_sslEndpointInfoClass);
}

bool
IceRuby::checkEndpoint(VALUE v)
{
    return callRuby(rb_obj_is_kind_of, v, _endpointClass) == Qtrue;
}
