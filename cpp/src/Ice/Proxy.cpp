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
#include <Ice/Direct.h>
#include <Ice/Reference.h>
#include <Ice/Endpoint.h>
#include <Ice/Instance.h>
#include <Ice/Logger.h>
#include <Ice/TraceLevels.h>
#include <Ice/Emitter.h>
#include <Ice/BasicStream.h>
#include <Ice/Exception.h>
#include <Ice/Functional.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(::IceProxy::Ice::Object* p) { p->__incRef(); }
void IceInternal::decRef(::IceProxy::Ice::Object* p) { p->__decRef(); }

void IceInternal::incRef(::IceDelegate::Ice::Object* p) { p->__incRef(); }
void IceInternal::decRef(::IceDelegate::Ice::Object* p) { p->__decRef(); }

void IceInternal::incRef(::IceDelegateM::Ice::Object* p) { p->__incRef(); }
void IceInternal::decRef(::IceDelegateM::Ice::Object* p) { p->__decRef(); }

void IceInternal::incRef(::IceDelegateD::Ice::Object* p) { p->__incRef(); }
void IceInternal::decRef(::IceDelegateD::Ice::Object* p) { p->__decRef(); }

void
IceInternal::checkedCast(::IceProxy::Ice::Object* b, const string& f, ::IceProxy::Ice::Object*& d)
{
    // TODO: Check facet
    d = new ::IceProxy::Ice::Object;
    d->__copyFromWithFacet(b, f);
}

