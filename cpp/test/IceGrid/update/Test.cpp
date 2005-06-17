// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Ice version 2.1.0
// Generated from file `Test.ice'

#include <Test.h>
#include <Ice/LocalException.h>
#include <Ice/ObjectFactory.h>
#include <Ice/BasicStream.h>
#include <Ice/Object.h>

#ifndef ICE_IGNORE_VERSION
#   if ICE_INT_VERSION / 100 != 201
#       error Ice version mismatch!
#   endif
#   if ICE_INT_VERSION % 100 < 0
#       error Ice patch level mismatch!
#   endif
#endif

void
IceInternal::incRef(::Test::TestIntf* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::Test::TestIntf* p)
{
    p->__decRef();
}

void
IceInternal::incRef(::IceProxy::Test::TestIntf* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::IceProxy::Test::TestIntf* p)
{
    p->__decRef();
}

void
Test::__write(::IceInternal::BasicStream* __os, const ::Test::TestIntfPrx& v)
{
    __os->write(::Ice::ObjectPrx(v));
}

void
Test::__read(::IceInternal::BasicStream* __is, ::Test::TestIntfPrx& v)
{
    ::Ice::ObjectPrx proxy;
    __is->read(proxy);
    if(!proxy)
    {
	v = 0;
    }
    else
    {
	v = new ::IceProxy::Test::TestIntf;
	v->__copyFrom(proxy);
    }
}

void
Test::__write(::IceInternal::BasicStream* __os, const ::Test::TestIntfPtr& v)
{
    __os->write(::Ice::ObjectPtr(v));
}

void
IceProxy::Test::TestIntf::shutdown()
{
    shutdown(__defaultContext());
}

void
IceProxy::Test::TestIntf::shutdown(const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::Test::TestIntf* __del = dynamic_cast< ::IceDelegate::Test::TestIntf*>(__delBase.get());
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

::std::string
IceProxy::Test::TestIntf::getProperty(const ::std::string& name)
{
    return getProperty(name, __defaultContext());
}

::std::string
IceProxy::Test::TestIntf::getProperty(const ::std::string& name, const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    __checkTwowayOnly("getProperty");
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::Test::TestIntf* __del = dynamic_cast< ::IceDelegate::Test::TestIntf*>(__delBase.get());
	    return __del->getProperty(name, __ctx);
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
IceProxy::Test::TestIntf::ice_staticId()
{
    return ::Test::TestIntf::ice_staticId();
}

::IceInternal::Handle< ::IceDelegateM::Ice::Object>
IceProxy::Test::TestIntf::__createDelegateM()
{
    return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::Test::TestIntf);
}

::IceInternal::Handle< ::IceDelegateD::Ice::Object>
IceProxy::Test::TestIntf::__createDelegateD()
{
    return ::IceInternal::Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD::Test::TestIntf);
}

bool
IceProxy::Test::operator==(const ::IceProxy::Test::TestIntf& l, const ::IceProxy::Test::TestIntf& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) == static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::Test::operator!=(const ::IceProxy::Test::TestIntf& l, const ::IceProxy::Test::TestIntf& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) != static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::Test::operator<(const ::IceProxy::Test::TestIntf& l, const ::IceProxy::Test::TestIntf& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) < static_cast<const ::IceProxy::Ice::Object&>(r);
}

static const ::std::string __Test__TestIntf__shutdown_name = "shutdown";

