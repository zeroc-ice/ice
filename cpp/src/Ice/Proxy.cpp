// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Proxy.h>
#include <Ice/Object.h>
#include <Ice/ObjectAdapterFactory.h>
#include <Ice/Outgoing.h>
#include <Ice/Reference.h>
#include <Ice/Endpoint.h>
#include <Ice/Instance.h>
#include <Ice/Logger.h>
#include <Ice/TraceLevels.h>
#include <Ice/Emitter.h>
#include <Ice/Stream.h>
#include <Ice/LocalException.h>
#include <Ice/Functional.h>
#include <sstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(::IceProxy::Ice::Object* p) { p->__incRef(); }
void IceInternal::decRef(::IceProxy::Ice::Object* p) { p->__decRef(); }

void IceInternal::incRef(::IceDelegate::Ice::Object* p) { p->__incRef(); }
void IceInternal::decRef(::IceDelegate::Ice::Object* p) { p->__decRef(); }

void IceInternal::incRef(::IceDelegateM::Ice::Object* p) { p->__incRef(); }
void IceInternal::decRef(::IceDelegateM::Ice::Object* p) { p->__decRef(); }

void
IceInternal::checkedCast(::IceProxy::Ice::Object* b, ::IceProxy::Ice::Object*& d)
{
    d = b;
}

void
IceInternal::uncheckedCast(::IceProxy::Ice::Object* b, ::IceProxy::Ice::Object*& d)
{
    d = b;
}

Ice::ObjectPrxE::ObjectPrxE(const ObjectPrxE& p) :
    _prx(p._prx)
{
}

Ice::ObjectPrxE::ObjectPrxE(const ObjectPrx& p) :
    _prx(p)
{
}

Ice::ObjectPrxE::operator ObjectPrx() const
{
    return _prx;
}

IceProxy::Ice::Object*
Ice::ObjectPrxE::operator->() const
{
    return _prx.get();
}

Ice::ObjectPrxE::operator bool() const
{
    return _prx.get() ? true : false;
}

void
IceProxy::Ice::Object::_throw()
{
    throw ObjectPrxE(this);
}

