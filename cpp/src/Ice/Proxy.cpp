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
#include <Ice/ProxyFactory.h>
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
#include <Ice/SecurityException.h> // TODO: bandaid, see below.

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
IceInternal::checkedCast(const ObjectPrx& b, const string& f, ObjectPrx& d)
{
    d = 0;
    if (b)
    {
	if (f == b->ice_getFacet())
	{
	    d = b;
	}
	else
	{
	    ObjectPrx bb = b->ice_newFacet(f);
	    try
	    {
#ifdef NDEBUG
		bb->ice_isA("::Ice::Object");
#else
		bool ok = bb->ice_isA("::Ice::Object");
		assert(ok);
#endif
		d = bb;
	    }
	    catch (const FacetNotExistException&)
	    {
	    }
	}
    }
}

void
IceInternal::uncheckedCast(const ObjectPrx& b, const string& f, ObjectPrx& d)
{
    d = 0;
    if (b)
    {
	d = b->ice_newFacet(f);
    }
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
IceProxy::Ice::Object::ice_hash() const
{
    return _reference->hashValue;
}
bool
IceProxy::Ice::Object::ice_isA(const string& __id, const Context& __context)
{
    int __cnt = 0;
    while (true)
    {
	try
	{
	    Handle< ::IceDelegate::Ice::Object> __del = __getDelegate();
	    return __del->ice_isA(__id, __context);
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
IceProxy::Ice::Object::ice_ping(const Context& __context)
{
    int __cnt = 0;
    while (true)
    {
	try
	{
	    Handle< ::IceDelegate::Ice::Object> __del = __getDelegate();
	    __del->ice_ping(__context);
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
IceProxy::Ice::Object::ice_invoke(const string& operation,
				  bool nonmutating,
				  const vector<Byte>& inParams,
				  vector<Byte>& outParams,
				  const Context& __context)
{
    int __cnt = 0;
    while (true)
    {
	try
	{
	    Handle< ::IceDelegate::Ice::Object> __del = __getDelegate();
	    __del->ice_invoke(operation, nonmutating, inParams, outParams, __context);
	    return;
	}
	catch (const LocationForward& __ex)
	{
	    __locationForward(__ex);
	}
	catch (const NonRepeatable& __ex)
	{
	    if (nonmutating)
	    {
		__handleException(*__ex.get(), __cnt);
	    }
	    else
	    {
		__rethrowException(*__ex.get());
	    }
	}
	catch (const LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

Identity
IceProxy::Ice::Object::ice_getIdentity() const
{
    return _reference->identity;
}

ObjectPrx
IceProxy::Ice::Object::ice_newIdentity(const Identity& newIdentity) const
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

string
IceProxy::Ice::Object::ice_getFacet() const
{
    return _reference->facet;
}

ObjectPrx
IceProxy::Ice::Object::ice_newFacet(const string& newFacet) const
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
IceProxy::Ice::Object::ice_twoway() const
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
IceProxy::Ice::Object::ice_oneway() const
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
IceProxy::Ice::Object::ice_batchOneway() const
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
IceProxy::Ice::Object::ice_datagram() const
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
IceProxy::Ice::Object::ice_batchDatagram() const
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
IceProxy::Ice::Object::ice_secure(bool b) const
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
IceProxy::Ice::Object::ice_timeout(int t) const
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

void
IceProxy::Ice::Object::ice_flush()
{
    Handle< ::IceDelegate::Ice::Object> __del = __getDelegate();
    __del->ice_flush();
}

ReferencePtr
IceProxy::Ice::Object::__reference() const
{
    return _reference;
}

void
IceProxy::Ice::Object::__copyFrom(const ObjectPrx& from)
{
    setup(from->__reference());
}

void
IceProxy::Ice::Object::__handleException(const LocalException& ex, int& cnt)
{
    JTCSyncT<JTCMutex> sync(*this);

    _delegate = 0;

    static const int max = 1; // TODO: Make number of retries configurable

    try
    {
	ex.ice_throw();
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
    catch (const IceSecurity::SecurityException&) // TODO: bandaid to make retry w/ ssl work.
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
	ex.ice_throw();
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

    ex.ice_throw();
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
IceDelegateM::Ice::Object::ice_isA(const string& __id, const Context& __context)
{
    bool __sendProxy = false;
    while (true)
    {
	try
	{
	    Outgoing __out(__emitter, __reference, __sendProxy, "ice_isA", true, __context);
	    BasicStream* __is = __out.is();
	    BasicStream* __os = __out.os();
	    __os->write(__id);
	    if (!__out.invoke())
	    {
		throw ::Ice::UnknownUserException(__FILE__, __LINE__);
	    }
	    bool __ret;
	    __is->read(__ret);
	    return __ret;
	}
	catch (const ProxyRequested&)
	{
	    __sendProxy = true;
	}
    }
}

void
IceDelegateM::Ice::Object::ice_ping(const Context& __context)
{
    bool __sendProxy = false;
    while (true)
    {
	try
	{
	    Outgoing __out(__emitter, __reference, __sendProxy, "ice_ping", true, __context);
	    if (!__out.invoke())
	    {
		throw ::Ice::UnknownUserException(__FILE__, __LINE__);
	    }
	    return;
	}
	catch (const ProxyRequested&)
	{
	    __sendProxy = true;
	}
    }
}

void
IceDelegateM::Ice::Object::ice_invoke(const string& operation,
				      bool nonmutating,
				      const vector<Byte>& inParams,
				      vector<Byte>& outParams,
				      const Context& __context)
{
    bool __sendProxy = false;
    while (true)
    {
	try
	{
	    Outgoing __out(__emitter, __reference, __sendProxy, operation.c_str(), nonmutating, __context);
	    BasicStream* __os = __out.os();
	    __os->writeBlob(inParams);
	    __out.invoke();
	    if (__reference->mode == Reference::ModeTwoway)
	    {
		BasicStream* __is = __out.is();
		Int sz = __is->getReadEncapsSize();
		__is->readBlob(outParams, sz);
	    }
	    return;
	}
	catch (const ProxyRequested&)
	{
	    __sendProxy = true;
	}
    }
}

void
IceDelegateM::Ice::Object::ice_flush()
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
IceDelegateD::Ice::Object::ice_isA(const string& __id, const Context& __context)
{
    Current __current;
    __initCurrent(__current, "ice_isA", true, __context);
    while (true)
    {
	Direct __direct(__adapter, __current);
	try
	{
	    return __direct.facetServant()->ice_isA(__id, __current);
	}
	catch (const ProxyRequested&)
	{
	    __initCurrentProxy(__current);
	}
	catch (const LocalException&)
	{
	    throw UnknownLocalException(__FILE__, __LINE__);
	}
	catch (const UserException&)
	{
	    throw UnknownUserException(__FILE__, __LINE__);
	}
	catch (...)
	{
	    throw UnknownException(__FILE__, __LINE__);
	}
    }
}

void
IceDelegateD::Ice::Object::ice_ping(const ::Ice::Context& __context)
{
    Current __current;
    __initCurrent(__current, "ice_ping", true, __context);
    while (true)
    {
	Direct __direct(__adapter, __current);
	try
	{
	    __direct.facetServant()->ice_ping(__current);
	    return;
	}
	catch (const ProxyRequested&)
	{
	    __initCurrentProxy(__current);
	}
	catch (const LocalException&)
	{
	    throw UnknownLocalException(__FILE__, __LINE__);
	}
	catch (const UserException&)
	{
	    throw UnknownUserException(__FILE__, __LINE__);
	}
	catch (...)
	{
	    throw UnknownException(__FILE__, __LINE__);
	}
    }
}

void
IceDelegateD::Ice::Object::ice_invoke(const string& operation,
				      bool nonmutating,
				      const vector<Byte>& inParams,
				      vector<Byte>& outParams,
				      const ::Ice::Context& context)
{
    Current __current;
    __initCurrent(__current, operation, nonmutating, context);
    while (true)
    {
	Direct __direct(__adapter, __current);
	Blobject* __servant = dynamic_cast<Blobject*>(__direct.facetServant().get());
	if (!__servant)
	{
	    throw OperationNotExistException(__FILE__, __LINE__);
	}
	try
	{
	    __servant->ice_invoke(inParams, outParams, __current);
	    return;
	}
	catch (const ProxyRequested&)
	{
	    __initCurrentProxy(__current);
	}
	catch (const LocalException&)
	{
	    throw UnknownLocalException(__FILE__, __LINE__);
	}
	catch (const UserException&)
	{
	    throw UnknownUserException(__FILE__, __LINE__);
	}
	catch (...)
	{
	    throw UnknownException(__FILE__, __LINE__);
	}
    }
}

void
IceDelegateD::Ice::Object::ice_flush()
{
    // Nothing to do for direct delegates
}

void
IceDelegateD::Ice::Object::__initCurrent(Current& current, const string& op, bool nonmutating, const Context& context)
{
    current.identity = __reference->identity;
    current.facet = __reference->facet;
    current.operation = op;
    current.nonmutating = nonmutating;
    current.context = context;
}

void
IceDelegateD::Ice::Object::__initCurrentProxy(Current& current)
{
    current.proxy = __reference->instance->proxyFactory()->referenceToProxy(__reference);
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
