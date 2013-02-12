// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Connection.h>
#include <Endpoint.h>
#include <Util.h>
#include <Ice/Object.h>

using namespace std;
using namespace IceRuby;

static VALUE _connectionClass;

static VALUE _connectionInfoClass;
static VALUE _ipConnectionInfoClass;
static VALUE _tcpConnectionInfoClass;
static VALUE _udpConnectionInfoClass;

// **********************************************************************
// Connection
// **********************************************************************

extern "C"
void
IceRuby_Connection_free(Ice::ConnectionPtr* p)
{
    assert(p);
    delete p;
}

VALUE
IceRuby::createConnection(const Ice::ConnectionPtr& p)
{
    return Data_Wrap_Struct(_connectionClass, 0, IceRuby_Connection_free, new Ice::ConnectionPtr(p));
}

extern "C"
VALUE
IceRuby_Connection_close(VALUE self, VALUE b)
{
    ICE_RUBY_TRY
    {
        Ice::ConnectionPtr* p = reinterpret_cast<Ice::ConnectionPtr*>(DATA_PTR(self));
        assert(p);

        (*p)->close(RTEST(b));
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_Connection_flushBatchRequests(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ConnectionPtr* p = reinterpret_cast<Ice::ConnectionPtr*>(DATA_PTR(self));
        assert(p);

        (*p)->flushBatchRequests();
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_Connection_type(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ConnectionPtr* p = reinterpret_cast<Ice::ConnectionPtr*>(DATA_PTR(self));
        assert(p);

        string s = (*p)->type();
        return createString(s);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_Connection_timeout(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ConnectionPtr* p = reinterpret_cast<Ice::ConnectionPtr*>(DATA_PTR(self));
        assert(p);

        Ice::Int timeout = (*p)->timeout();
        return INT2FIX(timeout);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_Connection_getInfo(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ConnectionPtr* p = reinterpret_cast<Ice::ConnectionPtr*>(DATA_PTR(self));
        assert(p);

        Ice::ConnectionInfoPtr info = (*p)->getInfo();
        return createConnectionInfo(info);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_Connection_getEndpoint(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ConnectionPtr* p = reinterpret_cast<Ice::ConnectionPtr*>(DATA_PTR(self));
        assert(p);

        Ice::EndpointPtr endpoint = (*p)->getEndpoint();
        return createEndpoint(endpoint);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_Connection_toString(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ConnectionPtr* p = reinterpret_cast<Ice::ConnectionPtr*>(DATA_PTR(self));
        assert(p);

        string s = (*p)->toString();
        return createString(s);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_Connection_equals(VALUE self, VALUE other)
{
    ICE_RUBY_TRY
    {
        if(NIL_P(other))
        {
            return Qfalse;
        }
        if(callRuby(rb_obj_is_kind_of, other, _connectionClass) != Qtrue)
        {
            throw RubyException(rb_eTypeError, "argument must be a connection");
        }
        Ice::ConnectionPtr* p1 = reinterpret_cast<Ice::ConnectionPtr*>(DATA_PTR(self));
        Ice::ConnectionPtr* p2 = reinterpret_cast<Ice::ConnectionPtr*>(DATA_PTR(other));
        return *p1 == *p2 ? Qtrue : Qfalse;
    }
    ICE_RUBY_CATCH
    return Qnil;
}

// **********************************************************************
// ConnectionInfo
// **********************************************************************

extern "C"
void
IceRuby_ConnectionInfo_free(Ice::ConnectionInfoPtr* p)
{
    assert(p);
    delete p;
}

VALUE
IceRuby::createConnectionInfo(const Ice::ConnectionInfoPtr& p)
{
    VALUE info;
    if(Ice::TCPConnectionInfoPtr::dynamicCast(p))
    {
        info = Data_Wrap_Struct(_tcpConnectionInfoClass, 0, IceRuby_ConnectionInfo_free, new Ice::ConnectionInfoPtr(p));

        Ice::TCPConnectionInfoPtr tcp = Ice::TCPConnectionInfoPtr::dynamicCast(p);
        rb_ivar_set(info, rb_intern("@localAddress"), createString(tcp->localAddress));
        rb_ivar_set(info, rb_intern("@localPort"), INT2FIX(tcp->localPort));
        rb_ivar_set(info, rb_intern("@remoteAddress"), createString(tcp->remoteAddress));
        rb_ivar_set(info, rb_intern("@remotePort"), INT2FIX(tcp->remotePort));
    }
    else if(Ice::UDPConnectionInfoPtr::dynamicCast(p))
    {
        info = Data_Wrap_Struct(_udpConnectionInfoClass, 0, IceRuby_ConnectionInfo_free, new Ice::ConnectionInfoPtr(p));

        Ice::UDPConnectionInfoPtr udp = Ice::UDPConnectionInfoPtr::dynamicCast(p);
        rb_ivar_set(info, rb_intern("@localAddress"), createString(udp->localAddress));
        rb_ivar_set(info, rb_intern("@localPort"), INT2FIX(udp->localPort));
        rb_ivar_set(info, rb_intern("@remoteAddress"), createString(udp->remoteAddress));
        rb_ivar_set(info, rb_intern("@remotePort"), INT2FIX(udp->remotePort));
        rb_ivar_set(info, rb_intern("@mcastAddress"), createString(udp->mcastAddress));
        rb_ivar_set(info, rb_intern("@mcastPort"), INT2FIX(udp->mcastPort));
    }
    else if(Ice::IPConnectionInfoPtr::dynamicCast(p))
    {
        info = Data_Wrap_Struct(_ipConnectionInfoClass, 0, IceRuby_ConnectionInfo_free, new Ice::ConnectionInfoPtr(p));

        Ice::IPConnectionInfoPtr ip = Ice::IPConnectionInfoPtr::dynamicCast(p);
        rb_ivar_set(info, rb_intern("@localAddress"), createString(ip->localAddress));
        rb_ivar_set(info, rb_intern("@localPort"), INT2FIX(ip->localPort));
        rb_ivar_set(info, rb_intern("@remoteAddress"), createString(ip->remoteAddress));
        rb_ivar_set(info, rb_intern("@remotePort"), INT2FIX(ip->remotePort));
    }
    else
    {
        info = Data_Wrap_Struct(_connectionInfoClass, 0, IceRuby_ConnectionInfo_free, new Ice::ConnectionInfoPtr(p));
    }
    rb_ivar_set(info, rb_intern("@incoming"), p->incoming ? Qtrue : Qfalse);
    rb_ivar_set(info, rb_intern("@adapterName"), createString(p->adapterName));
    return info;
}

void
IceRuby::initConnection(VALUE iceModule)
{
    //
    // Connection.
    //
    _connectionClass = rb_define_class_under(iceModule, "ConnectionI", rb_cObject);

    //
    // Instance methods.
    //
    rb_define_method(_connectionClass, "close", CAST_METHOD(IceRuby_Connection_close), 1);
    rb_define_method(_connectionClass, "flushBatchRequests", CAST_METHOD(IceRuby_Connection_flushBatchRequests), 0);
    rb_define_method(_connectionClass, "type", CAST_METHOD(IceRuby_Connection_type), 0);
    rb_define_method(_connectionClass, "timeout", CAST_METHOD(IceRuby_Connection_timeout), 0);
    rb_define_method(_connectionClass, "getInfo", CAST_METHOD(IceRuby_Connection_getInfo), 0);
    rb_define_method(_connectionClass, "getEndpoint", CAST_METHOD(IceRuby_Connection_getEndpoint), 0);
    rb_define_method(_connectionClass, "toString", CAST_METHOD(IceRuby_Connection_toString), 0);
    rb_define_method(_connectionClass, "to_s", CAST_METHOD(IceRuby_Connection_toString), 0);
    rb_define_method(_connectionClass, "inspect", CAST_METHOD(IceRuby_Connection_toString), 0);
    rb_define_method(_connectionClass, "==", CAST_METHOD(IceRuby_Connection_equals), 1);
    rb_define_method(_connectionClass, "eql?", CAST_METHOD(IceRuby_Connection_equals), 1);

    //
    // ConnectionInfo.
    //
    _connectionInfoClass = rb_define_class_under(iceModule, "ConnectionInfo", rb_cObject);

    //
    // Instance members. 
    //
    rb_define_attr(_connectionInfoClass, "incoming", 1, 0); 
    rb_define_attr(_connectionInfoClass, "adapterName", 1, 0); 

    //
    // IPConnectionInfo
    //
    _ipConnectionInfoClass = rb_define_class_under(iceModule, "IPConnectionInfo", _connectionInfoClass);

    //
    // Instance members. 
    //
    rb_define_attr(_ipConnectionInfoClass, "localAddress", 1, 0); 
    rb_define_attr(_ipConnectionInfoClass, "localPort", 1, 0); 
    rb_define_attr(_ipConnectionInfoClass, "remoteAddress", 1, 0); 
    rb_define_attr(_ipConnectionInfoClass, "remotePort", 1, 0); 

    //
    // TCPConnectionInfo
    //
    _tcpConnectionInfoClass = rb_define_class_under(iceModule, "TCPConnectionInfo", _ipConnectionInfoClass);

    //
    // UDPConnectionInfo
    //
    _udpConnectionInfoClass = rb_define_class_under(iceModule, "UDPConnectionInfo", _ipConnectionInfoClass);

    //
    // Instance members. 
    //
    rb_define_attr(_udpConnectionInfoClass, "mcastAddress", 1, 0); 
    rb_define_attr(_udpConnectionInfoClass, "mcastPort", 1, 0); 
}