void
IceInternal::uncheckedCast(::IceProxy::Ice::Object* b, const string& f, ::IceProxy::Ice::Object*& d)
{
    d = new ::IceProxy::Ice::Object;
    d->__copyFromWithFacet(b, f);
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

bool
IceProxy::Ice::Object::_hasFacet(const string& s)
{
    int __cnt = 0;
    while (true)
    {
	try
	{
	    Handle< ::IceDelegate::Ice::Object> __del = __getDelegate();
	    return __del->_hasFacet(s);
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
    return _reference == r._reference;
}

bool
IceProxy::Ice::Object::operator<(const Object& r) const
{
    return _reference < r._reference;
}

Int
IceProxy::Ice::Object::_hash() const
{
    return _reference->hashValue;
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

std::string
IceProxy::Ice::Object::_getFacet() const
{
    return _reference->facet;
}

::Ice::ObjectPrx
IceProxy::Ice::Object::_newFacet(const std::string& newFacet) const
{
    if (newFacet == _reference->facet)
    {
	return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
	ObjectPrx proxy(new ::IceProxy::Ice::Object());
	proxy->setup(_reference->changeFacet(newFacet));
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
IceProxy::Ice::Object::__copyFrom(const ::IceProxy::Ice::Object* from)
{
    setup(from->__reference());
}

void
IceProxy::Ice::Object::__copyFromWithFacet(const ::IceProxy::Ice::Object* from, const string& facet)
{
    setup(from->__reference()->changeFacet(facet));
}

void
IceProxy::Ice::Object::__handleException(const LocalException& ex, int& cnt)
{
    JTCSyncT<JTCMutex> sync(*this);

    _delegate = 0;

    static const int max = 1; // TODO: Make number of retries configurable

    try
    {
	ex._throw();
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
    catch (const TimeoutException&)
    {
	++cnt;
    }

    TraceLevelsPtr traceLevels = _reference->instance->traceLevels();
    LoggerPtr logger = _reference->instance->logger();

    if(cnt > max)
    {
	if (traceLevels->retry >= 1)
	{
	    ostringstream s;
	    s << "cannot retry operation call because retry limit has been exceeded\n" << ex;
	    logger->trace(traceLevels->retryCat, s.str());
	}
	ex._throw();
    }

    if (traceLevels->retry >= 1)
    {
	ostringstream s;
	s << "re-trying operation call because of exception\n" << ex;
	logger->trace(traceLevels->retryCat, s.str());
    }

    //
    // Reset the endpoints to the original endpoints upon retry
    //
    _reference = _reference->changeEndpoints(_reference->origEndpoints);
}

void
IceProxy::Ice::Object::__locationForward(const LocationForward& ex)
{
    JTCSyncT<JTCMutex> sync(*this);

    _delegate = 0;

    if (_reference->identity != ex._prx->_reference->identity)
    {
	throw ReferenceIdentityException(__FILE__, __LINE__);
    }

    _reference = _reference->changeEndpoints(ex._prx->_reference->endpoints);

/*
    TraceLevelsPtr traceLevels = _reference->instance->traceLevels();
    LoggerPtr logger = _reference->instance->logger();

    if (traceLevels->locationForward >= 1)
    {
	ostringstream s;
	s << "location forward for object with identity `" << _reference.identity << "'";
	logger->trace(traceLevels->locationForwardCat, s.str());
    }
*/
}

void
IceProxy::Ice::Object::__rethrowException(const LocalException& ex)
{
    JTCSyncT<JTCMutex> sync(*this);

    _delegate = 0;

    ex._throw();
}

Handle< ::IceDelegate::Ice::Object>
IceProxy::Ice::Object::__getDelegate()
{
    JTCSyncT<JTCMutex> sync(*this);
    if (!_delegate)
    {
	ObjectAdapterPtr adapter = _reference->instance->objectAdapterFactory()->findObjectAdapter(this);
	if (adapter)
	{
	    Handle< ::IceDelegateD::Ice::Object> delegate = __createDelegateD();
	    delegate->setup(_reference, adapter);
	    _delegate = delegate;
	}
	else
	{
	    Handle< ::IceDelegateM::Ice::Object> delegate = __createDelegateM();
	    delegate->setup(_reference);
	    _delegate = delegate;
	}
    }

    return _delegate;
}

Handle< ::IceDelegateM::Ice::Object>
IceProxy::Ice::Object::__createDelegateM()
{
    return Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::Ice::Object);
}

Handle< ::IceDelegateD::Ice::Object>
IceProxy::Ice::Object::__createDelegateD()
{
    return Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD::Ice::Object);
}

void
IceProxy::Ice::Object::setup(const ReferencePtr& ref)
{
    //
    // No need to synchronize, as this operation is only called
    // upon initial initialization.
    //
    _reference = ref;
}

bool
IceDelegateM::Ice::Object::_isA(const string& s)
{
    Outgoing __out(__emitter, __reference);
    BasicStream* __is = __out.is();
    BasicStream* __os = __out.os();
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

bool
IceDelegateM::Ice::Object::_hasFacet(const string& s)
{
    Outgoing __out(__emitter, __reference);
    BasicStream* __is = __out.is();
    BasicStream* __os = __out.os();
    __os->write("_hasFacet");
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
    Outgoing __out(__emitter, __reference);
    BasicStream* __os = __out.os();
    __os->write("_ping");
    if (!__out.invoke())
    {
	throw ::Ice::UnknownUserException(__FILE__, __LINE__);
    }
}

void
IceDelegateM::Ice::Object::_flush()
{
    __emitter->flushBatchRequest();
}

void
IceDelegateM::Ice::Object::setup(const ReferencePtr& ref)
{
    //
    // No need to synchronize, as this operation is only called
    // upon initial initialization.
    //
    __reference = ref;

    vector<EndpointPtr> endpoints;
    switch (__reference->mode)
    {
	case Reference::ModeTwoway:
	case Reference::ModeOneway:
	case Reference::ModeBatchOneway:
	{
	    remove_copy_if(__reference->endpoints.begin(), __reference->endpoints.end(), back_inserter(endpoints), 
			   ::Ice::constMemFun(&Endpoint::datagram));
	    break;
	}
	
	case Reference::ModeDatagram:
	case Reference::ModeBatchDatagram:
	{
	    remove_copy_if(__reference->endpoints.begin(), __reference->endpoints.end(), back_inserter(endpoints),
			   not1(::Ice::constMemFun(&Endpoint::datagram)));
	    break;
	}
    }

    if (__reference->secure)
    {
	endpoints.erase(remove_if(endpoints.begin(), endpoints.end(), not1(::Ice::constMemFun(&Endpoint::secure))),
			endpoints.end());
    }
    else
    {
	endpoints.erase(remove_if(endpoints.begin(), endpoints.end(), ::Ice::constMemFun(&Endpoint::secure)),
			endpoints.end());
    }

    if (endpoints.empty())
    {
	throw NoEndpointException(__FILE__, __LINE__);
    }

    random_shuffle(endpoints.begin(), endpoints.end());

    EmitterFactoryPtr factory = __reference->instance->emitterFactory();
    __emitter = factory->create(endpoints);
    assert(__emitter);
}

bool
IceDelegateD::Ice::Object::_isA(const string& s)
{
    Direct __direct(__adapter, __reference, "_isA");
    return __direct.servant()->_isA(s);
}

bool
IceDelegateD::Ice::Object::_hasFacet(const string& s)
{
    Direct __direct(__adapter, __reference, "_hasFacet");
    return __direct.servant()->_hasFacet(s);
}

void
IceDelegateD::Ice::Object::_ping()
{
    Direct __direct(__adapter, __reference, "_ping");
    __direct.servant()->_ping();
}

void
IceDelegateD::Ice::Object::_flush()
{
    // Nothing to do for direct delegates
}

void
IceDelegateD::Ice::Object::setup(const ReferencePtr& ref, const ObjectAdapterPtr& adapter)
{
    //
    // No need to synchronize, as this operation is only called
    // upon initial initialization.
    //
    __reference = ref;
    __adapter = adapter;
}

