// **********************************************************************
//
// Copyright (c) 2002
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

#include <Ice/Outgoing.h>
#include <Ice/Object.h>
#include <Ice/Connection.h>
#include <Ice/Reference.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::NonRepeatable::NonRepeatable(const NonRepeatable& ex)
{
    _ex = auto_ptr<LocalException>(dynamic_cast<LocalException*>(ex.get()->ice_clone()));
}

IceInternal::NonRepeatable::NonRepeatable(const ::Ice::LocalException& ex)
{
    _ex = auto_ptr<LocalException>(dynamic_cast<LocalException*>(ex.ice_clone()));
}

const ::Ice::LocalException*
IceInternal::NonRepeatable::get() const
{
    assert(_ex.get());
    return _ex.get();
}

IceInternal::Outgoing::Outgoing(Connection* connection, Reference* ref, const string& operation,
				OperationMode mode, const Context& context) :
    _connection(connection),
    _reference(ref),
    _state(StateUnsent),
    _is(ref->instance.get()),
    _os(ref->instance.get())
{
    switch(_reference->mode)
    {
	case Reference::ModeTwoway:
	case Reference::ModeOneway:
	case Reference::ModeDatagram:
	{
	    _connection->prepareRequest(&_os);
	    break;
	}

	case Reference::ModeBatchOneway:
	case Reference::ModeBatchDatagram:
	{
	    _connection->prepareBatchRequest(&_os);
	    break;
	}
    }

    _reference->identity.__write(&_os);
    _os.write(_reference->facet);
    _os.write(operation);
    _os.write(static_cast<Byte>(mode));
    _os.writeSize(Int(context.size()));
    Context::const_iterator p;
    for(p = context.begin(); p != context.end(); ++p)
    {
	_os.write(p->first);
	_os.write(p->second);
    }
    
    //
    // Input and output parameters are always sent in an
    // encapsulation, which makes it possible to forward requests as
    // blobs.
    //
    _os.startWriteEncaps();
}

IceInternal::Outgoing::~Outgoing()
{
    if(_state == StateUnsent &&
       (_reference->mode == Reference::ModeBatchOneway || _reference->mode == Reference::ModeBatchDatagram))
    {
	_connection->abortBatchRequest();
    }
}

bool
IceInternal::Outgoing::invoke()
{
    _os.endWriteEncaps();
    
    switch(_reference->mode)
    {
	case Reference::ModeTwoway:
	{
	    bool timedOut = false;

	    {
		IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
		
		_connection->sendRequest(this, false, _reference->compress);
		_state = StateInProgress;
		
		Int timeout = _connection->timeout();
		while(_state == StateInProgress)
		{
		    if(timeout >= 0)
		    {	
			timedWait(IceUtil::Time::milliSeconds(timeout));
			if(_state == StateInProgress)
			{
			    timedOut = true;
			    _state = StateLocalException;
			    _exception = auto_ptr<LocalException>(new TimeoutException(__FILE__, __LINE__));
			}
		    }
		    else
		    {
			wait();
		    }
		}
	    }

	    if(timedOut)
	    {
		//
		// Must be called outside the synchronization of this
		// object.
		//
		_connection->exception(*_exception.get());
	    }

	    if(_exception.get())
	    {
		//
		// A CloseConnectionException indicates graceful
		// server shutdown, and is therefore always repeatable
		// without violating "at-most-once". That's because by
		// sending a close connection message, the server
		// guarantees that all outstanding requests can safely
		// be repeated.
		//
		if(dynamic_cast<const CloseConnectionException*>(_exception.get()))
		{
		    _exception->ice_throw();
		}
		
		//
		// Throw the exception wrapped in a NonRepeatable, to
		// indicate that the request cannot be resent without
		// potentially violating the "at-most-once" principle.
		//
		throw NonRepeatable(*_exception.get());
	    }
	    
	    if(_state == StateUserException)
	    {
		return false;
	    }

	    assert(_state == StateOK);
	    break;
	}
	
	case Reference::ModeOneway:
	case Reference::ModeDatagram:
	{
	    _connection->sendRequest(this, true, _reference->compress);
	    _state = StateInProgress;
	    break;
	}

	case Reference::ModeBatchOneway:
	case Reference::ModeBatchDatagram:
	{
	    //
	    // The state must be set to StateInProgress before calling
	    // finishBatchRequest, because otherwise if
	    // finishBatchRequest raises an exception, the destructor
	    // of this class will call abortBatchRequest, and calling
	    // both finishBatchRequest and abortBatchRequest is
	    // illegal.
	    //
	    _state = StateInProgress;
	    _connection->finishBatchRequest(&_os);
	    break;
	}
    }

    return true;
}

