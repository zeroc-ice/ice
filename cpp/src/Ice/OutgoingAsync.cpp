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
    __os(0)
{
}

IceInternal::OutgoingAsync::~OutgoingAsync()
{
    assert(!__is);
    assert(!__os);
}

void
IceInternal::OutgoingAsync::__finished(BasicStream& is)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(_monitor);

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
		throw UnknownReplyStatusException(__FILE__, __LINE__);
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

    cleanup();
}

void
IceInternal::OutgoingAsync::__finished(const LocalException& exc)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(_monitor);

    if(_reference)
    {
	if(_reference->locatorInfo)
	{
	    _reference->locatorInfo->clearObjectCache(_reference);
	}
	
	bool doRetry = false;
	
	//
	// A CloseConnectionException indicates graceful server
	// shutdown, and is therefore always repeatable without
	// violating "at-most-once". That's because by sending a close
	// connection message, the server guarantees that all
	// outstanding requests can safely be repeated. Otherwise, we
	// can also retry if the operation mode Nonmutating or
	// Idempotent.
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
	    _connection = 0;
	    __send();
	    return;
	}
    }
    
    try
    {
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

    cleanup();
}

void
IceInternal::OutgoingAsync::__prepare(const ReferencePtr& ref, const string& operation, OperationMode mode,
				      const Context& context)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(_monitor);

    try
    {
	//
	// We must first wait for other requests to finish.
	//
	while(_reference)
	{
	    _monitor.wait();
	}
	
	_reference = ref;
	assert(!_connection);
	_connection = _reference->getConnection();
	_cnt = 0;
	_mode = mode;
	assert(!__is);
	__is = new BasicStream(_reference->instance.get());
	assert(!__os);
	__os = new BasicStream(_reference->instance.get());
	
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
    catch(const LocalException& ex)
    {
	cleanup();
	ex.ice_throw();
    }
}

void
IceInternal::OutgoingAsync::__send()
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(_monitor);
    
    try
    {
	while(true)
	{
	    if(!_connection)
	    {
		_connection = _reference->getConnection();
	    }
	    
	    try
	    {
		_connection->sendAsyncRequest(__os, this);
		
		//
		// Don't do anything after sendAsyncRequest() returned
		// without an exception.  I such case, there will be
		// callbacks, i.e., calls to the __finished()
		// functions. Since there is no mutex protection, we
		// cannot modify state here and in such callbacks.
		//
		return;
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
	__finished(ex);
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
IceInternal::OutgoingAsync::cleanup()
{
    if(_reference)
    {
	_reference = 0;
    }
    
    if(_connection)
    {
	_connection = 0;
    }
    
    if(__is)
    {
	delete __is;
	__is = 0;
    }
    
    if(__os)
    {
	delete __os;
	__os = 0;
    }

    _monitor.notify();
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
    ice_response(ok, outParams);
}
