// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/OutgoingAsync.h>
#include <Ice/Object.h>
#include <Ice/Connection.h>
#include <Ice/Reference.h>
#include <Ice/Instance.h>
#include <Ice/LocalException.h>
#include <Ice/Properties.h>
#include <Ice/LoggerUtil.h>
#include <Ice/LocatorInfo.h>
#include <Ice/ProxyFactory.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(OutgoingAsync* p) { p->__incRef(); }
void IceInternal::decRef(OutgoingAsync* p) { p->__decRef(); }

void IceInternal::incRef(AMI_Object_ice_invoke* p) { p->__incRef(); }
void IceInternal::decRef(AMI_Object_ice_invoke* p) { p->__decRef(); }

IceInternal::OutgoingAsync::OutgoingAsync() :
    __is(0),
    __os(0),
    _state(StateUnsent)
{
}

IceInternal::OutgoingAsync::~OutgoingAsync()
{
    assert(!_reference);
    assert(!_connection);
    assert(!__is);
    assert(!__os);
}

void
IceInternal::OutgoingAsync::__finished(BasicStream& is)
{
    //
    // Wait until sending has completed.
    //
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
	while(_state < StateSent)
	{
	    _monitor.wait();
	}
    }

    assert(_reference);
    assert(_connection);
    assert(__is);
    assert(__os);

    DispatchStatus status;
    
    try
    {
	__is->swap(is);

	Byte b;
	__is->read(b);
	status = static_cast<DispatchStatus>(b);
	
	switch(status)
	{
	    case DispatchOK:
	    case DispatchUserException:
	    {
		__is->startReadEncaps();
		break;
	    }
	    
	    case DispatchObjectNotExist:
	    {
		ObjectNotExistException ex(__FILE__, __LINE__);
		ex.id.__read(__is);
		__is->read(ex.facet);
		__is->read(ex.operation);
		throw ex;
	    }
	    
	    case DispatchFacetNotExist:
	    {
		FacetNotExistException ex(__FILE__, __LINE__);
		ex.id.__read(__is);
		__is->read(ex.facet);
		__is->read(ex.operation);
		throw ex;
	    }
	    
	    case DispatchOperationNotExist:
	    {
		OperationNotExistException ex(__FILE__, __LINE__);
		ex.id.__read(__is);
		__is->read(ex.facet);
		__is->read(ex.operation);
		throw ex;
	    }
	    
	    case DispatchUnknownException:
	    {
		UnknownException ex(__FILE__, __LINE__);
		__is->read(ex.unknown);
		throw ex;
	    }
	    
	    case DispatchUnknownLocalException:
	    {
		UnknownLocalException ex(__FILE__, __LINE__);
		__is->read(ex.unknown);
		throw ex;
	    }
	    
	    case DispatchUnknownUserException:
	    {
		UnknownUserException ex(__FILE__, __LINE__);
		__is->read(ex.unknown);
		throw ex;
	    }
	    
	    default:
	    {
		UnknownReplyStatusException ex(__FILE__, __LINE__);
		throw ex;
	    }
	}
    }
    catch(const LocalException& ex)
    {
	__finished(ex);
	return;
    }

    assert(status == DispatchOK || status == DispatchUserException);

    try
    {
	__response(status == DispatchOK);
    }
    catch(const Exception& ex)
    {
	warning(ex);
    }
    catch(const std::exception& ex)
    {
	warning(ex);
    }
    catch(...)
    {
	warning();
    }
}

void
IceInternal::OutgoingAsync::__finished(const LocalException& exc)
{
    //
    // Wait until sending has completed.
    //
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
	while(_state < StateSent)
	{
	    _monitor.wait();
	}
    }

    assert(_reference);
    assert(_connection);
    assert(__is);
    assert(__os);

    if(_reference->locatorInfo)
    {
	_reference->locatorInfo->clearObjectCache(_reference);
    }
    
    bool doRetry = false;

    //
    // A CloseConnectionException indicates graceful server shutdown,
    // and is therefore always repeatable without violating
    // "at-most-once". That's because by sending a close connection
    // message, the server guarantees that all outstanding requests
    // can safely be repeated. Otherwise, we can also retry if the
    // operation mode Nonmutating or Idempotent.
    //
    if(_mode == Nonmutating || _mode == Idempotent || dynamic_cast<const CloseConnectionException*>(&exc))
    {
	try
	{
	    ProxyFactoryPtr proxyFactory = _reference->instance->proxyFactory();
	    if(proxyFactory)
	    {
		proxyFactory->checkRetryAfterException(exc, _cnt);
	    }
	    else
	    {
		exc.ice_throw(); // The communicator is already destroyed, so we cannot retry.
	    }

	    doRetry = true;
	}
	catch(const LocalException&)
	{
	}
    }

    if(doRetry)
    {
	{
	    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
	    assert(_state == StateSent);
	    _state = StateUnsent;
	}
	_connection = 0;
	__send();
    }
    else
    {
	try
	{
	    __cleanup();
	    ice_exception(exc);
	}
	catch(const Exception& ex)
	{
	    warning(ex);
	}
	catch(const std::exception& ex)
	{
	    warning(ex);
	}
	catch(...)
	{
	    warning();
	}
    }
}

