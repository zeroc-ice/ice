// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

// Generated from file `Admin.ice'
// Ice version 0.0.1

#include <IcePack/Admin.h>

#ifndef ICE_IGNORE_VERSION
#   if ICE_INT_VERSION != 0x1
#       error Ice version mismatch!
#   endif
#endif

void IceInternal::incRef(::IcePack::ServerDescription* p) { p->__incRef(); }
void IceInternal::decRef(::IcePack::ServerDescription* p) { p->__decRef(); }

void IceInternal::incRef(::IceProxy::IcePack::ServerDescription* p) { p->__incRef(); }
void IceInternal::decRef(::IceProxy::IcePack::ServerDescription* p) { p->__decRef(); }

void IceInternal::checkedCast(::IceProxy::Ice::Object* b, ::IceProxy::IcePack::ServerDescription*& d)
{
    d = dynamic_cast< ::IceProxy::IcePack::ServerDescription*>(b);
    if (!d && b->_isA("::IcePack::ServerDescription"))
    {
	d = new ::IceProxy::IcePack::ServerDescription;
	b->__copyTo(d);
    }
}

void IceInternal::uncheckedCast(::IceProxy::Ice::Object* b, ::IceProxy::IcePack::ServerDescription*& d)
{
    d = dynamic_cast< ::IceProxy::IcePack::ServerDescription*>(b);
    if (!d)
    {
	d = new ::IceProxy::IcePack::ServerDescription;
	b->__copyTo(d);
    }
}

void IceInternal::incRef(::IcePack::Admin* p) { p->__incRef(); }
void IceInternal::decRef(::IcePack::Admin* p) { p->__decRef(); }

void IceInternal::incRef(::IceProxy::IcePack::Admin* p) { p->__incRef(); }
void IceInternal::decRef(::IceProxy::IcePack::Admin* p) { p->__decRef(); }

void IceInternal::checkedCast(::IceProxy::Ice::Object* b, ::IceProxy::IcePack::Admin*& d)
{
    d = dynamic_cast< ::IceProxy::IcePack::Admin*>(b);
    if (!d && b->_isA("::IcePack::Admin"))
    {
	d = new ::IceProxy::IcePack::Admin;
	b->__copyTo(d);
    }
}

void IceInternal::uncheckedCast(::IceProxy::Ice::Object* b, ::IceProxy::IcePack::Admin*& d)
{
    d = dynamic_cast< ::IceProxy::IcePack::Admin*>(b);
    if (!d)
    {
	d = new ::IceProxy::IcePack::Admin;
	b->__copyTo(d);
    }
}

void
IcePack::__write(::IceInternal::Stream* __os, const ::IcePack::ServerDescriptionPrx& v)
{
    __os->write(::Ice::ObjectPrx(v));
}

void
IcePack::__read(::IceInternal::Stream* __is, ::IcePack::ServerDescriptionPrx& v)
{
    ::Ice::ObjectPrx proxy;
    __is->read(proxy);
    if (!proxy)
    {
	v = 0;
    }
    else
    {
	v = new ::IceProxy::IcePack::ServerDescription;
	proxy->__copyTo(v.get());
    }
}

void
IcePack::__write(::IceInternal::Stream* __os, const ::IcePack::AdminPrx& v)
{
    __os->write(::Ice::ObjectPrx(v));
}

void
IcePack::__read(::IceInternal::Stream* __is, ::IcePack::AdminPrx& v)
{
    ::Ice::ObjectPrx proxy;
    __is->read(proxy);
    if (!proxy)
    {
	v = 0;
    }
    else
    {
	v = new ::IceProxy::IcePack::Admin;
	proxy->__copyTo(v.get());
    }
}

void
IceProxy::IcePack::ServerDescription::_throw()
{
    throw ::IcePack::ServerDescriptionPrxE(this);
}

::IceInternal::Handle< ::IceDelegateM::Ice::Object>
IceProxy::IcePack::ServerDescription::__createDelegateM()
{
    return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::IcePack::ServerDescription);
}

void
IceProxy::IcePack::Admin::_throw()
{
    throw ::IcePack::AdminPrxE(this);
}

