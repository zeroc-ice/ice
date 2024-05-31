//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Connection.h"
#include "../../cpp/src/Ice/SSL/SSLUtil.h"
#include "Endpoint.h"
#include "Ice/Ice.h"
#include "Types.h"
#include "Util.h"

using namespace std;
using namespace IceRuby;

static VALUE _connectionClass;

static VALUE _connectionInfoClass;
static VALUE _ipConnectionInfoClass;
static VALUE _tcpConnectionInfoClass;
static VALUE _udpConnectionInfoClass;
static VALUE _wsConnectionInfoClass;
static VALUE _sslConnectionInfoClass;

// Connection

extern "C" void
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

extern "C" VALUE
IceRuby_Connection_close(VALUE self, VALUE mode)
{
    ICE_RUBY_TRY
    {
        Ice::ConnectionPtr* p = reinterpret_cast<Ice::ConnectionPtr*>(DATA_PTR(self));
        assert(p);

        volatile VALUE type = callRuby(rb_path2class, "Ice::ConnectionClose");
        if (callRuby(rb_obj_is_instance_of, mode, type) != Qtrue)
        {
            throw RubyException(
                rb_eTypeError,
                "value for 'mode' argument must be an enumerator of Ice::ConnectionClose");
        }
        volatile VALUE modeValue = callRuby(rb_funcall, mode, rb_intern("to_i"), 0);
        assert(TYPE(modeValue) == T_FIXNUM);
        Ice::ConnectionClose cc = static_cast<Ice::ConnectionClose>(FIX2LONG(modeValue));
        (*p)->close(cc);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_Connection_flushBatchRequests(VALUE self, VALUE compress)
{
    ICE_RUBY_TRY
    {
        Ice::ConnectionPtr* p = reinterpret_cast<Ice::ConnectionPtr*>(DATA_PTR(self));
        assert(p);

        volatile VALUE type = callRuby(rb_path2class, "Ice::CompressBatch");
        if (callRuby(rb_obj_is_instance_of, compress, type) != Qtrue)
        {
            throw RubyException(
                rb_eTypeError,
                "value for 'compress' argument must be an enumerator of Ice::CompressBatch");
        }
        volatile VALUE compressValue = callRuby(rb_funcall, compress, rb_intern("to_i"), 0);
        assert(TYPE(compressValue) == T_FIXNUM);
        Ice::CompressBatch cb = static_cast<Ice::CompressBatch>(FIX2LONG(compressValue));
        (*p)->flushBatchRequests(cb);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_Connection_heartbeat(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ConnectionPtr* p = reinterpret_cast<Ice::ConnectionPtr*>(DATA_PTR(self));
        assert(p);

        (*p)->heartbeat();
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
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

extern "C" VALUE
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

extern "C" VALUE
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

extern "C" VALUE
IceRuby_Connection_setBufferSize(VALUE self, VALUE r, VALUE s)
{
    ICE_RUBY_TRY
    {
        Ice::ConnectionPtr* p = reinterpret_cast<Ice::ConnectionPtr*>(DATA_PTR(self));
        assert(p);

        int rcvSize = static_cast<int>(getInteger(r));
        int sndSize = static_cast<int>(getInteger(s));

        (*p)->setBufferSize(rcvSize, sndSize);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_Connection_throwException(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::ConnectionPtr* p = reinterpret_cast<Ice::ConnectionPtr*>(DATA_PTR(self));
        assert(p);

        (*p)->throwException();
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
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

extern "C" VALUE
IceRuby_Connection_equals(VALUE self, VALUE other)
{
    ICE_RUBY_TRY
    {
        if (NIL_P(other))
        {
            return Qfalse;
        }
        if (callRuby(rb_obj_is_kind_of, other, _connectionClass) != Qtrue)
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

// ConnectionInfo

extern "C" void
IceRuby_ConnectionInfo_free(Ice::ConnectionInfoPtr* p)
{
    assert(p);
    delete p;
}

VALUE
IceRuby::createConnectionInfo(const Ice::ConnectionInfoPtr& p)
{
    if (!p)
    {
        return Qnil;
    }

    VALUE info;
    if (dynamic_pointer_cast<Ice::WSConnectionInfo>(p))
    {
        info = Data_Wrap_Struct(_wsConnectionInfoClass, 0, IceRuby_ConnectionInfo_free, new Ice::ConnectionInfoPtr(p));

        Ice::WSConnectionInfoPtr ws = dynamic_pointer_cast<Ice::WSConnectionInfo>(p);
        volatile VALUE result = callRuby(rb_hash_new);
        for (Ice::HeaderDict::const_iterator q = ws->headers.begin(); q != ws->headers.end(); ++q)
        {
            volatile VALUE key = createString(q->first);
            volatile VALUE value = createString(q->second);
            callRuby(rb_hash_aset, result, key, value);
        }
        rb_ivar_set(info, rb_intern("@headers"), result);
    }
    else if (dynamic_pointer_cast<Ice::TCPConnectionInfo>(p))
    {
        info = Data_Wrap_Struct(_tcpConnectionInfoClass, 0, IceRuby_ConnectionInfo_free, new Ice::ConnectionInfoPtr(p));

        Ice::TCPConnectionInfoPtr tcp = dynamic_pointer_cast<Ice::TCPConnectionInfo>(p);
        rb_ivar_set(info, rb_intern("@rcvSize"), INT2FIX(tcp->rcvSize));
        rb_ivar_set(info, rb_intern("@sndSize"), INT2FIX(tcp->sndSize));
    }
    else if (dynamic_pointer_cast<Ice::UDPConnectionInfo>(p))
    {
        info = Data_Wrap_Struct(_udpConnectionInfoClass, 0, IceRuby_ConnectionInfo_free, new Ice::ConnectionInfoPtr(p));

        Ice::UDPConnectionInfoPtr udp = dynamic_pointer_cast<Ice::UDPConnectionInfo>(p);
        rb_ivar_set(info, rb_intern("@mcastAddress"), createString(udp->mcastAddress));
        rb_ivar_set(info, rb_intern("@mcastPort"), INT2FIX(udp->mcastPort));
        rb_ivar_set(info, rb_intern("@rcvSize"), INT2FIX(udp->rcvSize));
        rb_ivar_set(info, rb_intern("@sndSize"), INT2FIX(udp->sndSize));
    }
    else if (dynamic_pointer_cast<Ice::SSL::ConnectionInfo>(p))
    {
        info = Data_Wrap_Struct(_sslConnectionInfoClass, 0, IceRuby_ConnectionInfo_free, new Ice::ConnectionInfoPtr(p));

        Ice::SSL::ConnectionInfoPtr ssl = dynamic_pointer_cast<Ice::SSL::ConnectionInfo>(p);
        string encoded;
        if (ssl->peerCertificate)
        {
            encoded = Ice::SSL::encodeCertificate(ssl->peerCertificate);
        }
        rb_ivar_set(info, rb_intern("@peerCertificate"), createString(encoded));
    }
    else if (dynamic_pointer_cast<Ice::IPConnectionInfo>(p))
    {
        info = Data_Wrap_Struct(_ipConnectionInfoClass, 0, IceRuby_ConnectionInfo_free, new Ice::ConnectionInfoPtr(p));
    }
    else
    {
        info = Data_Wrap_Struct(_connectionInfoClass, 0, IceRuby_ConnectionInfo_free, new Ice::ConnectionInfoPtr(p));
    }

    if (dynamic_pointer_cast<Ice::IPConnectionInfo>(p))
    {
        Ice::IPConnectionInfoPtr ip = dynamic_pointer_cast<Ice::IPConnectionInfo>(p);
        rb_ivar_set(info, rb_intern("@localAddress"), createString(ip->localAddress));
        rb_ivar_set(info, rb_intern("@localPort"), INT2FIX(ip->localPort));
        rb_ivar_set(info, rb_intern("@remoteAddress"), createString(ip->remoteAddress));
        rb_ivar_set(info, rb_intern("@remotePort"), INT2FIX(ip->remotePort));
    }

    rb_ivar_set(info, rb_intern("@underlying"), createConnectionInfo(p->underlying));
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
    rb_undef_alloc_func(_connectionClass);

    //
    // Instance methods.
    //
    rb_define_method(_connectionClass, "close", CAST_METHOD(IceRuby_Connection_close), 1);
    rb_define_method(_connectionClass, "flushBatchRequests", CAST_METHOD(IceRuby_Connection_flushBatchRequests), 1);
    rb_define_method(_connectionClass, "heartbeat", CAST_METHOD(IceRuby_Connection_heartbeat), 0);
    rb_define_method(_connectionClass, "type", CAST_METHOD(IceRuby_Connection_type), 0);
    rb_define_method(_connectionClass, "getInfo", CAST_METHOD(IceRuby_Connection_getInfo), 0);
    rb_define_method(_connectionClass, "getEndpoint", CAST_METHOD(IceRuby_Connection_getEndpoint), 0);
    rb_define_method(_connectionClass, "setBufferSize", CAST_METHOD(IceRuby_Connection_setBufferSize), 2);
    rb_define_method(_connectionClass, "throwException", CAST_METHOD(IceRuby_Connection_throwException), 0);
    rb_define_method(_connectionClass, "toString", CAST_METHOD(IceRuby_Connection_toString), 0);
    rb_define_method(_connectionClass, "to_s", CAST_METHOD(IceRuby_Connection_toString), 0);
    rb_define_method(_connectionClass, "inspect", CAST_METHOD(IceRuby_Connection_toString), 0);
    rb_define_method(_connectionClass, "==", CAST_METHOD(IceRuby_Connection_equals), 1);
    rb_define_method(_connectionClass, "eql?", CAST_METHOD(IceRuby_Connection_equals), 1);

    //
    // ConnectionInfo.
    //
    _connectionInfoClass = rb_define_class_under(iceModule, "ConnectionInfo", rb_cObject);
    rb_undef_alloc_func(_connectionInfoClass);

    //
    // Instance members.
    //
    rb_define_attr(_connectionInfoClass, "incoming", 1, 0);
    rb_define_attr(_connectionInfoClass, "adapterName", 1, 0);
    rb_define_attr(_connectionInfoClass, "rcvSize", 1, 0);
    rb_define_attr(_connectionInfoClass, "sndSize", 1, 0);

    //
    // IPConnectionInfo
    //
    _ipConnectionInfoClass = rb_define_class_under(iceModule, "IPConnectionInfo", _connectionInfoClass);
    rb_undef_alloc_func(_ipConnectionInfoClass);

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
    rb_undef_alloc_func(_tcpConnectionInfoClass);

    //
    // UDPConnectionInfo
    //
    _udpConnectionInfoClass = rb_define_class_under(iceModule, "UDPConnectionInfo", _ipConnectionInfoClass);
    rb_undef_alloc_func(_udpConnectionInfoClass);

    //
    // Instance members.
    //
    rb_define_attr(_udpConnectionInfoClass, "mcastAddress", 1, 0);
    rb_define_attr(_udpConnectionInfoClass, "mcastPort", 1, 0);

    //
    // WSConnectionInfo
    //
    _wsConnectionInfoClass = rb_define_class_under(iceModule, "WSConnectionInfo", _connectionInfoClass);
    rb_undef_alloc_func(_wsConnectionInfoClass);

    //
    // Instance members.
    //
    rb_define_attr(_wsConnectionInfoClass, "headers", 1, 0);

    //
    // SSLConnectionInfo
    //
    _sslConnectionInfoClass = rb_define_class_under(iceModule, "SSLConnectionInfo", _connectionInfoClass);
    rb_undef_alloc_func(_sslConnectionInfoClass);

    //
    // Instance members.
    //
    rb_define_attr(_sslConnectionInfoClass, "peerCertificate", 1, 0);
}

Ice::ConnectionPtr
IceRuby::getConnection(VALUE v)
{
    Ice::ConnectionPtr* p = reinterpret_cast<Ice::ConnectionPtr*>(DATA_PTR(v));
    return *p;
}

bool
IceRuby::checkConnection(VALUE v)
{
    return callRuby(rb_obj_is_kind_of, v, _connectionClass) == Qtrue;
}
