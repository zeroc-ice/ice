// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Ice version 2.1.0
// Generated from file `Latency.ice'

#include <Latency.h>
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
IceInternal::incRef(::Demo::Ping* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::Demo::Ping* p)
{
    p->__decRef();
}

void
IceInternal::incRef(::IceProxy::Demo::Ping* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::IceProxy::Demo::Ping* p)
{
    p->__decRef();
}

void
Demo::__write(::IceInternal::BasicStream* __os, const ::Demo::PingPrx& v)
{
    __os->write(::Ice::ObjectPrx(v));
}

void
Demo::__read(::IceInternal::BasicStream* __is, ::Demo::PingPrx& v)
{
    ::Ice::ObjectPrx proxy;
    __is->read(proxy);
    if(!proxy)
    {
	v = 0;
    }
    else
    {
	v = new ::IceProxy::Demo::Ping;
	v->__copyFrom(proxy);
    }
}

void
Demo::__write(::IceInternal::BasicStream* __os, const ::Demo::PingPtr& v)
{
    __os->write(::Ice::ObjectPtr(v));
}

void
IceProxy::Demo::Ping::shutdown()
{
    shutdown(__defaultContext());
}

void
IceProxy::Demo::Ping::shutdown(const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::Demo::Ping* __del = dynamic_cast< ::IceDelegate::Demo::Ping*>(__delBase.get());
	    __del->shutdown(__ctx);
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

const ::std::string&
IceProxy::Demo::Ping::ice_staticId()
{
    return ::Demo::Ping::ice_staticId();
}

::IceInternal::Handle< ::IceDelegateM::Ice::Object>
IceProxy::Demo::Ping::__createDelegateM()
{
    return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::Demo::Ping);
}

::IceInternal::Handle< ::IceDelegateD::Ice::Object>
IceProxy::Demo::Ping::__createDelegateD()
{
    return ::IceInternal::Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD::Demo::Ping);
}

bool
IceProxy::Demo::operator==(const ::IceProxy::Demo::Ping& l, const ::IceProxy::Demo::Ping& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) == static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::Demo::operator!=(const ::IceProxy::Demo::Ping& l, const ::IceProxy::Demo::Ping& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) != static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::Demo::operator<(const ::IceProxy::Demo::Ping& l, const ::IceProxy::Demo::Ping& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) < static_cast<const ::IceProxy::Ice::Object&>(r);
}

void
IceDelegateM::Demo::Ping::shutdown(const ::Ice::Context& __context)
{
    static const ::std::string __operation("shutdown");
    ::IceInternal::Outgoing __out(__connection.get(), __reference.get(), __operation, static_cast< ::Ice::OperationMode>(0), __context, __compress);
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
IceDelegateD::Demo::Ping::shutdown(const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, "shutdown", static_cast< ::Ice::OperationMode>(0), __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::Demo::Ping* __servant = dynamic_cast< ::Demo::Ping*>(__direct.servant().get());
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

static const ::std::string __Demo__Ping_ids[2] =
{
    "::Demo::Ping",
    "::Ice::Object"
};

bool
Demo::Ping::ice_isA(const ::std::string& _s, const ::Ice::Current&) const
{
    return ::std::binary_search(__Demo__Ping_ids, __Demo__Ping_ids + 2, _s);
}

::std::vector< ::std::string>
Demo::Ping::ice_ids(const ::Ice::Current&) const
{
    return ::std::vector< ::std::string>(&__Demo__Ping_ids[0], &__Demo__Ping_ids[2]);
}

const ::std::string&
Demo::Ping::ice_id(const ::Ice::Current&) const
{
    return __Demo__Ping_ids[0];
}

const ::std::string&
Demo::Ping::ice_staticId()
{
    return __Demo__Ping_ids[0];
}

::IceInternal::DispatchStatus
Demo::Ping::___shutdown(::IceInternal::Incoming& __in, const ::Ice::Current& __current)
{
    shutdown(__current);
    return ::IceInternal::DispatchOK;
}

static ::std::string __Demo__Ping_all[] =
{
    "ice_id",
    "ice_ids",
    "ice_isA",
    "ice_ping",
    "shutdown"
};

::IceInternal::DispatchStatus
Demo::Ping::__dispatch(::IceInternal::Incoming& in, const ::Ice::Current& current)
{
    ::std::pair< ::std::string*, ::std::string*> r = ::std::equal_range(__Demo__Ping_all, __Demo__Ping_all + 5, current.operation);
    if(r.first == r.second)
    {
	return ::IceInternal::DispatchOperationNotExist;
    }

    switch(r.first - __Demo__Ping_all)
    {
	case 0:
	{
	    return ___ice_id(in, current);
	}
	case 1:
	{
	    return ___ice_ids(in, current);
	}
	case 2:
	{
	    return ___ice_isA(in, current);
	}
	case 3:
	{
	    return ___ice_ping(in, current);
	}
	case 4:
	{
	    return ___shutdown(in, current);
	}
    }

    assert(false);
    return ::IceInternal::DispatchOperationNotExist;
}

void
Demo::Ping::__write(::IceInternal::BasicStream* __os) const
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
Demo::Ping::__read(::IceInternal::BasicStream* __is, bool __rid)
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
Demo::Ping::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type Demo::Ping was not generated with stream support";
    throw ex;
}

void
Demo::Ping::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type Demo::Ping was not generated with stream support";
    throw ex;
}

void 
Demo::__patch__PingPtr(void* __addr, ::Ice::ObjectPtr& v)
{
    ::Demo::PingPtr* p = static_cast< ::Demo::PingPtr*>(__addr);
    assert(p);
    *p = ::Demo::PingPtr::dynamicCast(v);
    if(v && !*p)
    {
	::Ice::NoObjectFactoryException e(__FILE__, __LINE__);
	e.type = ::Demo::Ping::ice_staticId();
	throw e;
    }
}

bool
Demo::operator==(const ::Demo::Ping& l, const ::Demo::Ping& r)
{
    return static_cast<const ::Ice::Object&>(l) == static_cast<const ::Ice::Object&>(r);
}

bool
Demo::operator!=(const ::Demo::Ping& l, const ::Demo::Ping& r)
{
    return static_cast<const ::Ice::Object&>(l) != static_cast<const ::Ice::Object&>(r);
}

bool
Demo::operator<(const ::Demo::Ping& l, const ::Demo::Ping& r)
{
    return static_cast<const ::Ice::Object&>(l) < static_cast<const ::Ice::Object&>(r);
}