void
IceProxy::IcePack::Admin::add(const ::IcePack::ServerDescriptionPtr& description)
{
    int __cnt = 0;
    while (true)
    {
	::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	::IceDelegate::IcePack::Admin* __del = dynamic_cast< ::IceDelegate::IcePack::Admin*>(__delBase.get());
	try
	{
	    __del->add(description);
	    return;
	}
	catch (const ::Ice::LocationForward& __ex)
	{
	    __locationForward(__ex);
	}
	catch (const ::IceInternal::NonRepeatable& __ex)
	{
	    __ex.raise();
	}
	catch (const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

void
IceProxy::IcePack::Admin::remove(const ::Ice::ObjectPrx& object)
{
    int __cnt = 0;
    while (true)
    {
	::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	::IceDelegate::IcePack::Admin* __del = dynamic_cast< ::IceDelegate::IcePack::Admin*>(__delBase.get());
	try
	{
	    __del->remove(object);
	    return;
	}
	catch (const ::Ice::LocationForward& __ex)
	{
	    __locationForward(__ex);
	}
	catch (const ::IceInternal::NonRepeatable& __ex)
	{
	    __ex.raise();
	}
	catch (const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

::IcePack::ServerDescriptionPtr
IceProxy::IcePack::Admin::find(const ::Ice::ObjectPrx& object)
{
    int __cnt = 0;
    while (true)
    {
	::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	::IceDelegate::IcePack::Admin* __del = dynamic_cast< ::IceDelegate::IcePack::Admin*>(__delBase.get());
	try
	{
	    return __del->find(object);
	}
	catch (const ::Ice::LocationForward& __ex)
	{
	    __locationForward(__ex);
	}
	catch (const ::IceInternal::NonRepeatable& __ex)
	{
	    __ex.raise();
	}
	catch (const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

void
IceProxy::IcePack::Admin::shutdown()
{
    int __cnt = 0;
    while (true)
    {
	::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	::IceDelegate::IcePack::Admin* __del = dynamic_cast< ::IceDelegate::IcePack::Admin*>(__delBase.get());
	try
	{
	    __del->shutdown();
	    return;
	}
	catch (const ::Ice::LocationForward& __ex)
	{
	    __locationForward(__ex);
	}
	catch (const ::IceInternal::NonRepeatable& __ex)
	{
	    __ex.raise();
	}
	catch (const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

::IceInternal::Handle< ::IceDelegateM::Ice::Object>
IceProxy::IcePack::Admin::__createDelegateM()
{
    return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::IcePack::Admin);
}

void
IceDelegateM::IcePack::Admin::add(const ::IcePack::ServerDescriptionPtr& description)
{
    ::IceInternal::Outgoing __out(__emitter(), __reference());
    ::IceInternal::Stream* __os = __out.os();
    __os->write("add");
    {
	::Ice::ObjectPtr __obj = description;
	__os->write(__obj);
    }
    if (!__out.invoke())
    {
	throw ::Ice::UnknownUserException(__FILE__, __LINE__);
    }
}

void
IceDelegateM::IcePack::Admin::remove(const ::Ice::ObjectPrx& object)
{
    ::IceInternal::Outgoing __out(__emitter(), __reference());
    ::IceInternal::Stream* __os = __out.os();
    __os->write("remove");
    __os->write(object);
    if (!__out.invoke())
    {
	throw ::Ice::UnknownUserException(__FILE__, __LINE__);
    }
}

::IcePack::ServerDescriptionPtr
IceDelegateM::IcePack::Admin::find(const ::Ice::ObjectPrx& object)
{
    ::IceInternal::Outgoing __out(__emitter(), __reference());
    ::IceInternal::Stream* __is = __out.is();
    ::IceInternal::Stream* __os = __out.os();
    __os->write("find");
    __os->write(object);
    if (!__out.invoke())
    {
	throw ::Ice::UnknownUserException(__FILE__, __LINE__);
    }
    ::IcePack::ServerDescriptionPtr __ret;
    {
	::Ice::ObjectPtr __obj;
	__is->read(__obj, ::IcePack::ServerDescription::__classIds[0]);
	if (!__obj)
	{
	    __obj = new ::IcePack::ServerDescription;
	    __obj->__read(__is);
	}
	__ret = ::IcePack::ServerDescriptionPtr::dynamicCast(__obj);
	if (!__ret)
	{
	    throw ::Ice::ValueUnmarshalException(__FILE__, __LINE__);
	}
    }
    return __ret;
}

void
IceDelegateM::IcePack::Admin::shutdown()
{
    ::IceInternal::Outgoing __out(__emitter(), __reference());
    ::IceInternal::Stream* __os = __out.os();
    __os->write("shutdown");
    if (!__out.invoke())
    {
	throw ::Ice::UnknownUserException(__FILE__, __LINE__);
    }
}

IcePack::ServerDescriptionPtrE::ServerDescriptionPtrE(const ServerDescriptionPtrE& p)
{
    _ptr = p._ptr;
}

IcePack::ServerDescriptionPtrE::ServerDescriptionPtrE(const ::IcePack::ServerDescriptionPtr& p)
{
    _ptr = p;
}

IcePack::ServerDescriptionPtrE::operator ::IcePack::ServerDescriptionPtr() const
{
    return ::IcePack::ServerDescriptionPtr(dynamic_cast< ::IcePack::ServerDescription*>(_ptr.get()));
}

::IcePack::ServerDescription*
IcePack::ServerDescriptionPtrE::operator->() const
{
    return dynamic_cast< ::IcePack::ServerDescription*>(_ptr.get());
}

IcePack::ServerDescriptionPrxE::ServerDescriptionPrxE(const ServerDescriptionPrxE& p)
{
    _prx = p._prx;
}

IcePack::ServerDescriptionPrxE::ServerDescriptionPrxE(const ::IcePack::ServerDescriptionPrx& p)
{
    _prx = p;
}

IcePack::ServerDescriptionPrxE::operator ::IcePack::ServerDescriptionPrx() const
{
    return ::IcePack::ServerDescriptionPrx(dynamic_cast< ::IceProxy::IcePack::ServerDescription*>(_prx.get()));
}

::IceProxy::IcePack::ServerDescription*
IcePack::ServerDescriptionPrxE::operator->() const
{
    return dynamic_cast< ::IceProxy::IcePack::ServerDescription*>(_prx.get());
}

void
IcePack::ServerDescription::_throw()
{
    throw ::IcePack::ServerDescriptionPtrE(this);
}

std::string IcePack::ServerDescription::__ids[2] =
{
    "::Ice::Object",
    "::IcePack::ServerDescription"
};

std::string IcePack::ServerDescription::__classIds[2] =
{
    "::IcePack::ServerDescription",
    "::Ice::Object"
};

bool
IcePack::ServerDescription::_isA(const std::string& s)
{
    std::string* b = __ids;
    std::string* e = __ids + 2;
    std::pair<std::string*, std::string*> r = std::equal_range(b, e, s);
    return r.first != r.second;
}

const std::string*
IcePack::ServerDescription::_classIds()
{
    return __classIds;
}

void
IcePack::ServerDescription::__write(::IceInternal::Stream* __os)
{
    __os->startWriteEncaps();
    __os->write(object);
    __os->write(regex);
    __os->write(host);
    __os->write(path);
    __os->endWriteEncaps();
}

void
IcePack::ServerDescription::__read(::IceInternal::Stream* __is)
{
    __is->startReadEncaps();
    __is->read(object);
    __is->read(regex);
    __is->read(host);
    __is->read(path);
    __is->endReadEncaps();
}

IcePack::AdminPtrE::AdminPtrE(const AdminPtrE& p)
{
    _ptr = p._ptr;
}

IcePack::AdminPtrE::AdminPtrE(const ::IcePack::AdminPtr& p)
{
    _ptr = p;
}

IcePack::AdminPtrE::operator ::IcePack::AdminPtr() const
{
    return ::IcePack::AdminPtr(dynamic_cast< ::IcePack::Admin*>(_ptr.get()));
}

::IcePack::Admin*
IcePack::AdminPtrE::operator->() const
{
    return dynamic_cast< ::IcePack::Admin*>(_ptr.get());
}

IcePack::AdminPrxE::AdminPrxE(const AdminPrxE& p)
{
    _prx = p._prx;
}

IcePack::AdminPrxE::AdminPrxE(const ::IcePack::AdminPrx& p)
{
    _prx = p;
}

IcePack::AdminPrxE::operator ::IcePack::AdminPrx() const
{
    return ::IcePack::AdminPrx(dynamic_cast< ::IceProxy::IcePack::Admin*>(_prx.get()));
}

::IceProxy::IcePack::Admin*
IcePack::AdminPrxE::operator->() const
{
    return dynamic_cast< ::IceProxy::IcePack::Admin*>(_prx.get());
}

void
IcePack::Admin::_throw()
{
    throw ::IcePack::AdminPtrE(this);
}

std::string IcePack::Admin::__ids[2] =
{
    "::Ice::Object",
    "::IcePack::Admin"
};

std::string IcePack::Admin::__classIds[2] =
{
    "::IcePack::Admin",
    "::Ice::Object"
};

bool
IcePack::Admin::_isA(const std::string& s)
{
    std::string* b = __ids;
    std::string* e = __ids + 2;
    std::pair<std::string*, std::string*> r = std::equal_range(b, e, s);
    return r.first != r.second;
}

const std::string*
IcePack::Admin::_classIds()
{
    return __classIds;
}

::IceInternal::DispatchStatus
IcePack::Admin::___add(::IceInternal::Incoming& __in)
{
    ::IceInternal::Stream* __is = __in.is();
    ::IcePack::ServerDescriptionPtr description;
    {
	::Ice::ObjectPtr __obj;
	__is->read(__obj, ::IcePack::ServerDescription::__classIds[0]);
	if (!__obj)
	{
	    __obj = new ::IcePack::ServerDescription;
	    __obj->__read(__is);
	}
	description = ::IcePack::ServerDescriptionPtr::dynamicCast(__obj);
	if (!description)
	{
	    throw ::Ice::ValueUnmarshalException(__FILE__, __LINE__);
	}
    }
    add(description);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IcePack::Admin::___remove(::IceInternal::Incoming& __in)
{
    ::IceInternal::Stream* __is = __in.is();
    ::Ice::ObjectPrx object;
    __is->read(object);
    remove(object);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IcePack::Admin::___find(::IceInternal::Incoming& __in)
{
    ::IceInternal::Stream* __is = __in.is();
    ::IceInternal::Stream* __os = __in.os();
    ::Ice::ObjectPrx object;
    __is->read(object);
    ::IcePack::ServerDescriptionPtr __ret = find(object);
    {
	::Ice::ObjectPtr __obj = __ret;
	__os->write(__obj);
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IcePack::Admin::___shutdown(::IceInternal::Incoming& __in)
{
    shutdown();
    return ::IceInternal::DispatchOK;
}

std::string IcePack::Admin::__names[6] =
{
    "_isA",
    "_ping",
    "add",
    "find",
    "remove",
    "shutdown"
};

::IceInternal::DispatchStatus
IcePack::Admin::__dispatch(::IceInternal::Incoming& in, const std::string& s)
{
    std::string* b = __names;
    std::string* e = __names + 6;
    std::pair<std::string*, std::string*> r = std::equal_range(b, e, s);
    if (r.first == r.second)
    {
	return ::IceInternal::DispatchOperationNotExist;
    }

    switch (r.first - __names)
    {
	case 0:
	{
	    return ____isA(in);
	}
	case 1:
	{
	    return ____ping(in);
	}
	case 2:
	{
	    return ___add(in);
	}
	case 3:
	{
	    return ___find(in);
	}
	case 4:
	{
	    return ___remove(in);
	}
	case 5:
	{
	    return ___shutdown(in);
	}
    }

    assert(false);
    return ::IceInternal::DispatchOperationNotExist;
}

void
IcePack::Admin::__write(::IceInternal::Stream* __os)
{
    __os->startWriteEncaps();
    __os->endWriteEncaps();
}

void
IcePack::Admin::__read(::IceInternal::Stream* __is)
{
    __is->startReadEncaps();
    __is->endReadEncaps();
}
