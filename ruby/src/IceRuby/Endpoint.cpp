// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Endpoint.h>
#include <Util.h>
#include <Ice/Object.h>
#include <IceSSL/EndpointInfo.h>

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
static VALUE _wssEndpointInfoClass;

// **********************************************************************
// Endpoint
// **********************************************************************

extern "C"
void
IceRuby_Endpoint_free(Ice::EndpointPtr* p)
{
    assert(p);
    delete p;
}

VALUE
IceRuby::createEndpoint(const Ice::EndpointPtr& p)
{
    return Data_Wrap_Struct(_endpointClass, 0, IceRuby_Endpoint_free, new Ice::EndpointPtr(p));
}

extern "C"
VALUE
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

extern "C"
VALUE
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

extern "C"
VALUE
IceRuby_Endpoint_cmp(VALUE self, VALUE other)
{
    ICE_RUBY_TRY
    {
        if(NIL_P(other))
        {
            return INT2NUM(1);
        }
        if(!checkEndpoint(other))
        {
            throw RubyException(rb_eTypeError, "argument must be a endpoint");
        }
        Ice::EndpointPtr p1 = Ice::EndpointPtr(*reinterpret_cast<Ice::EndpointPtr*>(DATA_PTR(self)));
        Ice::EndpointPtr p2 = Ice::EndpointPtr(*reinterpret_cast<Ice::EndpointPtr*>(DATA_PTR(other)));
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
IceRuby_Endpoint_equals(VALUE self, VALUE other)
{
    return IceRuby_Endpoint_cmp(self, other) == INT2NUM(0) ? Qtrue : Qfalse;
}


// **********************************************************************
// EndpointInfo
// **********************************************************************

extern "C"
void
IceRuby_EndpointInfo_free(Ice::EndpointPtr* p)
{
    assert(p);
    delete p;
}

VALUE
IceRuby::createEndpointInfo(const Ice::EndpointInfoPtr& p)
{
    VALUE info;
    if(Ice::WSEndpointInfoPtr::dynamicCast(p))
    {
        info = Data_Wrap_Struct(_wsEndpointInfoClass, 0, IceRuby_EndpointInfo_free, new Ice::EndpointInfoPtr(p));

        Ice::WSEndpointInfoPtr ws = Ice::WSEndpointInfoPtr::dynamicCast(p);
        rb_ivar_set(info, rb_intern("@host"), createString(ws->host));
        rb_ivar_set(info, rb_intern("@port"), INT2FIX(ws->port));
        rb_ivar_set(info, rb_intern("@sourceAddress"), createString(ws->sourceAddress));
        rb_ivar_set(info, rb_intern("@resource"), createString(ws->resource));
    }
    else if(Ice::TCPEndpointInfoPtr::dynamicCast(p))
    {
        info = Data_Wrap_Struct(_tcpEndpointInfoClass, 0, IceRuby_EndpointInfo_free, new Ice::EndpointInfoPtr(p));

        Ice::TCPEndpointInfoPtr tcp = Ice::TCPEndpointInfoPtr::dynamicCast(p);
        rb_ivar_set(info, rb_intern("@host"), createString(tcp->host));
        rb_ivar_set(info, rb_intern("@port"), INT2FIX(tcp->port));
        rb_ivar_set(info, rb_intern("@sourceAddress"), createString(tcp->sourceAddress));
    }
    else if(Ice::UDPEndpointInfoPtr::dynamicCast(p))
    {
        info = Data_Wrap_Struct(_udpEndpointInfoClass, 0, IceRuby_EndpointInfo_free, new Ice::EndpointInfoPtr(p));

        Ice::UDPEndpointInfoPtr udp = Ice::UDPEndpointInfoPtr::dynamicCast(p);
        rb_ivar_set(info, rb_intern("@host"), createString(udp->host));
        rb_ivar_set(info, rb_intern("@port"), INT2FIX(udp->port));
        rb_ivar_set(info, rb_intern("@sourceAddress"), createString(udp->sourceAddress));
        rb_ivar_set(info, rb_intern("@mcastInterface"), createString(udp->mcastInterface));
        rb_ivar_set(info, rb_intern("@mcastTtl"), INT2FIX(udp->mcastTtl));
    }
    else if(Ice::OpaqueEndpointInfoPtr::dynamicCast(p))
    {
        info = Data_Wrap_Struct(_opaqueEndpointInfoClass, 0, IceRuby_EndpointInfo_free, new Ice::EndpointInfoPtr(p));

        Ice::OpaqueEndpointInfoPtr opaque = Ice::OpaqueEndpointInfoPtr::dynamicCast(p);
        Ice::ByteSeq b = opaque->rawBytes;
        volatile VALUE v = callRuby(rb_str_new, reinterpret_cast<const char*>(&b[0]), static_cast<long>(b.size()));
        rb_ivar_set(info, rb_intern("@rawBytes"), v);
        rb_ivar_set(info, rb_intern("@rawEncoding"), createEncodingVersion(opaque->rawEncoding));
    }
    else if(IceSSL::WSSEndpointInfoPtr::dynamicCast(p))
    {
        info = Data_Wrap_Struct(_wssEndpointInfoClass, 0, IceRuby_EndpointInfo_free, new Ice::EndpointInfoPtr(p));

        IceSSL::WSSEndpointInfoPtr wss = IceSSL::WSSEndpointInfoPtr::dynamicCast(p);
        rb_ivar_set(info, rb_intern("@host"), createString(wss->host));
        rb_ivar_set(info, rb_intern("@port"), INT2FIX(wss->port));
        rb_ivar_set(info, rb_intern("@sourceAddress"), createString(wss->sourceAddress));
        rb_ivar_set(info, rb_intern("@resource"), createString(wss->resource));
    }
    else if(IceSSL::EndpointInfoPtr::dynamicCast(p))
    {
        info = Data_Wrap_Struct(_sslEndpointInfoClass, 0, IceRuby_EndpointInfo_free, new Ice::EndpointInfoPtr(p));

        IceSSL::EndpointInfoPtr ssl = IceSSL::EndpointInfoPtr::dynamicCast(p);
        rb_ivar_set(info, rb_intern("@host"), createString(ssl->host));
        rb_ivar_set(info, rb_intern("@port"), INT2FIX(ssl->port));
        rb_ivar_set(info, rb_intern("@sourceAddress"), createString(ssl->sourceAddress));
    }
    else if(Ice::IPEndpointInfoPtr::dynamicCast(p))
    {
        info = Data_Wrap_Struct(_ipEndpointInfoClass, 0, IceRuby_EndpointInfo_free, new Ice::EndpointInfoPtr(p));

        Ice::IPEndpointInfoPtr ip = Ice::IPEndpointInfoPtr::dynamicCast(p);
        rb_ivar_set(info, rb_intern("@host"), createString(ip->host));
        rb_ivar_set(info, rb_intern("@port"), INT2FIX(ip->port));
        rb_ivar_set(info, rb_intern("@sourceAddress"), createString(ip->sourceAddress));
    }
    else
    {
        info = Data_Wrap_Struct(_endpointInfoClass, 0, IceRuby_EndpointInfo_free, new Ice::EndpointInfoPtr(p));
    }

    rb_ivar_set(info, rb_intern("@timeout"), INT2FIX(p->timeout));
    rb_ivar_set(info, rb_intern("@compress"), p->compress ? Qtrue : Qfalse);
    return info;
}

extern "C"
VALUE
IceRuby_EndpointInfo_type(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::EndpointInfoPtr* p = reinterpret_cast<Ice::EndpointInfoPtr*>(DATA_PTR(self));
        assert(p);

        Ice::Short type = (*p)->type();
        return INT2FIX(type);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
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

extern "C"
VALUE
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

    //
    // Instance methods.
    //
    rb_define_method(_endpointInfoClass, "type", CAST_METHOD(IceRuby_EndpointInfo_type), 0);
    rb_define_method(_endpointInfoClass, "datagram", CAST_METHOD(IceRuby_EndpointInfo_datagram), 0);
    rb_define_method(_endpointInfoClass, "secure", CAST_METHOD(IceRuby_EndpointInfo_secure), 0);

    //
    // Instance members.
    //
    rb_define_attr(_endpointInfoClass, "protocol", 1, 0);
    rb_define_attr(_endpointInfoClass, "encoding", 1, 0);
    rb_define_attr(_endpointInfoClass, "timeout", 1, 0);
    rb_define_attr(_endpointInfoClass, "compress", 1, 0);

    //
    // IPEndpointInfo
    //
    _ipEndpointInfoClass = rb_define_class_under(iceModule, "IPEndpointInfo", _endpointInfoClass);

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

    //
    // UDPEndpointInfo
    //
    _udpEndpointInfoClass = rb_define_class_under(iceModule, "UDPEndpointInfo", _ipEndpointInfoClass);

    //
    // Instance members.
    //
    rb_define_attr(_udpEndpointInfoClass, "mcastInterface", 1, 0);
    rb_define_attr(_udpEndpointInfoClass, "mcastTtl", 1, 0);

    //
    // WSEndpointInfo
    //
    _wsEndpointInfoClass = rb_define_class_under(iceModule, "WSEndpointInfo", _tcpEndpointInfoClass);

    //
    // Instance members.
    //
    rb_define_attr(_wsEndpointInfoClass, "resource", 1, 0);

    //
    // OpaqueEndpointInfo
    //
    _opaqueEndpointInfoClass = rb_define_class_under(iceModule, "OpaqueEndpointInfo", _endpointInfoClass);

    //
    // Instance members.
    //
    rb_define_attr(_opaqueEndpointInfoClass, "rawBytes", 1, 0);
    rb_define_attr(_opaqueEndpointInfoClass, "rawEncoding", 1, 0);

    //
    // SSLEndpointInfo
    //
    _sslEndpointInfoClass = rb_define_class_under(iceModule, "SSLEndpointInfo", _ipEndpointInfoClass);

    //
    // WSSEndpointInfo
    //
    _wssEndpointInfoClass = rb_define_class_under(iceModule, "WSSEndpointInfo", _sslEndpointInfoClass);

    //
    // Instance members.
    //
    rb_define_attr(_wssEndpointInfoClass, "resource", 1, 0);
}

bool
IceRuby::checkEndpoint(VALUE v)
{
    return callRuby(rb_obj_is_kind_of, v, _endpointClass) == Qtrue;
}
