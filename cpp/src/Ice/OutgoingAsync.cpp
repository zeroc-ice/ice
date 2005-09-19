// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/OutgoingAsync.h>
#include <Ice/Object.h>
#include <Ice/ConnectionI.h>
#include <Ice/Reference.h>
#include <Ice/Instance.h>
#include <Ice/LocalException.h>
#include <Ice/Properties.h>
#include <Ice/LoggerUtil.h>
#include <Ice/LocatorInfo.h>
#include <Ice/ProxyFactory.h>
#include <Ice/RouterInfo.h>

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
	    case DispatchFacetNotExist:
	    case DispatchOperationNotExist:
	    {
		Identity ident;
		ident.__read(__is);
		
		//
		// For compatibility with the old FacetPath.
		//
		vector<string> facetPath;
		__is->read(facetPath);
		string facet;
		if(!facetPath.empty())
		{
		    if(facetPath.size() > 1)
		    {
			throw MarshalException(__FILE__, __LINE__);
		    }
		    facet.swap(facetPath[0]);
		}
		
		string operation;
		__is->read(operation);
		
		auto_ptr<RequestFailedException> ex;
		switch(static_cast<DispatchStatus>(status))
		{
		    case DispatchObjectNotExist:
		    {
			ex.reset(new ObjectNotExistException(__FILE__, __LINE__));
			break;
		    }
		    
		    case DispatchFacetNotExist:
		    {
			ex.reset(new FacetNotExistException(__FILE__, __LINE__));
			break;
		    }
		    
		    case DispatchOperationNotExist:
		    {
			ex.reset(new OperationNotExistException(__FILE__, __LINE__));
			break;
		    }
		    
		    default:
		    {
			assert(false);
			break;
		    }
		}

		ex->id = ident;
		ex->facet = facet;
		ex->operation = operation;
		ex->ice_throw();
	    }
	    
	    case DispatchUnknownException:
	    case DispatchUnknownLocalException:
	    case DispatchUnknownUserException:
	    {
		string unknown;
		__is->read(unknown);
		
		auto_ptr<UnknownException> ex;
		switch(static_cast<DispatchStatus>(status))
		{
		    case DispatchUnknownException:
		    {
			ex.reset(new UnknownException(__FILE__, __LINE__));
			break;
		    }
		    
		    case DispatchUnknownLocalException:
		    {
			ex.reset(new UnknownLocalException(__FILE__, __LINE__));
			break;
		    }
		    
		    case DispatchUnknownUserException:
		    {
			ex.reset(new UnknownUserException(__FILE__, __LINE__));
			break;
		    }
		    
		    default:
		    {
			assert(false);
			break;
		    }
		}
		
		ex->unknown = unknown;
		ex->ice_throw();
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

    //
    // It is possible for __response to trigger a retry.
    // If it does we must not perform a cleanup.
    //
    bool retry = false;
    try
    {
	retry = __response(status == DispatchOK);
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

    if(!retry)
    {
        cleanup();
    }
}

bool
IceInternal::OutgoingAsync::__finished(const LocalException& exc)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(_monitor);

    if(_reference)
    {
	bool doRetry = false;
	
	//
	// A CloseConnectionException indicates graceful server
	// shutdown, and is therefore always repeatable without
	// violating "at-most-once". That's because by sending a close
	// connection message, the server guarantees that all
	// outstanding requests can safely be repeated. Otherwise, we
	// can also retry if the operation mode is Nonmutating or
	// Idempotent.
	//
	// An ObjectNotExistException can always be retried as
	// well without violating "at-most-once".
	//
	if(_mode == Nonmutating || _mode == Idempotent || dynamic_cast<const CloseConnectionException*>(&exc) ||
	   dynamic_cast<const ObjectNotExistException*>(&exc))
	{
	    try
	    {
		ProxyFactoryPtr proxyFactory = _reference->getInstance()->proxyFactory();
		if(proxyFactory)
		{
		    proxyFactory->checkRetryAfterException(exc, _reference, _cnt);
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
	    return true;
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
    return false;
}

void
IceInternal::OutgoingAsync::__prepare(const ObjectPrx& prx, const string& operation, OperationMode mode,
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
	
	_reference = prx->__reference();
	assert(!_connection);
	_connection = _reference->getConnection(_compress);
	_cnt = 0;
	_mode = mode;
	assert(!__is);
	__is = new BasicStream(_reference->getInstance().get());
	assert(!__os);
	__os = new BasicStream(_reference->getInstance().get());
	
	//
	// If we are using a router, then add the proxy to the router info object.
	//
	RoutableReferencePtr rr = RoutableReferencePtr::dynamicCast(_reference);
	if(rr && rr->getRouterInfo())
	{
	    rr->getRouterInfo()->addProxy(prx);
	}

	_connection->prepareRequest(__os);

	_reference->getIdentity().__write(__os);

	//
	// For compatibility with the old FacetPath.
	//
	if(_reference->getFacet().empty())
	{
	    __os->write(vector<string>());
	}
	else
	{
	    vector<string> facetPath;
	    facetPath.push_back(_reference->getFacet());
	    __os->write(facetPath);
	}

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
		_connection = _reference->getConnection(_compress);
	    }
	    
	    try
	    {
		_connection->sendAsyncRequest(__os, this, _compress);
		
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
		ProxyFactoryPtr proxyFactory = _reference->getInstance()->proxyFactory();
		if(proxyFactory)
		{
		    proxyFactory->checkRetryAfterException(ex, _reference, _cnt);
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
    if(_reference) // Don't print anything if cleanup() was already called.
    {
	if(_reference->getInstance()->properties()->getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
	{
	    Warning out(_reference->getInstance()->logger());
	    out << "Ice::Exception raised by AMI callback:\n" << ex;
	}
    }
}

void
IceInternal::OutgoingAsync::warning(const std::exception& ex) const
{
    if(_reference) // Don't print anything if cleanup() was already called.
    {
	if(_reference->getInstance()->properties()->getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
	{
	    Warning out(_reference->getInstance()->logger());
	    out << "std::exception raised by AMI callback:\n" << ex.what();
	}
    }
}

void
IceInternal::OutgoingAsync::warning() const
{
    if(_reference) // Don't print anything if cleanup() was already called.
    {
	if(_reference->getInstance()->properties()->getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
	{
	    Warning out(_reference->getInstance()->logger());
	    out << "unknown exception raised by AMI callback";
	}
    }
}

void
IceInternal::OutgoingAsync::cleanup()
{
    _reference = 0;
    _connection = 0;
    delete __is;
    __is = 0;
    delete __os;
    __os = 0;

    _monitor.notify();
}

void
Ice::AMI_Object_ice_invoke::__invoke(const ObjectPrx& prx, const string& operation, OperationMode mode,
				     const vector<Byte>& inParams, const Context& context)
{
    try
    {
	__prepare(prx, operation, mode, context);
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

bool
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
	return __finished(ex);
    }
    ice_response(ok, outParams);
    return false;
}