bool
IceInternal::OutgoingAsync::__timedOut() const
{
    if(_connection && _connection->timeout() >= 0)
    {
	return IceUtil::Time::now() >= _absoluteTimeout;
    }
    else
    {
	return false;
    }
}

void
IceInternal::OutgoingAsync::__prepare(const ReferencePtr& ref, const string& operation, OperationMode mode,
				      const Context& context)
{
    assert(!_reference);
    assert(!_connection);
    assert(!__is);
    assert(!__os);
    assert(_state == StateUnsent);

    _reference = ref;
    _connection = _reference->getConnection();
    __is = new BasicStream(_reference->instance.get());
    __os = new BasicStream(_reference->instance.get());

    _cnt = 0;
    _mode = mode;

    _connection->prepareRequest(__os);
    _reference->identity.__write(__os);
    __os->write(_reference->facet);
    __os->write(operation);
    __os->write(static_cast<Byte>(_mode));
    __os->writeSize(Int(context.size()));
    Context::const_iterator p;
    for(p = context.begin(); p != context.end(); ++p)
    {
	__os->write(p->first);
	__os->write(p->second);
    }

    __os->startWriteEncaps();
}

void
IceInternal::OutgoingAsync::__send()
{
    assert(_reference);
    //assert(_connection); // Might be 0, in case we retry from __finished().
    assert(__is);
    assert(__os);
    assert(_state == StateUnsent);

    try
    {
	while(true)
	{
	    if(!_connection)
	    {
		_connection = _reference->getConnection();
	    }

	    if(_connection->timeout() >= 0)
	    {
		_absoluteTimeout = IceUtil::Time::now() + IceUtil::Time::milliSeconds(_connection->timeout());
	    }

	    try
	    {
		_connection->sendAsyncRequest(__os, this);
		break;
	    }
	    catch(const LocalException& ex)
	    {
		if(_reference->locatorInfo)
		{
		    _reference->locatorInfo->clearObjectCache(_reference);
		}
		
		ProxyFactoryPtr proxyFactory = _reference->instance->proxyFactory();
		if(proxyFactory)
		{
		    proxyFactory->checkRetryAfterException(ex, _cnt);
		}
		else
		{
		    ex.ice_throw(); // The communicator is already destroyed, so we cannot retry.
		}
	    }

	    _connection = 0;
	}
    }
    catch(const LocalException& ex)
    {
	{
	    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
	    assert(_state == StateUnsent);
	    _state = StateSent;
	    _monitor.notify();
	}

	__finished(ex);
	return;
    }
    
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
	assert(_state == StateUnsent);
	_state = StateSent;
	_monitor.notify();
    }
}

void
IceInternal::OutgoingAsync::__cleanup()
{
    assert(_reference);
    assert(_connection);
    assert(__is);
    assert(__os);

    _reference = 0;
    _connection = 0;
    delete __is;
    __is = 0;
    delete __os;
    __os = 0;

    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
	assert(_state == StateSent);
	_state = StateUnsent;
    }
}

void
IceInternal::OutgoingAsync::warning(const Exception& ex) const
{
    if(_reference->instance->properties()->getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
    {
	Warning out(_reference->instance->logger());
	out << "Ice::Exception raised by AMI callback:\n" << ex;
    }
}

void
IceInternal::OutgoingAsync::warning(const std::exception& ex) const
{
    if(_reference->instance->properties()->getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
    {
	Warning out(_reference->instance->logger());
	out << "std::exception raised by AMI callback:\n" << ex.what();
    }
}

void
IceInternal::OutgoingAsync::warning() const
{
    if(_reference->instance->properties()->getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
    {
	Warning out(_reference->instance->logger());
	out << "unknown exception raised by AMI callback";
    }
}

void
Ice::AMI_Object_ice_invoke::__invoke(const IceInternal::ReferencePtr& ref, const string& operation, OperationMode mode,
				     const vector<Byte>& inParams, const Context& context)
{
    try
    {
	__prepare(ref, operation, mode, context);
	__os->writeBlob(inParams);
	__os->endWriteEncaps();
    }
    catch(const LocalException& ex)
    {
	__finished(ex);
	return;
    }
    __send();
}

void
Ice::AMI_Object_ice_invoke::__response(bool ok) // ok == true means no user exception.
{
    vector<Byte> outParams;
    try
    {
	Int sz = __is->getReadEncapsSize();
	__is->readBlob(outParams, sz);
    }
    catch(const LocalException& ex)
    {
	__finished(ex);
	return;
    }
    __cleanup();
    ice_response(ok, outParams);
}
