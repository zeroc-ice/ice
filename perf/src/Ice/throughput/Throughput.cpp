// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Ice version 2.1.0
// Generated from file `Throughput.ice'

#include <Throughput.h>
#include <Ice/LocalException.h>
#include <Ice/ObjectFactory.h>
#include <Ice/BasicStream.h>
#include <Ice/Object.h>

#ifndef ICE_IGNORE_VERSION
#   if ICE_INT_VERSION != 20100
#       error Ice version mismatch!
#   endif
#endif

void
IceInternal::incRef(::Demo::Throughput* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::Demo::Throughput* p)
{
    p->__decRef();
}

void
IceInternal::incRef(::IceProxy::Demo::Throughput* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::IceProxy::Demo::Throughput* p)
{
    p->__decRef();
}

void
Demo::__write(::IceInternal::BasicStream* __os, const ::Demo::ThroughputPrx& v)
{
    __os->write(::Ice::ObjectPrx(v));
}

void
Demo::__read(::IceInternal::BasicStream* __is, ::Demo::ThroughputPrx& v)
{
    ::Ice::ObjectPrx proxy;
    __is->read(proxy);
    if(!proxy)
    {
	v = 0;
    }
    else
    {
	v = new ::IceProxy::Demo::Throughput;
	v->__copyFrom(proxy);
    }
}

void
Demo::__write(::IceInternal::BasicStream* __os, const ::Demo::ThroughputPtr& v)
{
    __os->write(::Ice::ObjectPtr(v));
}

bool
Demo::StringDouble::operator==(const StringDouble& __rhs) const
{
    return !operator!=(__rhs);
}

bool
Demo::StringDouble::operator!=(const StringDouble& __rhs) const
{
    if(this == &__rhs)
    {
	return false;
    }
    if(s != __rhs.s)
    {
	return true;
    }
    if(d != __rhs.d)
    {
	return true;
    }
    return false;
}

bool
Demo::StringDouble::operator<(const StringDouble& __rhs) const
{
    if(this == &__rhs)
    {
	return false;
    }
    if(s < __rhs.s)
    {
	return true;
    }
    else if(__rhs.s < s)
    {
	return false;
    }
    if(d < __rhs.d)
    {
	return true;
    }
    else if(__rhs.d < d)
    {
	return false;
    }
    return false;
}

void
Demo::StringDouble::__write(::IceInternal::BasicStream* __os) const
{
    __os->write(s);
    __os->write(d);
}

void
Demo::StringDouble::__read(::IceInternal::BasicStream* __is)
{
    __is->read(s);
    __is->read(d);
}

void
Demo::__write(::IceInternal::BasicStream* __os, const ::Demo::StringDoubleSeq& v, ::Demo::__U__StringDoubleSeq)
{
    __os->writeSize(::Ice::Int(v.size()));
    ::Demo::StringDoubleSeq::const_iterator p;
    for(p = v.begin(); p != v.end(); ++p)
    {
	(*p).__write(__os);
    }
}

void
Demo::__read(::IceInternal::BasicStream* __is, ::Demo::StringDoubleSeq& v, ::Demo::__U__StringDoubleSeq)
{
    ::Ice::Int sz;
    __is->readSize(sz);
    __is->startSeq(sz, 9);
    v.resize(sz);
    for(int i = 0; i < sz; ++i)
    {
	v[i].__read(__is);
	__is->checkSeq();
	__is->endElement();
    }
    __is->endSeq(sz);
}

void
IceProxy::Demo::Throughput::sendByteSeq(const ::Demo::ByteSeq& seq)
{
    sendByteSeq(seq, __defaultContext());
}

