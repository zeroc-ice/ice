// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <ConnectionInfo.h>
#include <Util.h>
#include <Ice/Connection.h>

using namespace std;
using namespace IceRuby;

static VALUE _connectionInfoClass;
static VALUE _tcpConnectionInfoClass;
static VALUE _udpConnectionInfoClass;

static VALUE _endpointInfoClass;
static VALUE _tcpEndpointInfoClass;
static VALUE _udpEndpointInfoClass;
static VALUE _opaqueEndpointInfoClass;

// **********************************************************************
// ConnectionInfo
// **********************************************************************

extern "C"
void
IceRuby_ConnectionInfo_free(Ice::ConnectionPtr* p)
{
    assert(p);
    delete p;
}

VALUE
IceRuby::createConnectionInfo(const Ice::ConnectionInfoPtr& p)
{
    VALUE info;
    if(Ice::TcpConnectionInfoPtr::dynamicCast(p))
    {
        info = Data_Wrap_Struct(_tcpConnectionInfoClass, 0, IceRuby_ConnectionInfo_free, new Ice::ConnectionInfoPtr(p));

        Ice::TcpConnectionInfoPtr tcp = Ice::TcpConnectionInfoPtr::dynamicCast(p);
        rb_ivar_set(info, rb_intern("@localAddress"), createString(tcp->localAddress));
        rb_ivar_set(info, rb_intern("@localPort"), INT2FIX(tcp->localPort));
        rb_ivar_set(info, rb_intern("@remoteAddress"), createString(tcp->remoteAddress));
        rb_ivar_set(info, rb_intern("@remotePort"), INT2FIX(tcp->remotePort));
    }
    else if(Ice::UdpConnectionInfoPtr::dynamicCast(p))
    {
        info = Data_Wrap_Struct(_udpConnectionInfoClass, 0, IceRuby_ConnectionInfo_free, new Ice::ConnectionInfoPtr(p));

        Ice::UdpConnectionInfoPtr udp = Ice::UdpConnectionInfoPtr::dynamicCast(p);
        rb_ivar_set(info, rb_intern("@localAddress"), createString(udp->localAddress));
        rb_ivar_set(info, rb_intern("@localPort"), INT2FIX(udp->localPort));
        rb_ivar_set(info, rb_intern("@remoteAddress"), createString(udp->remoteAddress));
        rb_ivar_set(info, rb_intern("@remotePort"), INT2FIX(udp->remotePort));
        rb_ivar_set(info, rb_intern("@mcastAddress"), createString(udp->mcastAddress));
        rb_ivar_set(info, rb_intern("@mcastPort"), INT2FIX(udp->mcastPort));
    }
    else
    {
        info = Data_Wrap_Struct(_connectionInfoClass, 0, IceRuby_ConnectionInfo_free, new Ice::ConnectionInfoPtr(p));
    }
    rb_ivar_set(info, rb_intern("@endpoint"), createEndpointInfo(p->endpoint));
    return info;
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
    if(Ice::TcpEndpointInfoPtr::dynamicCast(p))
    {
        info = Data_Wrap_Struct(_tcpEndpointInfoClass, 0, IceRuby_EndpointInfo_free, new Ice::EndpointInfoPtr(p));

        Ice::TcpEndpointInfoPtr tcp = Ice::TcpEndpointInfoPtr::dynamicCast(p);
        rb_ivar_set(info, rb_intern("@host"), createString(tcp->host));
        rb_ivar_set(info, rb_intern("@port"), INT2FIX(tcp->port));
    }
    else if(Ice::UdpEndpointInfoPtr::dynamicCast(p))
    {
        info = Data_Wrap_Struct(_udpEndpointInfoClass, 0, IceRuby_EndpointInfo_free, new Ice::EndpointInfoPtr(p));

        Ice::UdpEndpointInfoPtr udp = Ice::UdpEndpointInfoPtr::dynamicCast(p);
        rb_ivar_set(info, rb_intern("@host"), createString(udp->host));
        rb_ivar_set(info, rb_intern("@port"), INT2FIX(udp->port));
        rb_ivar_set(info, rb_intern("@protocolMajor"), CHR2FIX(udp->protocolMajor));
        rb_ivar_set(info, rb_intern("@protocolMinor"), CHR2FIX(udp->protocolMinor));
        rb_ivar_set(info, rb_intern("@encodingMajor"), CHR2FIX(udp->encodingMajor));
        rb_ivar_set(info, rb_intern("@encodingMinor"), CHR2FIX(udp->encodingMinor));
        rb_ivar_set(info, rb_intern("@mcastInterface"), createString(udp->mcastInterface));
        rb_ivar_set(info, rb_intern("@mcastTtl"), INT2FIX(udp->mcastTtl));
    }
    else if(Ice::OpaqueEndpointInfoPtr::dynamicCast(p))
    {
        info = Data_Wrap_Struct(_opaqueEndpointInfoClass, 0, IceRuby_EndpointInfo_free, new Ice::EndpointInfoPtr(p));

        Ice::OpaqueEndpointInfoPtr opaque = Ice::OpaqueEndpointInfoPtr::dynamicCast(p);
        Ice::ByteSeq b = opaque->rawBytes;
        VALUE v = callRuby(rb_str_new, reinterpret_cast<const char*>(&b[0]), static_cast<long>(b.size()));
        rb_ivar_set(info, rb_intern("@rawBytes"), v);
    }
    else
    {
        info = Data_Wrap_Struct(_endpointInfoClass, 0, IceRuby_EndpointInfo_free, new Ice::EndpointInfoPtr(p));
    }
    rb_ivar_set(info, rb_intern("@timeout"), INT2FIX(p->timeout));
    rb_ivar_set(info, rb_intern("@compress"), p->compress ? Qtrue : Qfalse);
    return info;
}