bool
IceProxy::Ice::Object::_isA(const string& s)
{
    int __cnt = 0;
    while (true)
    {
	try
	{
	    Handle< ::IceDelegate::Ice::Object> __del = __getDelegate();
	    return __del->_isA(s);
	}
	catch (const LocationForward& __ex)
	{
	    __locationForward(__ex);
	}
	catch (const NonRepeatable& __ex)
	{
	    __handleException(*__ex.get(), __cnt);
	}
	catch (const LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

void
IceProxy::Ice::Object::_ping()
{
    int __cnt = 0;
    while (true)
    {
	try
	{
	    Handle< ::IceDelegate::Ice::Object> __del = __getDelegate();
	    __del->_ping();
	    return;
	}
	catch (const LocationForward& __ex)
	{
	    __locationForward(__ex);
	}
	catch (const NonRepeatable& __ex)
	{
	    __handleException(*__ex.get(), __cnt);
	}
	catch (const LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

void
IceProxy::Ice::Object::_flush()
{
    Handle< ::IceDelegate::Ice::Object> __del = __getDelegate();
    __del->_flush();
}

bool
IceProxy::Ice::Object::operator==(const Object& r) const
{
    return _reference->identity == r._reference->identity;
}

bool
IceProxy::Ice::Object::operator!=(const Object& r) const
{
    return _reference->identity != r._reference->identity;
}

bool
IceProxy::Ice::Object::operator<(const Object& r) const
{
    return _reference->identity < r._reference->identity;
}

std::string
IceProxy::Ice::Object::_getIdentity() const
{
    return _reference->identity;
}

::Ice::ObjectPrx
IceProxy::Ice::Object::_newIdentity(const std::string& newIdentity) const
{
    if (newIdentity == _reference->identity)
    {
	return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
	ObjectPrx proxy(new ::IceProxy::Ice::Object());
	proxy->setup(_reference->changeIdentity(newIdentity));
	return proxy;
    }
}

ObjectPrx
IceProxy::Ice::Object::_twoway() const
{
    ReferencePtr ref = _reference->changeMode(Reference::ModeTwoway);
    if (ref == _reference)
    {
	return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
	ObjectPrx proxy(new ::IceProxy::Ice::Object());
	proxy->setup(ref);
	return proxy;
    }
}

ObjectPrx
IceProxy::Ice::Object::_oneway() const
{
    ReferencePtr ref = _reference->changeMode(Reference::ModeOneway);
    if (ref == _reference)
    {
	return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
	ObjectPrx proxy(new ::IceProxy::Ice::Object());
	proxy->setup(ref);
	return proxy;
    }
}

ObjectPrx
IceProxy::Ice::Object::_batchOneway() const
{
    ReferencePtr ref = _reference->changeMode(Reference::ModeBatchOneway);
    if (ref == _reference)
    {
	return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
	ObjectPrx proxy(new ::IceProxy::Ice::Object());
	proxy->setup(ref);
	return proxy;
    }
}

ObjectPrx
IceProxy::Ice::Object::_datagram() const
{
    ReferencePtr ref = _reference->changeMode(Reference::ModeDatagram);
    if (ref == _reference)
    {
	return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
	ObjectPrx proxy(new ::IceProxy::Ice::Object());
	proxy->setup(ref);
	return proxy;
    }
}

ObjectPrx
IceProxy::Ice::Object::_batchDatagram() const
{
    ReferencePtr ref = _reference->changeMode(Reference::ModeBatchDatagram);
    if (ref == _reference)
    {
	return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
	ObjectPrx proxy(new ::IceProxy::Ice::Object());
	proxy->setup(ref);
	return proxy;
    }
}

ObjectPrx
IceProxy::Ice::Object::_secure(bool b) const
{
    ReferencePtr ref = _reference->changeSecure(b);
    if (ref == _reference)
    {
	return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
	ObjectPrx proxy(new ::IceProxy::Ice::Object());
	proxy->setup(ref);
	return proxy;
    }
}

ObjectPrx
IceProxy::Ice::Object::_timeout(int t) const
{
    ReferencePtr ref = _reference->changeTimeout(t);
    if (ref == _reference)
    {
	return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
	ObjectPrx proxy(new ::IceProxy::Ice::Object());
	proxy->setup(ref);
	return proxy;
    }
}

ReferencePtr
IceProxy::Ice::Object::__reference() const
{
    return _reference;
}

void
IceProxy::Ice::Object::__copyTo(::IceProxy::Ice::Object* to) const
{
    to->setup(_reference);
}

void
IceProxy::Ice::Object::__handleException(const LocalException& ex, int& cnt)
{
    JTCSyncT<JTCMutex> sync(*this);

    _delegate = 0;
    static const int max = 1; // TODO: Make number of retries configurable

    try
    {
	ex.raise();
    }
    catch (const NoEndpointException&)
    {
	//
	// We always retry on a no endpoint exception, as we might
	// have a forwarded reference, but we retry with the original
	// reference.
	//
    }
    catch (const CloseConnectionException&)
    {
	//
	// We always retry on a close connection exception, as this
	// indicates graceful server shutdown.
	//
	// TODO: configurable timeout before we try again?
    }
    catch (const SocketException&)
    {
	++cnt;
    }
    catch (const DNSException&)
    {
	++cnt;
    }

#ifndef ICE_NO_TRACE
    TraceLevelsPtr traceLevels = _reference->instance->traceLevels();
    LoggerPtr logger = _reference->instance->logger();
#endif

    if(cnt > max)
    {
#ifndef ICE_NO_TRACE
	if (traceLevels->retry >= 1)
	{
	    ostringstream s;
	    s << "cannot retry operation call because retry limit has been exceeded\n" << ex;
	    logger->trace(traceLevels->retryCat, s.str());
	}
#endif	
	ex.raise();
    }

#ifndef ICE_NO_TRACE
    if (traceLevels->retry >= 1)
    {
	ostringstream s;
	s << "re-trying operation call because of exception\n" << ex;
	logger->trace(traceLevels->retryCat, s.str());
    }
#endif

    //
    // Reset the endpoints to the original endpoints upon retry
    //
    _reference = _reference->changeEndpoints(_reference->origEndpoints);
}

void
IceProxy::Ice::Object::__locationForward(const LocationForward& ex)
{
    JTCSyncT<JTCMutex> sync(*this);

    if (_reference->identity != ex._prx->_reference->identity)
    {
	throw ReferenceIdentityException(__FILE__, __LINE__);
    }

    _delegate = 0;
    _reference = _reference->changeEndpoints(ex._prx->_reference->endpoints);

/*
#ifndef ICE_NO_TRACE
    TraceLevelsPtr traceLevels = _reference->instance->traceLevels();
    LoggerPtr logger = _reference->instance->logger();

    if (traceLevels->locationForward >= 1)
    {
	ostringstream s;
	s << "location forward for object with identity `" << _reference.identity << "'";
	logger->trace(traceLevels->locationForwardCat, s.str());
    }
#endif
*/
}

IceProxy::Ice::Object::Object()
{
}

IceProxy::Ice::Object::~Object()
{
}

Handle< ::IceDelegate::Ice::Object>
IceProxy::Ice::Object::__getDelegate()
{
    JTCSyncT<JTCMutex> sync(*this);

    if (!_delegate)
    {
	ObjectPtr obj = _reference->instance->objectAdapterFactory()->proxyToObject(this);

	if (obj)
	{
	    _delegate = obj;
	}
	else
	{
	    _delegate = __createDelegateM();
	    _delegate->setup(_reference);
	}
    }

    return _delegate;
}

Handle< ::IceDelegateM::Ice::Object>
IceProxy::Ice::Object::__createDelegateM()
{
    return Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::Ice::Object);
}

void
IceProxy::Ice::Object::setup(const ReferencePtr& reference)
{
    //
    // No need to synchronize, as this operation is only called
    // upon initial initialization.
    //
    _reference = reference;
}

void
IceDelegate::Ice::Object::_flush()
{
    // Do nothing
}

IceDelegate::Ice::Object::Object()
{
}

IceDelegate::Ice::Object::~Object()
{
}

void
IceDelegate::Ice::Object::setup(const ReferencePtr&)
{
}

bool
IceDelegateM::Ice::Object::_isA(const string& s)
{
    Outgoing __out(__emitter(), __reference());
    Stream* __is = __out.is();
    Stream* __os = __out.os();
    __os->write("_isA");
    __os->write(s);
    if (!__out.invoke())
    {
	throw ::Ice::UnknownUserException(__FILE__, __LINE__);
    }
    bool __ret;
    __is->read(__ret);
    return __ret;
}

void
IceDelegateM::Ice::Object::_ping()
{
    Outgoing __out(__emitter(), __reference());
    Stream* __os = __out.os();
    __os->write("_ping");
    if (!__out.invoke())
    {
	throw ::Ice::UnknownUserException(__FILE__, __LINE__);
    }
}

void
IceDelegateM::Ice::Object::_flush()
{
    __emitter()->flushBatchRequest();
}

IceDelegateM::Ice::Object::Object()
{
}

IceDelegateM::Ice::Object::~Object()
{
}

const EmitterPtr&
IceDelegateM::Ice::Object::__emitter()
{
    return _emitter;
}

const ReferencePtr&
IceDelegateM::Ice::Object::__reference()
{
    return _reference;
}

void
IceDelegateM::Ice::Object::setup(const ReferencePtr& reference)
{
    //
    // No need to synchronize, as this operation is only called
    // upon initial initialization.
    //
    _reference = reference;
    vector<EndpointPtr> endpoints;
    switch (_reference->mode)
    {
	case Reference::ModeTwoway:
	case Reference::ModeOneway:
	case Reference::ModeBatchOneway:
	{
	    remove_copy_if(_reference->endpoints.begin(), _reference->endpoints.end(), back_inserter(endpoints), 
			   not1(::IceInternal::constMemFun(&Endpoint::regular)));
	    break;
	}
	
	case Reference::ModeDatagram:
	case Reference::ModeBatchDatagram:
	{
	    remove_copy_if(_reference->endpoints.begin(), _reference->endpoints.end(), back_inserter(endpoints),
			   not1(::IceInternal::constMemFun(&Endpoint::datagram)));
	    break;
	}
    }

    if (_reference->secure)
    {
	endpoints.erase(remove_if(endpoints.begin(), endpoints.end(),
				  not1(::IceInternal::constMemFun(&Endpoint::secure))),
			endpoints.end());
    }

    if (endpoints.empty())
    {
	throw NoEndpointException(__FILE__, __LINE__);
    }

    random_shuffle(endpoints.begin(), endpoints.end());

    EmitterFactoryPtr factory = _reference->instance->emitterFactory();
    _emitter = factory->create(endpoints);
}