void
IceProxy::Demo::Throughput::sendByteSeq(const ::Demo::ByteSeq& seq, const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::Demo::Throughput* __del = dynamic_cast< ::IceDelegate::Demo::Throughput*>(__delBase.get());
	    __del->sendByteSeq(seq, __ctx);
	    return;
	}
	catch(const ::IceInternal::NonRepeatable& __ex)
	{
	    __rethrowException(*__ex.get());
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

::Demo::ByteSeq
IceProxy::Demo::Throughput::recvByteSeq()
{
    return recvByteSeq(__defaultContext());
}

::Demo::ByteSeq
IceProxy::Demo::Throughput::recvByteSeq(const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    __checkTwowayOnly("recvByteSeq");
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::Demo::Throughput* __del = dynamic_cast< ::IceDelegate::Demo::Throughput*>(__delBase.get());
	    return __del->recvByteSeq(__ctx);
	}
	catch(const ::IceInternal::NonRepeatable& __ex)
	{
	    __rethrowException(*__ex.get());
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

::Demo::ByteSeq
IceProxy::Demo::Throughput::echoByteSeq(const ::Demo::ByteSeq& seq)
{
    return echoByteSeq(seq, __defaultContext());
}

::Demo::ByteSeq
IceProxy::Demo::Throughput::echoByteSeq(const ::Demo::ByteSeq& seq, const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    __checkTwowayOnly("echoByteSeq");
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::Demo::Throughput* __del = dynamic_cast< ::IceDelegate::Demo::Throughput*>(__delBase.get());
	    return __del->echoByteSeq(seq, __ctx);
	}
	catch(const ::IceInternal::NonRepeatable& __ex)
	{
	    __rethrowException(*__ex.get());
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

void
IceProxy::Demo::Throughput::sendStringSeq(const ::Demo::StringSeq& seq)
{
    sendStringSeq(seq, __defaultContext());
}

void
IceProxy::Demo::Throughput::sendStringSeq(const ::Demo::StringSeq& seq, const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::Demo::Throughput* __del = dynamic_cast< ::IceDelegate::Demo::Throughput*>(__delBase.get());
	    __del->sendStringSeq(seq, __ctx);
	    return;
	}
	catch(const ::IceInternal::NonRepeatable& __ex)
	{
	    __rethrowException(*__ex.get());
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

::Demo::StringSeq
IceProxy::Demo::Throughput::recvStringSeq()
{
    return recvStringSeq(__defaultContext());
}

::Demo::StringSeq
IceProxy::Demo::Throughput::recvStringSeq(const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    __checkTwowayOnly("recvStringSeq");
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::Demo::Throughput* __del = dynamic_cast< ::IceDelegate::Demo::Throughput*>(__delBase.get());
	    return __del->recvStringSeq(__ctx);
	}
	catch(const ::IceInternal::NonRepeatable& __ex)
	{
	    __rethrowException(*__ex.get());
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

::Demo::StringSeq
IceProxy::Demo::Throughput::echoStringSeq(const ::Demo::StringSeq& seq)
{
    return echoStringSeq(seq, __defaultContext());
}

::Demo::StringSeq
IceProxy::Demo::Throughput::echoStringSeq(const ::Demo::StringSeq& seq, const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    __checkTwowayOnly("echoStringSeq");
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::Demo::Throughput* __del = dynamic_cast< ::IceDelegate::Demo::Throughput*>(__delBase.get());
	    return __del->echoStringSeq(seq, __ctx);
	}
	catch(const ::IceInternal::NonRepeatable& __ex)
	{
	    __rethrowException(*__ex.get());
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

void
IceProxy::Demo::Throughput::sendStructSeq(const ::Demo::StringDoubleSeq& seq)
{
    sendStructSeq(seq, __defaultContext());
}

void
IceProxy::Demo::Throughput::sendStructSeq(const ::Demo::StringDoubleSeq& seq, const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::Demo::Throughput* __del = dynamic_cast< ::IceDelegate::Demo::Throughput*>(__delBase.get());
	    __del->sendStructSeq(seq, __ctx);
	    return;
	}
	catch(const ::IceInternal::NonRepeatable& __ex)
	{
	    __rethrowException(*__ex.get());
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

::Demo::StringDoubleSeq
IceProxy::Demo::Throughput::recvStructSeq()
{
    return recvStructSeq(__defaultContext());
}

::Demo::StringDoubleSeq
IceProxy::Demo::Throughput::recvStructSeq(const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    __checkTwowayOnly("recvStructSeq");
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::Demo::Throughput* __del = dynamic_cast< ::IceDelegate::Demo::Throughput*>(__delBase.get());
	    return __del->recvStructSeq(__ctx);
	}
	catch(const ::IceInternal::NonRepeatable& __ex)
	{
	    __rethrowException(*__ex.get());
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

::Demo::StringDoubleSeq
IceProxy::Demo::Throughput::echoStructSeq(const ::Demo::StringDoubleSeq& seq)
{
    return echoStructSeq(seq, __defaultContext());
}

::Demo::StringDoubleSeq
IceProxy::Demo::Throughput::echoStructSeq(const ::Demo::StringDoubleSeq& seq, const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    __checkTwowayOnly("echoStructSeq");
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::Demo::Throughput* __del = dynamic_cast< ::IceDelegate::Demo::Throughput*>(__delBase.get());
	    return __del->echoStructSeq(seq, __ctx);
	}
	catch(const ::IceInternal::NonRepeatable& __ex)
	{
	    __rethrowException(*__ex.get());
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

void
IceProxy::Demo::Throughput::shutdown()
{
    shutdown(__defaultContext());
}

void
IceProxy::Demo::Throughput::shutdown(const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::Demo::Throughput* __del = dynamic_cast< ::IceDelegate::Demo::Throughput*>(__delBase.get());
	    __del->shutdown(__ctx);
	    return;
	}
	catch(const ::IceInternal::NonRepeatable& __ex)
	{
	    __handleException(*__ex.get(), __cnt);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

const ::std::string&
IceProxy::Demo::Throughput::ice_staticId()
{
    return ::Demo::Throughput::ice_staticId();
}

::IceInternal::Handle< ::IceDelegateM::Ice::Object>
IceProxy::Demo::Throughput::__createDelegateM()
{
    return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::Demo::Throughput);
}

::IceInternal::Handle< ::IceDelegateD::Ice::Object>
IceProxy::Demo::Throughput::__createDelegateD()
{
    return ::IceInternal::Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD::Demo::Throughput);
}

bool
IceProxy::Demo::operator==(const ::IceProxy::Demo::Throughput& l, const ::IceProxy::Demo::Throughput& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) == static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::Demo::operator!=(const ::IceProxy::Demo::Throughput& l, const ::IceProxy::Demo::Throughput& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) != static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::Demo::operator<(const ::IceProxy::Demo::Throughput& l, const ::IceProxy::Demo::Throughput& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) < static_cast<const ::IceProxy::Ice::Object&>(r);
}

void
IceDelegateM::Demo::Throughput::sendByteSeq(const ::Demo::ByteSeq& seq, const ::Ice::Context& __context)
{
    static const ::std::string __operation("sendByteSeq");
    ::IceInternal::Outgoing __out(__connection.get(), __reference.get(), __operation, static_cast< ::Ice::OperationMode>(0), __context, __compress);
    try
    {
	::IceInternal::BasicStream* __os = __out.os();
	__os->write(seq);
    }
    catch(const ::Ice::LocalException& __ex)
    {
	__out.abort(__ex);
    }
    bool __ok = __out.invoke();
    try
    {
	::IceInternal::BasicStream* __is = __out.is();
	if(!__ok)
	{
	    __is->throwException();
	}
    }
    catch(const ::Ice::UserException&)
    {
	throw ::Ice::UnknownUserException(__FILE__, __LINE__);
    }
    catch(const ::Ice::LocalException& __ex)
    {
	throw ::IceInternal::NonRepeatable(__ex);
    }
}

::Demo::ByteSeq
IceDelegateM::Demo::Throughput::recvByteSeq(const ::Ice::Context& __context)
{
    static const ::std::string __operation("recvByteSeq");
    ::IceInternal::Outgoing __out(__connection.get(), __reference.get(), __operation, static_cast< ::Ice::OperationMode>(0), __context, __compress);
    bool __ok = __out.invoke();
    try
    {
	::IceInternal::BasicStream* __is = __out.is();
	if(!__ok)
	{
	    __is->throwException();
	}
	::Demo::ByteSeq __ret;
	__is->read(__ret);
	return __ret;
    }
    catch(const ::Ice::UserException&)
    {
	throw ::Ice::UnknownUserException(__FILE__, __LINE__);
    }
    catch(const ::Ice::LocalException& __ex)
    {
	throw ::IceInternal::NonRepeatable(__ex);
    }
}

::Demo::ByteSeq
IceDelegateM::Demo::Throughput::echoByteSeq(const ::Demo::ByteSeq& seq, const ::Ice::Context& __context)
{
    static const ::std::string __operation("echoByteSeq");
    ::IceInternal::Outgoing __out(__connection.get(), __reference.get(), __operation, static_cast< ::Ice::OperationMode>(0), __context, __compress);
    try
    {
	::IceInternal::BasicStream* __os = __out.os();
	__os->write(seq);
    }
    catch(const ::Ice::LocalException& __ex)
    {
	__out.abort(__ex);
    }
    bool __ok = __out.invoke();
    try
    {
	::IceInternal::BasicStream* __is = __out.is();
	if(!__ok)
	{
	    __is->throwException();
	}
	::Demo::ByteSeq __ret;
	__is->read(__ret);
	return __ret;
    }
    catch(const ::Ice::UserException&)
    {
	throw ::Ice::UnknownUserException(__FILE__, __LINE__);
    }
    catch(const ::Ice::LocalException& __ex)
    {
	throw ::IceInternal::NonRepeatable(__ex);
    }
}

void
IceDelegateM::Demo::Throughput::sendStringSeq(const ::Demo::StringSeq& seq, const ::Ice::Context& __context)
{
    static const ::std::string __operation("sendStringSeq");
    ::IceInternal::Outgoing __out(__connection.get(), __reference.get(), __operation, static_cast< ::Ice::OperationMode>(0), __context, __compress);
    try
    {
	::IceInternal::BasicStream* __os = __out.os();
	__os->write(seq);
    }
    catch(const ::Ice::LocalException& __ex)
    {
	__out.abort(__ex);
    }
    bool __ok = __out.invoke();
    try
    {
	::IceInternal::BasicStream* __is = __out.is();
	if(!__ok)
	{
	    __is->throwException();
	}
    }
    catch(const ::Ice::UserException&)
    {
	throw ::Ice::UnknownUserException(__FILE__, __LINE__);
    }
    catch(const ::Ice::LocalException& __ex)
    {
	throw ::IceInternal::NonRepeatable(__ex);
    }
}

::Demo::StringSeq
IceDelegateM::Demo::Throughput::recvStringSeq(const ::Ice::Context& __context)
{
    static const ::std::string __operation("recvStringSeq");
    ::IceInternal::Outgoing __out(__connection.get(), __reference.get(), __operation, static_cast< ::Ice::OperationMode>(0), __context, __compress);
    bool __ok = __out.invoke();
    try
    {
	::IceInternal::BasicStream* __is = __out.is();
	if(!__ok)
	{
	    __is->throwException();
	}
	::Demo::StringSeq __ret;
	__is->read(__ret);
	return __ret;
    }
    catch(const ::Ice::UserException&)
    {
	throw ::Ice::UnknownUserException(__FILE__, __LINE__);
    }
    catch(const ::Ice::LocalException& __ex)
    {
	throw ::IceInternal::NonRepeatable(__ex);
    }
}

::Demo::StringSeq
IceDelegateM::Demo::Throughput::echoStringSeq(const ::Demo::StringSeq& seq, const ::Ice::Context& __context)
{
    static const ::std::string __operation("echoStringSeq");
    ::IceInternal::Outgoing __out(__connection.get(), __reference.get(), __operation, static_cast< ::Ice::OperationMode>(0), __context, __compress);
    try
    {
	::IceInternal::BasicStream* __os = __out.os();
	__os->write(seq);
    }
    catch(const ::Ice::LocalException& __ex)
    {
	__out.abort(__ex);
    }
    bool __ok = __out.invoke();
    try
    {
	::IceInternal::BasicStream* __is = __out.is();
	if(!__ok)
	{
	    __is->throwException();
	}
	::Demo::StringSeq __ret;
	__is->read(__ret);
	return __ret;
    }
    catch(const ::Ice::UserException&)
    {
	throw ::Ice::UnknownUserException(__FILE__, __LINE__);
    }
    catch(const ::Ice::LocalException& __ex)
    {
	throw ::IceInternal::NonRepeatable(__ex);
    }
}

void
IceDelegateM::Demo::Throughput::sendStructSeq(const ::Demo::StringDoubleSeq& seq, const ::Ice::Context& __context)
{
    static const ::std::string __operation("sendStructSeq");
    ::IceInternal::Outgoing __out(__connection.get(), __reference.get(), __operation, static_cast< ::Ice::OperationMode>(0), __context, __compress);
    try
    {
	::IceInternal::BasicStream* __os = __out.os();
	::Demo::__write(__os, seq, ::Demo::__U__StringDoubleSeq());
    }
    catch(const ::Ice::LocalException& __ex)
    {
	__out.abort(__ex);
    }
    bool __ok = __out.invoke();
    try
    {
	::IceInternal::BasicStream* __is = __out.is();
	if(!__ok)
	{
	    __is->throwException();
	}
    }
    catch(const ::Ice::UserException&)
    {
	throw ::Ice::UnknownUserException(__FILE__, __LINE__);
    }
    catch(const ::Ice::LocalException& __ex)
    {
	throw ::IceInternal::NonRepeatable(__ex);
    }
}

::Demo::StringDoubleSeq
IceDelegateM::Demo::Throughput::recvStructSeq(const ::Ice::Context& __context)
{
    static const ::std::string __operation("recvStructSeq");
    ::IceInternal::Outgoing __out(__connection.get(), __reference.get(), __operation, static_cast< ::Ice::OperationMode>(0), __context, __compress);
    bool __ok = __out.invoke();
    try
    {
	::IceInternal::BasicStream* __is = __out.is();
	if(!__ok)
	{
	    __is->throwException();
	}
	::Demo::StringDoubleSeq __ret;
	::Demo::__read(__is, __ret, ::Demo::__U__StringDoubleSeq());
	return __ret;
    }
    catch(const ::Ice::UserException&)
    {
	throw ::Ice::UnknownUserException(__FILE__, __LINE__);
    }
    catch(const ::Ice::LocalException& __ex)
    {
	throw ::IceInternal::NonRepeatable(__ex);
    }
}

::Demo::StringDoubleSeq
IceDelegateM::Demo::Throughput::echoStructSeq(const ::Demo::StringDoubleSeq& seq, const ::Ice::Context& __context)
{
    static const ::std::string __operation("echoStructSeq");
    ::IceInternal::Outgoing __out(__connection.get(), __reference.get(), __operation, static_cast< ::Ice::OperationMode>(0), __context, __compress);
    try
    {
	::IceInternal::BasicStream* __os = __out.os();
	::Demo::__write(__os, seq, ::Demo::__U__StringDoubleSeq());
    }
    catch(const ::Ice::LocalException& __ex)
    {
	__out.abort(__ex);
    }
    bool __ok = __out.invoke();
    try
    {
	::IceInternal::BasicStream* __is = __out.is();
	if(!__ok)
	{
	    __is->throwException();
	}
	::Demo::StringDoubleSeq __ret;
	::Demo::__read(__is, __ret, ::Demo::__U__StringDoubleSeq());
	return __ret;
    }
    catch(const ::Ice::UserException&)
    {
	throw ::Ice::UnknownUserException(__FILE__, __LINE__);
    }
    catch(const ::Ice::LocalException& __ex)
    {
	throw ::IceInternal::NonRepeatable(__ex);
    }
}

void
IceDelegateM::Demo::Throughput::shutdown(const ::Ice::Context& __context)
{
    static const ::std::string __operation("shutdown");
    ::IceInternal::Outgoing __out(__connection.get(), __reference.get(), __operation, static_cast< ::Ice::OperationMode>(2), __context, __compress);
    bool __ok = __out.invoke();
    try
    {
	::IceInternal::BasicStream* __is = __out.is();
	if(!__ok)
	{
	    __is->throwException();
	}
    }
    catch(const ::Ice::UserException&)
    {
	throw ::Ice::UnknownUserException(__FILE__, __LINE__);
    }
    catch(const ::Ice::LocalException& __ex)
    {
	throw ::IceInternal::NonRepeatable(__ex);
    }
}

void
IceDelegateD::Demo::Throughput::sendByteSeq(const ::Demo::ByteSeq& seq, const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, "sendByteSeq", static_cast< ::Ice::OperationMode>(0), __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::Demo::Throughput* __servant = dynamic_cast< ::Demo::Throughput*>(__direct.servant().get());
	if(!__servant)
	{
	    ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
	    __opEx.id = __current.id;
	    __opEx.facet = __current.facet;
	    __opEx.operation = __current.operation;
	    throw __opEx;
	}
	try
	{
	    __servant->sendByteSeq(seq, __current);
	    return;
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::NonRepeatable(__ex);
	}
    }
}

::Demo::ByteSeq
IceDelegateD::Demo::Throughput::recvByteSeq(const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, "recvByteSeq", static_cast< ::Ice::OperationMode>(0), __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::Demo::Throughput* __servant = dynamic_cast< ::Demo::Throughput*>(__direct.servant().get());
	if(!__servant)
	{
	    ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
	    __opEx.id = __current.id;
	    __opEx.facet = __current.facet;
	    __opEx.operation = __current.operation;
	    throw __opEx;
	}
	try
	{
	    return __servant->recvByteSeq(__current);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::NonRepeatable(__ex);
	}
    }
}

::Demo::ByteSeq
IceDelegateD::Demo::Throughput::echoByteSeq(const ::Demo::ByteSeq& seq, const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, "echoByteSeq", static_cast< ::Ice::OperationMode>(0), __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::Demo::Throughput* __servant = dynamic_cast< ::Demo::Throughput*>(__direct.servant().get());
	if(!__servant)
	{
	    ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
	    __opEx.id = __current.id;
	    __opEx.facet = __current.facet;
	    __opEx.operation = __current.operation;
	    throw __opEx;
	}
	try
	{
	    return __servant->echoByteSeq(seq, __current);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::NonRepeatable(__ex);
	}
    }
}

void
IceDelegateD::Demo::Throughput::sendStringSeq(const ::Demo::StringSeq& seq, const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, "sendStringSeq", static_cast< ::Ice::OperationMode>(0), __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::Demo::Throughput* __servant = dynamic_cast< ::Demo::Throughput*>(__direct.servant().get());
	if(!__servant)
	{
	    ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
	    __opEx.id = __current.id;
	    __opEx.facet = __current.facet;
	    __opEx.operation = __current.operation;
	    throw __opEx;
	}
	try
	{
	    __servant->sendStringSeq(seq, __current);
	    return;
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::NonRepeatable(__ex);
	}
    }
}

::Demo::StringSeq
IceDelegateD::Demo::Throughput::recvStringSeq(const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, "recvStringSeq", static_cast< ::Ice::OperationMode>(0), __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::Demo::Throughput* __servant = dynamic_cast< ::Demo::Throughput*>(__direct.servant().get());
	if(!__servant)
	{
	    ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
	    __opEx.id = __current.id;
	    __opEx.facet = __current.facet;
	    __opEx.operation = __current.operation;
	    throw __opEx;
	}
	try
	{
	    return __servant->recvStringSeq(__current);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::NonRepeatable(__ex);
	}
    }
}

::Demo::StringSeq
IceDelegateD::Demo::Throughput::echoStringSeq(const ::Demo::StringSeq& seq, const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, "echoStringSeq", static_cast< ::Ice::OperationMode>(0), __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::Demo::Throughput* __servant = dynamic_cast< ::Demo::Throughput*>(__direct.servant().get());
	if(!__servant)
	{
	    ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
	    __opEx.id = __current.id;
	    __opEx.facet = __current.facet;
	    __opEx.operation = __current.operation;
	    throw __opEx;
	}
	try
	{
	    return __servant->echoStringSeq(seq, __current);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::NonRepeatable(__ex);
	}
    }
}

void
IceDelegateD::Demo::Throughput::sendStructSeq(const ::Demo::StringDoubleSeq& seq, const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, "sendStructSeq", static_cast< ::Ice::OperationMode>(0), __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::Demo::Throughput* __servant = dynamic_cast< ::Demo::Throughput*>(__direct.servant().get());
	if(!__servant)
	{
	    ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
	    __opEx.id = __current.id;
	    __opEx.facet = __current.facet;
	    __opEx.operation = __current.operation;
	    throw __opEx;
	}
	try
	{
	    __servant->sendStructSeq(seq, __current);
	    return;
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::NonRepeatable(__ex);
	}
    }
}