/*
void
IceInternal::Outgoing::invokeAsync(const IceAMI::Ice::ObjectPtr& asyncCB)
{
    _asyncCB = asyncCB;
    _os.endWriteEncaps();
    
    //
    // We cannot set _state to StateInProgress after sendRequest(),
    // because there would be a race with finished().
    //
    _state = StateInProgress;

    //
    // For asynchronous invocations, we always use twoway, regardless
    // of what the reference says. There is no point in asynchronous
    // oneways or datagrams, because there is no reply from the
    // server.
    //
    _connection->sendRequest(this, false, _reference->compress);
}
*/

void
IceInternal::Outgoing::finished(BasicStream& is)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    //
    // The state might be StateLocalException if there was a timeout
    // in invoke().
    //
    if(_state == StateInProgress)
    {
	_is.swap(is);
	Byte status;
	_is.read(status);

	switch(static_cast<DispatchStatus>(status))
	{
	    case DispatchOK:
	    {
		//
		// Input and output parameters are always sent in an
		// encapsulation, which makes it possible to forward
		// oneway requests as blobs.
		//
		_is.startReadEncaps();
		_state = StateOK;
		break;
	    }
	    
	    case DispatchUserException:
	    {
		//
		// Input and output parameters are always sent in an
		// encapsulation, which makes it possible to forward
		// oneway requests as blobs.
		//
		_is.startReadEncaps();
		_state = StateUserException;
		break;
	    }
	    
	    case DispatchObjectNotExist:
	    case DispatchFacetNotExist:
	    case DispatchOperationNotExist:
	    {
		_state = StateLocalException;
                // Don't read the exception members directly into the
                // exception. Otherwise if reading fails and raises an
                // exception, you will have a memory leak.
		Identity ident;
		ident.__read(&_is);
		vector<string> facet;
		_is.read(facet);
		string operation;
		_is.read(operation);

		RequestFailedException* ex;
		switch(static_cast<DispatchStatus>(status))
		{
		    case DispatchObjectNotExist:
		    {
			ex = new ObjectNotExistException(__FILE__, __LINE__);
			break;
		    }

		    case DispatchFacetNotExist:
		    {
			ex = new FacetNotExistException(__FILE__, __LINE__);
			break;
		    }

		    case DispatchOperationNotExist:
		    {
			ex = new OperationNotExistException(__FILE__, __LINE__);
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
		_exception = auto_ptr<LocalException>(ex);
		break;
	    }
	    
	    case DispatchUnknownException:
	    case DispatchUnknownLocalException:
	    case DispatchUnknownUserException:
	    {
		_state = StateLocalException;
                // Don't read the exception members directly into the
                // exception. Otherwise if reading fails and raises an
                // exception, you will have a memory leak.
		string unknown;
		_is.read(unknown);

		UnknownException* ex;
		switch(static_cast<DispatchStatus>(status))
		{
		    case DispatchUnknownException:
		    {
			ex = new UnknownException(__FILE__, __LINE__);
			break;
		    }

		    case DispatchUnknownLocalException:
		    {
			ex = new UnknownLocalException(__FILE__, __LINE__);
			break;
		    }

		    case DispatchUnknownUserException:
		    {
			ex = new UnknownUserException(__FILE__, __LINE__);
			break;
		    }

		    default:
		    {
			assert(false);
			break;
		    }
		}

		ex->unknown = unknown;
		_exception = auto_ptr<LocalException>(ex);
		break;
	    }
	    
	    default:
	    {
		_state = StateLocalException;
		_exception = auto_ptr<LocalException>(new UnknownReplyStatusException(__FILE__, __LINE__));
		break;
	    }
	}

	notify();
    }
}

void
IceInternal::Outgoing::finished(const LocalException& ex)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    
    //
    // The state might be StateLocalException if there was a timeout
    // in invoke().
    //
    if(_state == StateInProgress)
    {
	_state = StateLocalException;
	_exception = auto_ptr<LocalException>(dynamic_cast<LocalException*>(ex.ice_clone()));
	notify();
    }
}