//
// Ice::Endpoint::type
//
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

//
// Ice::Endpoint::datagram
//
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

//
// Ice::Endpoint::secure
//
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
IceRuby::initConnectionInfo(VALUE iceModule)
{
    //
    // ConnectionInfo.
    //
    _connectionInfoClass = rb_define_class_under(iceModule, "ConnectionInfo", rb_cObject);

    //
    // Instance members. 
    //
    rb_define_attr(_connectionInfoClass, "endpoint", 1, 0); 

    //
    // TcpConnectionInfo
    //
    _tcpConnectionInfoClass = rb_define_class_under(iceModule, "TcpConnectionInfo", _connectionInfoClass);

    //
    // Instance members. 
    //
    rb_define_attr(_tcpConnectionInfoClass, "localAddress", 1, 0); 
    rb_define_attr(_tcpConnectionInfoClass, "localPort", 1, 0); 
    rb_define_attr(_tcpConnectionInfoClass, "remoteAddress", 1, 0); 
    rb_define_attr(_tcpConnectionInfoClass, "remotePort", 1, 0); 

    //
    // UdpConnectionInfo
    //
    _udpConnectionInfoClass = rb_define_class_under(iceModule, "UdpConnectionInfo", _connectionInfoClass);

    //
    // Instance members. 
    //
    rb_define_attr(_udpConnectionInfoClass, "localAddress", 1, 0); 
    rb_define_attr(_udpConnectionInfoClass, "localPort", 1, 0); 
    rb_define_attr(_udpConnectionInfoClass, "remoteAddress", 1, 0); 
    rb_define_attr(_udpConnectionInfoClass, "remotePort", 1, 0); 
    rb_define_attr(_udpConnectionInfoClass, "mcastAddress", 1, 0); 
    rb_define_attr(_udpConnectionInfoClass, "mcastPort", 1, 0); 

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
    rb_define_attr(_endpointInfoClass, "timeout", 1, 0); 
    rb_define_attr(_endpointInfoClass, "compress", 1, 0); 

    //
    // TcpEndpointInfo
    //
    _tcpEndpointInfoClass = rb_define_class_under(iceModule, "TcpEndpointInfo", _endpointInfoClass);

    //
    // Instance members. 
    //
    rb_define_attr(_tcpEndpointInfoClass, "host", 1, 0); 
    rb_define_attr(_tcpEndpointInfoClass, "port", 1, 0); 

    //
    // UdpEndpointInfo
    //
    _udpEndpointInfoClass = rb_define_class_under(iceModule, "UdpEndpointInfo", _endpointInfoClass);

    //
    // Instance members. 
    //
    rb_define_attr(_udpEndpointInfoClass, "host", 1, 0); 
    rb_define_attr(_udpEndpointInfoClass, "port", 1, 0); 
    rb_define_attr(_udpEndpointInfoClass, "protocolMajor", 1, 0); 
    rb_define_attr(_udpEndpointInfoClass, "protocolMinor", 1, 0); 
    rb_define_attr(_udpEndpointInfoClass, "encodingMajor", 1, 0); 
    rb_define_attr(_udpEndpointInfoClass, "encodingMinor", 1, 0); 
    rb_define_attr(_udpEndpointInfoClass, "mcastInterface", 1, 0); 
    rb_define_attr(_udpEndpointInfoClass, "mcastTtl", 1, 0); 

    //
    // OpaqueEndpointInfo
    //
    _opaqueEndpointInfoClass = rb_define_class_under(iceModule, "OpaqueEndpointInfo", _endpointInfoClass);

    //
    // Instance members. 
    //
    rb_define_attr(_opaqueEndpointInfoClass, "rawBytes", 1, 0); 
}