::Demo::StringDoubleSeq
IceDelegateD::Demo::Throughput::recvStructSeq(const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, "recvStructSeq", static_cast< ::Ice::OperationMode>(0), __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::Demo::Throughput* __servant = dynamic_cast< ::Demo::Throughput*>(__direct.servant().get());
	if(!__servant)
	{
	    ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
	    __opEx.id = __current.id;
	    __opEx.facet = __current.facet;
	    __opEx.operation = __current.operation;
	    throw __opEx;
	}
	try
	{
	    return __servant->recvStructSeq(__current);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::NonRepeatable(__ex);
	}
    }
}

::Demo::StringDoubleSeq
IceDelegateD::Demo::Throughput::echoStructSeq(const ::Demo::StringDoubleSeq& seq, const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, "echoStructSeq", static_cast< ::Ice::OperationMode>(0), __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::Demo::Throughput* __servant = dynamic_cast< ::Demo::Throughput*>(__direct.servant().get());
	if(!__servant)
	{
	    ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
	    __opEx.id = __current.id;
	    __opEx.facet = __current.facet;
	    __opEx.operation = __current.operation;
	    throw __opEx;
	}
	try
	{
	    return __servant->echoStructSeq(seq, __current);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::NonRepeatable(__ex);
	}
    }
}

void
IceDelegateD::Demo::Throughput::shutdown(const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, "shutdown", static_cast< ::Ice::OperationMode>(2), __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::Demo::Throughput* __servant = dynamic_cast< ::Demo::Throughput*>(__direct.servant().get());
	if(!__servant)
	{
	    ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
	    __opEx.id = __current.id;
	    __opEx.facet = __current.facet;
	    __opEx.operation = __current.operation;
	    throw __opEx;
	}
	try
	{
	    __servant->shutdown(__current);
	    return;
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::NonRepeatable(__ex);
	}
    }
}

