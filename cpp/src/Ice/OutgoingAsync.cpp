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
#include <Ice/LocalException.h>
#include <Ice/Instance.h>
#include <Ice/Properties.h>
#include <Ice/LoggerUtil.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(OutgoingAsync* p) { p->__incRef(); }
void IceInternal::decRef(OutgoingAsync* p) { p->__decRef(); }

void IceInternal::incRef(AMI_Object_ice_invoke* p) { p->__incRef(); }
void IceInternal::decRef(AMI_Object_ice_invoke* p) { p->__decRef(); }

IceInternal::OutgoingAsync::OutgoingAsync() :
    _is(0),
    _os(0)
{
}

IceInternal::OutgoingAsync::~OutgoingAsync()
{
    delete _is;
    delete _os;
}

void
IceInternal::OutgoingAsync::__setup(const ConnectionPtr& connection, const ReferencePtr& ref,
				    const string& operation, OperationMode mode, const Context& context)
{
    _connection = connection;
    _instance = ref->instance;
    delete _is;
    delete _os;
    _is = new BasicStream(_instance.get());
    _os = new BasicStream(_instance.get());

    _connection->prepareRequest(_os);
    
    ref->identity.__write(_os);
    _os->write(ref->facet);
    _os->write(operation);
    _os->write(static_cast<Byte>(mode));
    _os->writeSize(Int(context.size()));
    Context::const_iterator p;
    for(p = context.begin(); p != context.end(); ++p)
    {
	_os->write(p->first);
	_os->write(p->second);
    }
    
    //
    // Input and output parameters are always sent in an
    // encapsulation, which makes it possible to forward requests as
    // blobs.
    //
    _os->startWriteEncaps();
}

void
IceInternal::OutgoingAsync::__invoke()
{
    _os->endWriteEncaps();

    try
    {
	_connection->sendAsyncRequest(this);
    }
    catch(const LocalException&)
    {
	//
	// Twoway requests report exceptions using finished().
	//
	assert(false);
    }

    if(_connection->timeout() >= 0)
    {
	_absoluteTimeout = IceUtil::Time::now() + IceUtil::Time::milliSeconds(_connection->timeout());
    }
}

void
IceInternal::OutgoingAsync::__finished(BasicStream& is)
{
    try
    {
	_is->swap(is);
	Byte status;
	_is->read(status);

	switch(static_cast<DispatchStatus>(status))
	{
	    case DispatchOK:
	    {
		_is->startReadEncaps();
		__response(true);
		break;
	    }
	    
	    case DispatchUserException:
	    {
		_is->startReadEncaps();
		__response(false);
		break;
	    }
	    
	    case DispatchObjectNotExist:
	    {
		ObjectNotExistException ex(__FILE__, __LINE__);
		ex.id.__read(_is);
		_is->read(ex.facet);
		_is->read(ex.operation);
		ice_exception(ex);
		break;
	    }

	    case DispatchFacetNotExist:
	    {
		FacetNotExistException ex(__FILE__, __LINE__);
		ex.id.__read(_is);
		_is->read(ex.facet);
		_is->read(ex.operation);
		ice_exception(ex);
		break;
	    }
	    
	    case DispatchOperationNotExist:
	    {
		OperationNotExistException ex(__FILE__, __LINE__);
		ex.id.__read(_is);
		_is->read(ex.facet);
		_is->read(ex.operation);
		ice_exception(ex);
		break;
	    }
	    
	    case DispatchUnknownException:
	    {
		UnknownException ex(__FILE__, __LINE__);
		_is->read(ex.unknown);
		ice_exception(ex);
		break;
	    }
	    
	    case DispatchUnknownLocalException:
	    {
		UnknownLocalException ex(__FILE__, __LINE__);
		_is->read(ex.unknown);
		ice_exception(ex);
		break;
	    }
	    
	    case DispatchUnknownUserException:
	    {
		UnknownUserException ex(__FILE__, __LINE__);
		_is->read(ex.unknown);
		ice_exception(ex);
		break;
	    }
	    
	    default:
	    {
		ice_exception(UnknownReplyStatusException(__FILE__, __LINE__));
		break;
	    }
	}
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
}

bool
IceInternal::OutgoingAsync::__timedOut() const
{
    if(_connection->timeout() >= 0)
    {
	return IceUtil::Time::now() >= _absoluteTimeout;
    }
    else
    {
	return false;
    }
}

BasicStream*
IceInternal::OutgoingAsync::__is()
{
    return _is;
}

BasicStream*
IceInternal::OutgoingAsync::__os()
{
    return _os;
}

void
IceInternal::OutgoingAsync::warning(const Exception& ex) const
{
    if(_instance->properties()->getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
    {
	Warning out(_instance->logger());
	out << "Ice::Exception raised by AMI callback:\n" << ex;
    }
}

void
IceInternal::OutgoingAsync::warning(const std::exception& ex) const
{
    if(_instance->properties()->getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
    {
	Warning out(_instance->logger());
	out << "std::exception raised by AMI callback:\n" << ex.what();
    }
}

void
IceInternal::OutgoingAsync::warning() const
{
    if(_instance->properties()->getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
    {
	Warning out(_instance->logger());
	out << "unknown exception raised by AMI callback";
    }
}

void
Ice::AMI_Object_ice_invoke::__response(bool ok) // ok == true means no user exception.
{
    vector<Byte> outParams;
    try
    {
	BasicStream* __is = this->__is();
	Int sz = __is->getReadEncapsSize();
	__is->readBlob(outParams, sz);
    }
    catch(const Exception& ex)
    {
	ice_exception(ex);
	return;
    }
    ice_response(ok, outParams);
}