void
IceDelegateM::Test::TestIntf::shutdown(const ::Ice::Context& __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __Test__TestIntf__shutdown_name, static_cast< ::Ice::OperationMode>(0), __context, __compress);
    bool __ok = __og.invoke();
    try
    {
	::IceInternal::BasicStream* __is = __og.is();
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

static const ::std::string __Test__TestIntf__getProperty_name = "getProperty";

::std::string
IceDelegateM::Test::TestIntf::getProperty(const ::std::string& name, const ::Ice::Context& __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __Test__TestIntf__getProperty_name, static_cast< ::Ice::OperationMode>(0), __context, __compress);
    try
    {
	::IceInternal::BasicStream* __os = __og.os();
	__os->write(name);
    }
    catch(const ::Ice::LocalException& __ex)
    {
	__og.abort(__ex);
    }
    bool __ok = __og.invoke();
    try
    {
	::IceInternal::BasicStream* __is = __og.is();
	if(!__ok)
	{
	    __is->throwException();
	}
	::std::string __ret;
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
IceDelegateD::Test::TestIntf::shutdown(const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, "shutdown", static_cast< ::Ice::OperationMode>(0), __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::Test::TestIntf* __servant = dynamic_cast< ::Test::TestIntf*>(__direct.servant().get());
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

::std::string
IceDelegateD::Test::TestIntf::getProperty(const ::std::string& name, const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, "getProperty", static_cast< ::Ice::OperationMode>(0), __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::Test::TestIntf* __servant = dynamic_cast< ::Test::TestIntf*>(__direct.servant().get());
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
	    return __servant->getProperty(name, __current);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::NonRepeatable(__ex);
	}
    }
}

static const ::std::string __Test__TestIntf_ids[2] =
{
    "::Ice::Object",
    "::Test::TestIntf"
};

bool
Test::TestIntf::ice_isA(const ::std::string& _s, const ::Ice::Current&) const
{
    return ::std::binary_search(__Test__TestIntf_ids, __Test__TestIntf_ids + 2, _s);
}

::std::vector< ::std::string>
Test::TestIntf::ice_ids(const ::Ice::Current&) const
{
    return ::std::vector< ::std::string>(&__Test__TestIntf_ids[0], &__Test__TestIntf_ids[2]);
}

const ::std::string&
Test::TestIntf::ice_id(const ::Ice::Current&) const
{
    return __Test__TestIntf_ids[1];
}

const ::std::string&
Test::TestIntf::ice_staticId()
{
    return __Test__TestIntf_ids[1];
}

::IceInternal::DispatchStatus
Test::TestIntf::___shutdown(::IceInternal::Incoming& __inS, const ::Ice::Current& __current)
{
    shutdown(__current);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
Test::TestIntf::___getProperty(::IceInternal::Incoming& __inS, const ::Ice::Current& __current)
{
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string name;
    __is->read(name);
    ::std::string __ret = getProperty(name, __current);
    __os->write(__ret);
    return ::IceInternal::DispatchOK;
}

static ::std::string __Test__TestIntf_all[] =
{
    "getProperty",
    "ice_id",
    "ice_ids",
    "ice_isA",
    "ice_ping",
    "shutdown"
};

::IceInternal::DispatchStatus
Test::TestIntf::__dispatch(::IceInternal::Incoming& in, const ::Ice::Current& current)
{
    ::std::pair< ::std::string*, ::std::string*> r = ::std::equal_range(__Test__TestIntf_all, __Test__TestIntf_all + 6, current.operation);
    if(r.first == r.second)
    {
	return ::IceInternal::DispatchOperationNotExist;
    }

    switch(r.first - __Test__TestIntf_all)
    {
	case 0:
	{
	    return ___getProperty(in, current);
	}
	case 1:
	{
	    return ___ice_id(in, current);
	}
	case 2:
	{
	    return ___ice_ids(in, current);
	}
	case 3:
	{
	    return ___ice_isA(in, current);
	}
	case 4:
	{
	    return ___ice_ping(in, current);
	}
	case 5:
	{
	    return ___shutdown(in, current);
	}
    }

    assert(false);
    return ::IceInternal::DispatchOperationNotExist;
}

void
Test::TestIntf::__write(::IceInternal::BasicStream* __os) const
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
Test::TestIntf::__read(::IceInternal::BasicStream* __is, bool __rid)
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
Test::TestIntf::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type Test::TestIntf was not generated with stream support";
    throw ex;
}

void
Test::TestIntf::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type Test::TestIntf was not generated with stream support";
    throw ex;
}

void 
Test::__patch__TestIntfPtr(void* __addr, ::Ice::ObjectPtr& v)
{
    ::Test::TestIntfPtr* p = static_cast< ::Test::TestIntfPtr*>(__addr);
    assert(p);
    *p = ::Test::TestIntfPtr::dynamicCast(v);
    if(v && !*p)
    {
	::Ice::NoObjectFactoryException e(__FILE__, __LINE__);
	e.type = ::Test::TestIntf::ice_staticId();
	throw e;
    }
}

bool
Test::operator==(const ::Test::TestIntf& l, const ::Test::TestIntf& r)
{
    return static_cast<const ::Ice::Object&>(l) == static_cast<const ::Ice::Object&>(r);
}

bool
Test::operator!=(const ::Test::TestIntf& l, const ::Test::TestIntf& r)
{
    return static_cast<const ::Ice::Object&>(l) != static_cast<const ::Ice::Object&>(r);
}

bool
Test::operator<(const ::Test::TestIntf& l, const ::Test::TestIntf& r)
{
    return static_cast<const ::Ice::Object&>(l) < static_cast<const ::Ice::Object&>(r);
}