static const ::std::string __Demo__Throughput_ids[2] =
{
    "::Demo::Throughput",
    "::Ice::Object"
};

bool
Demo::Throughput::ice_isA(const ::std::string& _s, const ::Ice::Current&) const
{
    return ::std::binary_search(__Demo__Throughput_ids, __Demo__Throughput_ids + 2, _s);
}

::std::vector< ::std::string>
Demo::Throughput::ice_ids(const ::Ice::Current&) const
{
    return ::std::vector< ::std::string>(&__Demo__Throughput_ids[0], &__Demo__Throughput_ids[2]);
}

const ::std::string&
Demo::Throughput::ice_id(const ::Ice::Current&) const
{
    return __Demo__Throughput_ids[0];
}

const ::std::string&
Demo::Throughput::ice_staticId()
{
    return __Demo__Throughput_ids[0];
}

::IceInternal::DispatchStatus
Demo::Throughput::___sendByteSeq(::IceInternal::Incoming& __in, const ::Ice::Current& __current)
{
    ::IceInternal::BasicStream* __is = __in.is();
    ::Demo::ByteSeq seq;
    __is->read(seq);
    sendByteSeq(seq, __current);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
Demo::Throughput::___recvByteSeq(::IceInternal::Incoming& __in, const ::Ice::Current& __current)
{
    ::IceInternal::BasicStream* __os = __in.os();
    ::Demo::ByteSeq __ret = recvByteSeq(__current);
    __os->write(__ret);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
Demo::Throughput::___echoByteSeq(::IceInternal::Incoming& __in, const ::Ice::Current& __current)
{
    ::IceInternal::BasicStream* __is = __in.is();
    ::IceInternal::BasicStream* __os = __in.os();
    ::Demo::ByteSeq seq;
    __is->read(seq);
    ::Demo::ByteSeq __ret = echoByteSeq(seq, __current);
    __os->write(__ret);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
Demo::Throughput::___sendStringSeq(::IceInternal::Incoming& __in, const ::Ice::Current& __current)
{
    ::IceInternal::BasicStream* __is = __in.is();
    ::Demo::StringSeq seq;
    __is->read(seq);
    sendStringSeq(seq, __current);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
Demo::Throughput::___recvStringSeq(::IceInternal::Incoming& __in, const ::Ice::Current& __current)
{
    ::IceInternal::BasicStream* __os = __in.os();
    ::Demo::StringSeq __ret = recvStringSeq(__current);
    __os->write(__ret);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
Demo::Throughput::___echoStringSeq(::IceInternal::Incoming& __in, const ::Ice::Current& __current)
{
    ::IceInternal::BasicStream* __is = __in.is();
    ::IceInternal::BasicStream* __os = __in.os();
    ::Demo::StringSeq seq;
    __is->read(seq);
    ::Demo::StringSeq __ret = echoStringSeq(seq, __current);
    __os->write(__ret);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
Demo::Throughput::___sendStructSeq(::IceInternal::Incoming& __in, const ::Ice::Current& __current)
{
    ::IceInternal::BasicStream* __is = __in.is();
    ::Demo::StringDoubleSeq seq;
    ::Demo::__read(__is, seq, ::Demo::__U__StringDoubleSeq());
    sendStructSeq(seq, __current);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
Demo::Throughput::___recvStructSeq(::IceInternal::Incoming& __in, const ::Ice::Current& __current)
{
    ::IceInternal::BasicStream* __os = __in.os();
    ::Demo::StringDoubleSeq __ret = recvStructSeq(__current);
    ::Demo::__write(__os, __ret, ::Demo::__U__StringDoubleSeq());
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
Demo::Throughput::___echoStructSeq(::IceInternal::Incoming& __in, const ::Ice::Current& __current)
{
    ::IceInternal::BasicStream* __is = __in.is();
    ::IceInternal::BasicStream* __os = __in.os();
    ::Demo::StringDoubleSeq seq;
    ::Demo::__read(__is, seq, ::Demo::__U__StringDoubleSeq());
    ::Demo::StringDoubleSeq __ret = echoStructSeq(seq, __current);
    ::Demo::__write(__os, __ret, ::Demo::__U__StringDoubleSeq());
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
Demo::Throughput::___shutdown(::IceInternal::Incoming& __in, const ::Ice::Current& __current)
{
    shutdown(__current);
    return ::IceInternal::DispatchOK;
}

static ::std::string __Demo__Throughput_all[] =
{
    "echoByteSeq",
    "echoStringSeq",
    "echoStructSeq",
    "ice_id",
    "ice_ids",
    "ice_isA",
    "ice_ping",
    "recvByteSeq",
    "recvStringSeq",
    "recvStructSeq",
    "sendByteSeq",
    "sendStringSeq",
    "sendStructSeq",
    "shutdown"
};

::IceInternal::DispatchStatus
Demo::Throughput::__dispatch(::IceInternal::Incoming& in, const ::Ice::Current& current)
{
    ::std::pair< ::std::string*, ::std::string*> r = ::std::equal_range(__Demo__Throughput_all, __Demo__Throughput_all + 14, current.operation);
    if(r.first == r.second)
    {
	return ::IceInternal::DispatchOperationNotExist;
    }

    switch(r.first - __Demo__Throughput_all)
    {
	case 0:
	{
	    return ___echoByteSeq(in, current);
	}
	case 1:
	{
	    return ___echoStringSeq(in, current);
	}
	case 2:
	{
	    return ___echoStructSeq(in, current);
	}
	case 3:
	{
	    return ___ice_id(in, current);
	}
	case 4:
	{
	    return ___ice_ids(in, current);
	}
	case 5:
	{
	    return ___ice_isA(in, current);
	}
	case 6:
	{
	    return ___ice_ping(in, current);
	}
	case 7:
	{
	    return ___recvByteSeq(in, current);
	}
	case 8:
	{
	    return ___recvStringSeq(in, current);
	}
	case 9:
	{
	    return ___recvStructSeq(in, current);
	}
	case 10:
	{
	    return ___sendByteSeq(in, current);
	}
	case 11:
	{
	    return ___sendStringSeq(in, current);
	}
	case 12:
	{
	    return ___sendStructSeq(in, current);
	}
	case 13:
	{
	    return ___shutdown(in, current);
	}
    }

    assert(false);
    return ::IceInternal::DispatchOperationNotExist;
}

void
Demo::Throughput::__write(::IceInternal::BasicStream* __os) const
{
    __os->writeTypeId(ice_staticId());
    __os->startWriteSlice();
    __os->endWriteSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    Object::__write(__os);
#else
    ::Ice::Object::__write(__os);
#endif
}

void
Demo::Throughput::__read(::IceInternal::BasicStream* __is, bool __rid)
{
    if(__rid)
    {
	::std::string myId;
	__is->readTypeId(myId);
    }
    __is->startReadSlice();
    __is->endReadSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    Object::__read(__is, true);
#else
    ::Ice::Object::__read(__is, true);
#endif
}

void
Demo::Throughput::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type Demo::Throughput was not generated with stream support";
    throw ex;
}

void
Demo::Throughput::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type Demo::Throughput was not generated with stream support";
    throw ex;
}

void 
Demo::__patch__ThroughputPtr(void* __addr, ::Ice::ObjectPtr& v)
{
    ::Demo::ThroughputPtr* p = static_cast< ::Demo::ThroughputPtr*>(__addr);
    assert(p);
    *p = ::Demo::ThroughputPtr::dynamicCast(v);
    if(v && !*p)
    {
	::Ice::NoObjectFactoryException e(__FILE__, __LINE__);
	e.type = ::Demo::Throughput::ice_staticId();
	throw e;
    }
}

bool
Demo::operator==(const ::Demo::Throughput& l, const ::Demo::Throughput& r)
{
    return static_cast<const ::Ice::Object&>(l) == static_cast<const ::Ice::Object&>(r);
}

bool
Demo::operator!=(const ::Demo::Throughput& l, const ::Demo::Throughput& r)
{
    return static_cast<const ::Ice::Object&>(l) != static_cast<const ::Ice::Object&>(r);
}

bool
Demo::operator<(const ::Demo::Throughput& l, const ::Demo::Throughput& r)
{
    return static_cast<const ::Ice::Object&>(l) < static_cast<const ::Ice::Object&>(r);
}
